#include "action_view_zoomout.h"


namespace open_edi {
namespace gui {

ActionViewZoomOut::ActionViewZoomOut(GraphicsView& view, QObject* parent)
  : ActionBase("Zoom out", view, parent) {
    view.slotZoomOut(true);
    
}

ActionViewZoomOut::~ActionViewZoomOut() {
}

} // namespace gui
} // namespace open_edi
