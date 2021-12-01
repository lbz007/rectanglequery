#ifndef EDI_GUI_LGI_WIRES_H
#define EDI_GUI_LGI_WIRES_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

class LGI_Wire : public LGI_Base {
  public:
    LGI_Wire(/* args */);
    ~LGI_Wire();
};
} // namespace gui
} // namespace open_edi

#endif