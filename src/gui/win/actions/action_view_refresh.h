#ifndef EDI_GUI_ACTION_VIEW_REFRESH_H_
#define EDI_GUI_ACTION_VIEW_REFRESH_H_

#include "action_base.h"
#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

class ActionViewRefresh : public ActionBase {
    Q_OBJECT
  public:
    ActionViewRefresh(GraphicsView &view, QObject* parent = nullptr);
    virtual ~ActionViewRefresh();
};

} // namespace gui
} // namespace open_edi

#endif // ACTION_VIEW_ZOOMIN_H
