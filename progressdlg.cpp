#include "progressdlg.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qvbox.h>
#include <dcopref.h>
#include <kanimwidget.h>
#include <kapplication.h>
#include <kconfig.h>
#include "cervisiapart.h"


struct ProgressDialog::Private
{
    bool            isCancelled;
    bool            isShown;
    bool            hasError;

    DCOPRef         job;
    QString         buffer;
    QString         errorId1, errorId2;
    QStringList     output;

    QTimer*         timer;
    KAnimWidget*    gear;
    QListBox*       resultbox;
};


ProgressDialog::ProgressDialog(QWidget* parent, const QString& heading,
                               const DCOPRef& job, const QString& errorIndicator,
                               const QString& caption)
    : KDialogBase(parent, 0, true, caption, Cancel, Cancel, true)
    , DCOPObject()
    , d(new Private)
{
    // initialize private data
    d->isCancelled = false;
    d->isShown     = false;
    d->hasError    = false;
    d->job         = job;
    d->buffer      = "";

    d->errorId1 = "cvs " + errorIndicator + ":";
    d->errorId2 = "cvs [" + errorIndicator + " aborted]:";

    setupGui(heading);
}


ProgressDialog::~ProgressDialog()
{
    delete d;
}


void ProgressDialog::setupGui(const QString& heading)
{
    QVBox* vbox = makeVBoxMainWidget();
    vbox->setSpacing(10);

    QWidget* headingBox = new QWidget(vbox);
    QHBoxLayout* hboxLayout = new QHBoxLayout(headingBox);

    QLabel* textLabel = new QLabel(heading, headingBox);
    textLabel->setMinimumWidth(textLabel->sizeHint().width());
    textLabel->setFixedHeight(textLabel->sizeHint().height());
    hboxLayout->addWidget(textLabel);
    hboxLayout->addStretch();

    d->gear = new KAnimWidget(QString("kde"), 32, headingBox);
    d->gear->setFixedSize(32, 32);
    hboxLayout->addWidget(d->gear);

    d->resultbox = new QListBox(vbox);
    d->resultbox->setEnabled(false);
    QFontMetrics fm(d->resultbox->fontMetrics());
    d->resultbox->setMinimumSize(fm.width("0")*70, fm.lineSpacing()*8);

    resize(sizeHint());
}


bool ProgressDialog::execute()
{
    KConfig* cfg = CervisiaPart::config();
    cfg->setGroup("General");
    unsigned timeout = cfg->readUnsignedNumEntry("Timeout", 4000);

    // get command line and display it
    QString cmdLine = d->job.call("cvsCommand");
    d->resultbox->insertItem(cmdLine);

    // establish connections to the signals of the cvs job
    connectDCOPSignal(d->job.app(), d->job.obj(), "jobExited(bool, int)",
                      "slotJobExited(bool, int)", true);
    connectDCOPSignal(d->job.app(), d->job.obj(), "receivedStdout(QString)",
                      "slotReceivedOutputNonGui(QString)", true);
    connectDCOPSignal(d->job.app(), d->job.obj(), "receivedStderr(QString)",
                      "slotReceivedOutputNonGui(QString)", true);

    // we wait for 4 seconds (or the timeout set by the user) before we
    // force the dialog to show up
    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(slotTimeoutOccurred()));
    d->timer->start(timeout, true);

    bool started = d->job.call("execute");
    if( !started )
        return false;

    QApplication::setOverrideCursor(waitCursor);
    kapp->enter_loop();
    if (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();

    return !d->isCancelled;
}


bool ProgressDialog::getLine(QString& line)
{
    if( d->output.isEmpty() )
        return false;

    line = d->output.first();
    d->output.remove(d->output.begin());

    return true;
}


void ProgressDialog::slotReceivedOutputNonGui(QString buffer)
{
    d->buffer += buffer;

    processOutput();
    if( d->hasError )
    {
        stopNonGuiPart();
        startGuiPart();
    }
}


void ProgressDialog::slotReceivedOutput(QString buffer)
{
    d->buffer += buffer;
    processOutput();
}


void ProgressDialog::slotJobExited(bool normalExit, int status)
{
    Q_UNUSED(status)

    if( !d->isShown )
        stopNonGuiPart();

    d->gear->stop();
    if( !d->buffer.isEmpty() )
    {
        d->buffer += '\n';
        processOutput();
    }

    // Close the dialog automatically if there are no
    // error messages or the process has been aborted
    // 'by hand' (e.g.  by clicking the cancel button)
    if( !d->hasError || !normalExit )
        kapp->exit_loop();
}


void ProgressDialog::slotCancel()
{
    d->isCancelled = true;

    bool isRunning = d->job.call("isRunning");
    if( isRunning )
        d->job.send("cancel");
    else
        kapp->exit_loop();
}


void ProgressDialog::slotTimeoutOccurred()
{
    stopNonGuiPart();
    startGuiPart();
}


void ProgressDialog::stopNonGuiPart()
{
    d->timer->stop();

    disconnectDCOPSignal(d->job.app(), d->job.obj(), "receivedStdout(QString)",
                      "slotReceivedOutputNonGui(QString)");
    disconnectDCOPSignal(d->job.app(), d->job.obj(), "receivedStderr(QString)",
                      "slotReceivedOutputNonGui(QString)");

    kapp->exit_loop();
}


void ProgressDialog::startGuiPart()
{
    connectDCOPSignal(d->job.app(), d->job.obj(), "receivedStdout(QString)",
                      "slotReceivedOutput(QString)", true);
    connectDCOPSignal(d->job.app(), d->job.obj(), "receivedStderr(QString)",
                      "slotReceivedOutput(QString)", true);

    show();
    d->isShown = true;

    d->gear->start();
    QApplication::restoreOverrideCursor();
    kapp->enter_loop();
}


void ProgressDialog::processOutput()
{
    int pos;
    while( (pos = d->buffer.find('\n')) != -1 )
    {
        QString item = d->buffer.left(pos);
        if( item.startsWith(d->errorId1) ||
            item.startsWith(d->errorId2) ||
            item.startsWith("cvs [server aborted]:") )
        {
            d->hasError = true;
            d->resultbox->insertItem(item);
        }
        else if( item.startsWith("cvs server:") )
            d->resultbox->insertItem(item);
        else
            d->output.append(item);

        // remove item from buffer
        d->buffer.remove(0, pos+1);
    }
}


#include "progressdlg.moc"
