#include "action_base.h"

namespace open_edi {
namespace gui {

ActionBase::ActionBase(QString name, GraphicsView& view, QObject* parent)
  : QObject(parent), view_(&view), action_name_(name) {
}

} // namespace gui
} // namespace open_edi
