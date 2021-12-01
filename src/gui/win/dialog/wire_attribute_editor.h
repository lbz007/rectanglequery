#ifndef OPEN_EDI_GUI_WIRE_PROPERTY_DLG_H_
#define OPEN_EDI_GUI_WIRE_PROPERTY_DLG_H_

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

#include "attribute_editor_base.h"

namespace open_edi {
namespace gui {
class WirePropertyDlg : public AttributeEditorBase {
    Q_OBJECT
  public:
    WirePropertyDlg(open_edi::db::Object* wire, QWidget* parent = nullptr);
    ~WirePropertyDlg(){};

  private:
    open_edi::db::Wire* wire_;

    QLineEdit* editor_layere_{new QLineEdit(this)};
    // QLineEdit*          editor_num_pins_{new QLineEdit(this)};
    QLineEdit* editor_location_{new QLineEdit(this)};
    // QLineEdit*          editor_orient_{new QLineEdit(this)};
    QLineEdit* net_name_{new QLineEdit(this)};
};

} // namespace gui
} // namespace open_edi

#endif