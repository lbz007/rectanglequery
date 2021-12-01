#include "action_view_refresh.h"


namespace open_edi {
namespace gui {

ActionViewRefresh::ActionViewRefresh(GraphicsView& view, QObject* parent)
  : ActionBase("Refresh", view, parent) {
    view.initByCmd();
    
}

ActionViewRefresh::~ActionViewRefresh() {
}

} // namespace gui
} // namespace open_edi
