#ifndef ACTION_EDIT_UNDO_H
#define ACTION_EDIT_UNDO_H

#include "action_base.h"

namespace open_edi {
namespace gui {

class ActionEditUndo : public ActionBase {
    Q_OBJECT
  public:
    ActionEditUndo(GraphicsView& view, QObject* parent = nullptr);
    virtual ~ActionEditUndo();
};

} // namespace gui
} // namespace open_edi

#endif // ACTION_EDIT_UNDO_H
