#ifndef OPEN_EDI_GUI_INST_PROPERTY_DLG_H_
#define OPEN_EDI_GUI_INST_PROPERTY_DLG_H_

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

#include "attribute_editor_base.h"

namespace open_edi {
namespace gui {
class InstPropertyDlg : public AttributeEditorBase {
    Q_OBJECT
  public:
    InstPropertyDlg(open_edi::db::Object* inst, QWidget* parent = nullptr);
    ~InstPropertyDlg(){};

  private:
    open_edi::db::Inst* inst_;
    QLineEdit*          editor_inst_name_{new QLineEdit(this)};
    QLineEdit*          editor_num_pins_{new QLineEdit(this)};
    QLineEdit*          editor_location_{new QLineEdit(this)};
    QLineEdit*          editor_orient_{new QLineEdit(this)};
};

} // namespace gui
} // namespace open_edi

#endif