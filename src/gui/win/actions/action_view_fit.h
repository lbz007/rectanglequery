#ifndef EDI_GUI_ACTION_VIEW_FIT_H_
#define EDI_GUI_ACTION_VIEW_FIT_H_

#include "action_base.h"
#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

class ActionViewFit : public ActionBase {
    Q_OBJECT
  public:
    ActionViewFit(GraphicsView &view, QObject* parent = nullptr);
    virtual ~ActionViewFit();
};

} // namespace gui
} // namespace open_edi

#endif // ACTION_VIEW_ZOOMIN_H
