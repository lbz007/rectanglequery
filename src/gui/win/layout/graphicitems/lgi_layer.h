#ifndef EDI_GUI_LGI_LAYER_H_
#define EDI_GUI_LGI_LAYER_H_

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

class LGI_Layer : public LGI_Base {
  public:
    LGI_Layer(/* args */);
    ~LGI_Layer();
};
} // namespace gui
} // namespace open_edi

#endif