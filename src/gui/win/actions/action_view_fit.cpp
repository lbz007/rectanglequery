#include "action_view_fit.h"


namespace open_edi {
namespace gui {

ActionViewFit::ActionViewFit(GraphicsView& view, QObject* parent)
  : ActionBase("Zoom out", view, parent) {
    view.refreshFitDraw();
    
}

ActionViewFit::~ActionViewFit() {
}

} // namespace gui
} // namespace open_edi
