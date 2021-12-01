#ifndef ACTION_VIEW_ZOOMIN_H_
#define ACTION_VIEW_ZOOMIN_H_

#include "action_base.h"
#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

class ActionViewZoomIn : public ActionBase {
    Q_OBJECT
  public:
    ActionViewZoomIn(GraphicsView &view, QObject* parent = nullptr);
    virtual ~ActionViewZoomIn();
};

} // namespace gui
} // namespace open_edi

#endif // ACTION_VIEW_ZOOMIN_H
