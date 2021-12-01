#ifndef OPEN_EDI_GUI_ATTRIBUTE_EDITOR_BASE_H_
#define OPEN_EDI_GUI_ATTRIBUTE_EDITOR_BASE_H_

#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"

namespace open_edi {
namespace gui {
class AttributeEditorBase : public QDialog {
    Q_OBJECT
  public:
    AttributeEditorBase(open_edi::db::Object* obj, QWidget* parent);

    enum Attribut {
        kName,
        kValue,
        kType
    };

  protected:
    // QGridLayout* grid_layout_{nullptr};
    // QLabel*      label_name_{new QLabel("Name", this)};
    // QLabel*      label_value_{new QLabel("Value", this)};
    // QLabel*      label_type_{new QLabel("Type", this)};
    // QLabel*      label_obj_type_{new QLabel("Object Type:", this)};
};

} // namespace gui
} // namespace open_edi

#endif