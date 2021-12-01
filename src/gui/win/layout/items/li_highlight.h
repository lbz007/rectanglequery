#ifndef OPEN_EDI_LI_HIGHTLIGHT_H_
#define OPEN_EDI_LI_HIGHTLIGHT_H_

#include <QImage>
#include <qmath.h>

#include "../graphicitems/lgi_highlight.h"
#include "../graphics_scene.h"
#include "li_base.h"

namespace open_edi {
namespace gui {
class LI_HighLight : public LI_Base {
  public:
    explicit LI_HighLight(ScaleFactor* scale_factor);
    LI_HighLight(const LI_HighLight& other) = delete;
    LI_HighLight& operator=(const LI_HighLight& rhs) = delete;
    ~LI_HighLight();

    virtual void    preDraw() override;
    LGI_HightLight* getGraphicItem();
    bool            isHighlightLI() { return true; };
    bool            isMainLI() { return true; };
    virtual void    setZ(int z) override;
    void            setMaskValue(int v) { mask_value_ = v; };
    int             getMaskValue() { return mask_value_; };

  protected:
    virtual void draw(QPainter* painter);

  private:
    LGI_HightLight* item_{nullptr};
    int             mask_value_{0};
};
} // namespace gui
} // namespace open_edi

#endif
