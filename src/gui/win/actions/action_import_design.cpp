#include "action_import_design.h"
#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

ActionFileImportDesign::ActionFileImportDesign(GraphicsView &view, QObject* parent)
  : ActionBase("Import Design", view, parent) {
    qDebug() << "........import design .........";

    // (GraphicsView*)(&view)->readData();
}

ActionFileImportDesign::~ActionFileImportDesign() {
}

} // namespace gui
} // namespace open_edi
