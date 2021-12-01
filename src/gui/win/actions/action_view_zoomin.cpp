#include "action_view_zoomin.h"


namespace open_edi {
namespace gui {

ActionViewZoomIn::ActionViewZoomIn(GraphicsView& view, QObject* parent)
  : ActionBase("Zoom In", view, parent) {
    view.slotZoomIn(true);
    
}

ActionViewZoomIn::~ActionViewZoomIn() {
}

} // namespace gui
} // namespace open_edi
