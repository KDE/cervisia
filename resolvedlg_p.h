#ifndef CERVISIA_RESOLVEEDITORDIALOG_H
#define CERVISIA_RESOLVEEDITORDIALOG_H

#include <kdialogbase.h>

class KTextEdit;
class QStringList;
class KConfig;


namespace Cervisia
{


class ResolveEditorDialog : public KDialogBase
{
public:
    explicit ResolveEditorDialog(KConfig& cfg, QWidget* parent=0, const char* name=0);
    virtual ~ResolveEditorDialog();

    void setContent(const QString& text);
    QString content() const;

private:   
    KTextEdit* m_edit;
    KConfig&   m_partConfig;
};  


}


#endif
