#ifndef CERVISIA_RESOLVEEDITORDIALOG_H
#define CERVISIA_RESOLVEEDITORDIALOG_H

#include <kdialogbase.h>

class QMultiLineEdit;
class QStringList;
class KConfig;


namespace Cervisia
{


class ResolveEditorDialog : public KDialogBase
{
public:
    explicit ResolveEditorDialog(KConfig& cfg, QWidget* parent=0, const char* name=0);
    virtual ~ResolveEditorDialog();

    void setContent(const QStringList& l);
    QStringList content() const;

private:   
    QMultiLineEdit* m_edit;
    KConfig&        m_partConfig;
};  


}


#endif
