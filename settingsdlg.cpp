/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "settingsdlg.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qwidgetlist.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <kbuttonbox.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include "misc.h"


FontButton::FontButton( const QString &text, QWidget *parent, const char *name )
    : QPushButton(text, parent, name)
{
    connect( this, SIGNAL(clicked()), this, SLOT(chooseFont()) );
}


void FontButton::chooseFont()
{
    QFont newFont(font());

    if (KFontDialog::getFont(newFont, false, this) == QDialog::Rejected)
        return;

    setFont(newFont);
    repaint(false);
}


SettingsDialog::SettingsDialog( KConfig *conf, QWidget *parent, const char *name )
    : KDialogBase(KDialogBase::Tabbed, i18n("Configure Cervisia"),
      KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
      KDialogBase::Ok,
      parent, name, true)
{
    config = conf;

    // open cvs DCOP service configuration file
    serviceConfig = new KConfig("cvsservicerc");

    //
    // General Options
    //
    QVBox *generalPage = addVBoxPage( i18n("&General") );

    QLabel *usernamelabel = new QLabel( i18n("&User name for the change log editor:"), generalPage );
    usernameedit = new KLineEdit(generalPage);
    usernameedit->setFocus();
    usernamelabel->setBuddy(usernameedit);

    QLabel *cvspathlabel = new QLabel( i18n("&Path to cvs:"), generalPage );
    cvspathedit = new KLineEdit(generalPage);
    cvspathlabel->setBuddy(cvspathedit);

    QLabel *editorlabel = new QLabel( i18n("&Editor:"), generalPage );
    editoredit = new KURLRequester(generalPage);
    editorlabel->setBuddy(editoredit);

    new QWidget(generalPage);

    //
    // Diff Options
    //
    QGrid *diffPage = addGridPage( 2, QGrid::Horizontal, i18n("&Diff") );

    QLabel *contextlabel = new QLabel( i18n("&Number of context lines in diff dialog:"), diffPage );
    contextedit = new KIntNumInput( 0, diffPage );
    contextedit->setRange(0, 65535, 1, false);
    contextlabel->setBuddy(contextedit);

    QLabel *diffoptlabel = new QLabel(i18n("Additional &options for cvs diff:"), diffPage);
    diffoptedit = new KLineEdit(diffPage);
    diffoptlabel->setBuddy(diffoptedit);

    QLabel *tabwidthlabel = new QLabel(i18n("Tab &width in diff dialog:"), diffPage);
    tabwidthedit = new KIntNumInput(0, diffPage);
    tabwidthedit->setRange(1, 16, 1, false);
    tabwidthlabel->setBuddy(tabwidthedit);

    QLabel *extdifflabel = new QLabel(i18n("External diff &frontend:"), diffPage);
    extdiffedit = new KLineEdit(diffPage);
    extdifflabel->setBuddy(extdiffedit);

    new QWidget(diffPage);

    //
    // Status Options
    //
    QVBox *statusPage = addVBoxPage( i18n("&Status") );

    remotestatusbox = new QCheckBox(i18n("When opening a sandbox from a &remote repository,\n"
                                         "start a File->Status command automatically"), statusPage);
    localstatusbox = new QCheckBox(i18n("When opening a sandbox from a &local repository,\n"
                                        "start a File->Status command automatically"), statusPage);

    new QWidget(statusPage);

    //
    // Advanced Options
    //
    QGrid *advancedPage = addGridPage( 2, QGrid::Horizontal, i18n("Ad&vanced") );

    QLabel *timeoutlabel = new QLabel( i18n("&Timeout after which a progress dialog appears (in ms):"),
                                       advancedPage );
    timeoutedit = new KIntNumInput( 0, advancedPage );
    timeoutedit->setRange( 0, 50000, 100, false );
    timeoutlabel->setBuddy( timeoutedit );

    QLabel *compressionlabel = new QLabel( i18n("Default compression &level:"), advancedPage );
    compressioncombo = new QComboBox( false, advancedPage );
    compressionlabel->setBuddy( compressioncombo );

    compressioncombo->insertItem("0", 0);
    compressioncombo->insertItem("1", 1);
    compressioncombo->insertItem("2", 2);
    compressioncombo->insertItem("3", 3);

    new QWidget(advancedPage);

    //
    // Look and Feel Options
    //
    QVBox *lookPage = addVBoxPage( i18n("&Appearance") );

    protocolfontbox = new FontButton(i18n("Font for &Protocol Window..."), lookPage);
    annotatefontbox = new FontButton(i18n("Font for A&nnotate View..."), lookPage);
    difffontbox = new FontButton(i18n("Font for D&iff View..."), lookPage);
    splitterbox = new QCheckBox(i18n("Split main window &horizontally"), lookPage);

    new QWidget(lookPage);

    //
    // Color Options
    //
    QGrid *colorsPage = addGridPage( 4, QGrid::Horizontal, i18n("Co&lors") );

    QLabel *conflictlabel = new QLabel( i18n("Conflict:"), colorsPage );
    conflictbutton = new KColorButton( colorsPage);
    conflictlabel->setBuddy( conflictbutton );

    QLabel *diffchangelabel = new QLabel( i18n("Diff change:"), colorsPage );
    diffchangebutton = new KColorButton( colorsPage);
    diffchangelabel->setBuddy( diffchangebutton );

    QLabel *localchangelabel = new QLabel( i18n("Local change:"), colorsPage );
    localchangebutton = new KColorButton( colorsPage);
    localchangelabel->setBuddy( localchangebutton );

    QLabel *diffinsertlabel = new QLabel( i18n("Diff insertion:"), colorsPage );
    diffinsertbutton = new KColorButton( colorsPage);
    diffinsertlabel->setBuddy( diffinsertbutton );

    QLabel *remotechangelabel = new QLabel( i18n("Remote change:"), colorsPage );
    remotechangebutton = new KColorButton( colorsPage);
    remotechangelabel->setBuddy( remotechangebutton );

    QLabel *diffdeletelabel = new QLabel( i18n("Diff deletion:"), colorsPage );
    diffdeletebutton = new KColorButton( colorsPage);
    diffdeletelabel->setBuddy( diffdeletebutton );

    new QWidget(colorsPage);

#if 0
    QGridLayout *editorlayout = new QGridLayout(editorgroup, 4, 2, 10, 6);

    editoredit = new KLineEdit(editorgroup);
    editorlayout->addWidget(editoredit, 0, 1);

    QLabel *editorlabel = new QLabel(editoredit, i18n("&Editor:"), editorgroup);
    editorlayout->addWidget(editorlabel, 0, 0);

    usedcopbox = new QCheckBox(i18n("Use &DCOP"), editorgroup);
    editorlayout->addMultiCellWidget(usedcopbox, 1, 1, 0, 1);

    clientedit = new KLineEdit(editorgroup);
    editorlayout->addWidget(clientedit, 2, 1);

    QLabel *clientlabel = new QLabel(clientedit, i18n("&Client:"), editorgroup);
    editorlayout->addWidget(clientlabel, 2, 0);

    objectedit = new KLineEdit(editorgroup);
    editorlayout->addWidget(objectedit, 3, 1);

    QLabel *objectlabel = new QLabel(objectedit, i18n("&Object:"), editorgroup);
    editorlayout->addWidget(objectlabel, 3, 0);

    connect(usedcopbox, SIGNAL(toggled(bool)),
            clientedit, SLOT(setEnabled(bool)));
    connect(usedcopbox, SIGNAL(toggled(bool)),
            objectedit, SLOT(setEnabled(bool)));
    editorlayout->activate();
#endif

    readSettings();

    setHelp("customization", "cervisia");
}

