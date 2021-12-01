#ifndef LGI_PINS_H
#define LGI_PINS_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

class LGI_Pin : public LGI_Base {
  public:
    LGI_Pin(/* args */);
    ~LGI_Pin();
};
} // namespace gui
} // namespace open_edi

#endif