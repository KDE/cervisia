#include "resolvedlg_p.h"
using namespace Cervisia;

#include <ktextedit.h>


ResolveEditorDialog::ResolveEditorDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel, Ok, true)
    , m_partConfig(cfg)
{
    m_edit = new KTextEdit(this);
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


void ResolveEditorDialog::setContent(const QString& text)
{
    m_edit->setText(text);
}


QString ResolveEditorDialog::content() const
{
    return m_edit->text();
}
