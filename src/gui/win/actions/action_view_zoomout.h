#ifndef ACTION_VIEW_ZOOMOUT_H_
#define ACTION_VIEW_ZOOMOUT_H_

#include "action_base.h"
#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

class ActionViewZoomOut : public ActionBase {
    Q_OBJECT
  public:
    ActionViewZoomOut(GraphicsView &view, QObject* parent = nullptr);
    virtual ~ActionViewZoomOut();
};

} // namespace gui
} // namespace open_edi

#endif // ACTION_VIEW_ZOOMIN_H