SettingsDialog::~SettingsDialog()
{
    delete serviceConfig;
}

void SettingsDialog::readSettings()
{
    // read entries from cvs DCOP service configuration
    serviceConfig->setGroup("General");
    cvspathedit->setText(serviceConfig->readEntry("CVSPath", "cvs"));
    compressioncombo->setCurrentItem(serviceConfig->readNumEntry("Compression", 0));

    config->setGroup("General");
    timeoutedit->setValue((int)config->readUnsignedNumEntry("Timeout", 4000));
    usernameedit->setText(config->readEntry("Username", userName()));

    contextedit->setValue((int)config->readUnsignedNumEntry("ContextLines", 65535));
    tabwidthedit->setValue((int)config->readUnsignedNumEntry("TabWidth", 8));
    diffoptedit->setText(config->readEntry("DiffOptions", ""));
    extdiffedit->setText(config->readEntry("ExternalDiff", ""));
    remotestatusbox->setChecked(config->readBoolEntry("StatusForRemoteRepos", false));
    localstatusbox->setChecked(config->readBoolEntry("StatusForLocalRepos", false));
    config->setGroup("Communication");
    editoredit->lineEdit()->setText(config->readEntry("Editor"));
    config->setGroup("LookAndFeel");
    protocolfontbox->setFont(config->readFontEntry("ProtocolFont"));
    annotatefontbox->setFont(config->readFontEntry("AnnotateFont"));
    difffontbox->setFont(config->readFontEntry("DiffFont"));
    splitterbox->setChecked(config->readBoolEntry("SplitHorizontally",true));

    config->setGroup("Colors");
    QColor defaultColor = QColor(255, 130, 130);
    conflictbutton->setColor(config->readColorEntry("Conflict",&defaultColor));
    defaultColor=QColor(130, 130, 255);
    localchangebutton->setColor(config->readColorEntry("LocalChange",&defaultColor));
    defaultColor=QColor(70, 210, 70);
    remotechangebutton->setColor(config->readColorEntry("RemoteChange",&defaultColor));

    defaultColor=QColor(237, 190, 190);
    diffchangebutton->setColor(config->readColorEntry("DiffChange",&defaultColor));
    defaultColor=QColor(190, 190, 237);
    diffinsertbutton->setColor(config->readColorEntry("DiffInsert",&defaultColor));
    defaultColor=QColor(190, 237, 190);
    diffdeletebutton->setColor(config->readColorEntry("DiffDelete",&defaultColor));
}


