#include "resolvedlg_p.h"
using namespace Cervisia;

#include <qmultilineedit.h>


ResolveEditorDialog::ResolveEditorDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel, Ok, true)
    , m_partConfig(cfg)
{
    m_edit = new QMultiLineEdit(this);
    m_edit->setFocus();

    setMainWidget(m_edit);

    QFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

    QSize size = configDialogSize(m_partConfig, "ResolveEditDialog");
    resize(size);
}


ResolveEditorDialog::~ResolveEditorDialog()
{
    saveDialogSize(m_partConfig, "ResolveEditDialog");
}


void ResolveEditorDialog::setContent(const QStringList &l)
{
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it)
        m_edit->insertLine((*it).left((*it).length()-1));
}


QStringList ResolveEditorDialog::content() const
{
    QStringList l;
    for (int i = 0; i < m_edit->numLines(); ++i)
        l << (m_edit->textLine(i) + '\n');

    return l;
}

