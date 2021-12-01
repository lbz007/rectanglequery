#ifndef EDI_GUI_LGI_HightLightS_H
#define EDI_GUI_LGI_HightLightS_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

class LGI_HightLight : public LGI_Base {
  public:
    LGI_HightLight(/* args */);
    ~LGI_HightLight();
};
} // namespace gui
} // namespace open_edi

#endif