void SettingsDialog::writeSettings()
{
    // write entries to cvs DCOP service configuration
    serviceConfig->setGroup("General");
    serviceConfig->writeEntry("CVSPath", cvspathedit->text());
    serviceConfig->writeEntry("Compression", compressioncombo->currentItem());

    // write to disk so other services can reparse the configuration
    serviceConfig->sync();

    config->setGroup("General");
    config->writeEntry("Timeout", (unsigned)timeoutedit->value());
    config->writeEntry("Username", usernameedit->text());
    // TODO: remove when move to cvs DCOP service is complete
    config->writeEntry("CVSPath", cvspathedit->text());
    config->writeEntry("Compression", compressioncombo->currentItem());
    // END TODO
    config->writeEntry("ContextLines", (unsigned)contextedit->value());
    config->writeEntry("TabWidth", tabwidthedit->value());
    config->writeEntry("DiffOptions", diffoptedit->text());
    config->writeEntry("ExternalDiff", extdiffedit->text());
    config->writeEntry("StatusForRemoteRepos", remotestatusbox->isChecked());
    config->writeEntry("StatusForLocalRepos", localstatusbox->isChecked());
    config->setGroup("Communication");
    config->writeEntry("Editor", editoredit->lineEdit()->text());
#if 0
    config->writeEntry("UseDCOP", usedcopbox->isChecked());
    config->writeEntry("DCOPClient", clientedit->text());
    config->writeEntry("DCOPObject", objectedit->text());
#endif
    config->setGroup("LookAndFeel");
    config->writeEntry("ProtocolFont", protocolfontbox->font());
    config->writeEntry("AnnotateFont", annotatefontbox->font());
    config->writeEntry("DiffFont", difffontbox->font());
    config->writeEntry("SplitHorizontally", splitterbox->isChecked());

    config->setGroup("Colors");
    config->writeEntry("Conflict",conflictbutton->color());
    config->writeEntry("LocalChange",localchangebutton->color());
    config->writeEntry("RemoteChange",remotechangebutton->color());
    config->writeEntry("DiffChange",diffchangebutton->color());
    config->writeEntry("DiffInsert",diffinsertbutton->color());
    config->writeEntry("DiffDelete",diffdeletebutton->color());

    // I'm not yet sure whether this is a hack or not :-)
    QWidgetListIt it(*QApplication::allWidgets());
    for (; it.current(); ++it)
        {
            QWidget *w = it.current();
            if (w->inherits("ProtocolView"))
                w->setFont(protocolfontbox->font());
            if (w->inherits("AnnotateView"))
                w->setFont(annotatefontbox->font());
            if (w->inherits("DiffView"))
                w->setFont(difffontbox->font());
        }
    config->sync();
}

void SettingsDialog::done(int res)
{

    if (res == Accepted)
        writeSettings();
    KDialogBase::done(res);
    delete this;
}

#include "settingsdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
