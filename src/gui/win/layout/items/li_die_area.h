#ifndef LI_DIE_AREA_H
#define LI_DIE_AREA_H

#include <QImage>
#include <qmath.h>

#include "../graphicitems/lgi_die_area.h"
#include "../graphics_scene.h"
#include "li_base.h"

namespace open_edi {
namespace gui {
class LI_DieArea : public LI_Base {
  public:
    explicit LI_DieArea(ScaleFactor* scale_factor);
    LI_DieArea(const LI_DieArea& other) = delete;
    LI_DieArea& operator=(const LI_DieArea& rhs) = delete;
    ~LI_DieArea();

    virtual void preDraw() override;
    LGI_DieArea* getGraphicItem();
    virtual bool isMainLI() override;
    virtual void setZ(int z) override;
    int          getDieW() { return die_area_w_; };
    int          getDieH() { return die_area_h_; };

  protected:
    virtual void draw(QPainter* painter);

  private:
    LGI_DieArea* item_{nullptr};
};
} // namespace gui
} // namespace open_edi

#endif
