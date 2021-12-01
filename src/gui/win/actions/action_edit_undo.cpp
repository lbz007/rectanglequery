#include "action_edit_undo.h"

namespace open_edi {
namespace gui {

ActionEditUndo::ActionEditUndo(GraphicsView &view, QObject* parent)
  : ActionBase("Undo", view, parent) {
    qDebug() << "edit undo .........";
}

ActionEditUndo::~ActionEditUndo() {
}

} // namespace gui
} // namespace open_edi
