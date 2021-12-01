#ifndef EDI_GUI_LI_WIRES_H_
#define EDI_GUI_LI_WIRES_H_

#include <QPainter>
#include <qmath.h>
#include "../graphicitems/lgi_wire.h"
#include "../graphics_scene.h"

#include "li_base.h"

namespace open_edi {
namespace gui {
class LI_Wire : public LI_Base {
  public:
    explicit LI_Wire(ScaleFactor* scale_factor);
    LI_Wire(const LI_Wire& other) = delete;
    LI_Wire& operator=(const LI_Wire& rhs) = delete;
    ~LI_Wire();

    virtual void preDraw() override;
    LGI_Wire*    getGraphicItem();
    void         drawWires(open_edi::db::Wire& wire);
    void         drawWires(open_edi::db::Wire* wire);
    void         fillImage();
    virtual bool isMainLI() override;
    virtual void setZ(int z) override;

  protected:
    virtual void draw(QPainter* painter);

  private:
    LGI_Wire* item_;
};
} // namespace gui
} // namespace open_edi

#endif
