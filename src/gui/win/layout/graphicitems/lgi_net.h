#ifndef EDI_GUI_LGI_NETS_H
#define EDI_GUI_LGI_NETS_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

class LGI_Net : public LGI_Base {
  public:
    LGI_Net(/* args */);
    ~LGI_Net();
};
} // namespace gui
} // namespace open_edi

#endif