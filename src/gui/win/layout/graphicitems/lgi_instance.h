#ifndef LGI_INSTANCES_H
#define LGI_INSTANCES_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

class LGI_Instance : public LGI_Base {
  public:
    LGI_Instance(/* args */);
    ~LGI_Instance();
};
} // namespace gui
} // namespace open_edi

#endif