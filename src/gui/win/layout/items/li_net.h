#ifndef EDI_GUI_LI_NETS_H_
#define EDI_GUI_LI_NETS_H_

#include <QPainter>
#include <qmath.h>
#include "../graphicitems/lgi_net.h"
#include "../graphics_scene.h"
#include "li_base.h"
#include "li_wire.h"

namespace open_edi {
namespace gui {
class LI_Net : public LI_Base {
  public:
    explicit LI_Net(ScaleFactor* scale_factor);
    LI_Net(const LI_Net& other) = delete;
    LI_Net& operator=(const LI_Net& rhs) = delete;
    ~LI_Net();

    virtual void preDraw() override;
    LGI_Net*     getGraphicItem();
    virtual bool hasSubLI() override;
    virtual bool isMainLI() override;
    virtual void setVisible(bool visible) override;
    LI_Wire*     li_wires;
    virtual void setZ(int z) override;

  protected:
    virtual void draw(QPainter* painter);

  private:
    LGI_Net* item_;
};
} // namespace gui
} // namespace open_edi

#endif
