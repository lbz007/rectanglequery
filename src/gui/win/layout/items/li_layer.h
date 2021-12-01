#ifndef EDI_GUI_LI_LAYER_H_
#define EDI_GUI_LI_LAYER_H_

#include <QPainter>
#include <qmath.h>
#include "../graphicitems/lgi_layer.h"
#include "../graphics_scene.h"
#include "li_base.h"

namespace open_edi {
namespace gui {
class LI_Layer : public LI_Base {
  public:
    LI_Layer()        = delete;
    LI_Layer& operator=(const LI_Layer& rhs) = delete;
    explicit LI_Layer(open_edi::db::Layer* layer, ScaleFactor* scale_factor);
    ~LI_Layer();

    virtual void   preDraw() override;
    virtual bool   hasSubLI() override;
    LGI_Layer*     getGraphicItem() override;
    virtual bool   isMainLI() override;
    auto           getLayer() { return db_layer_; };
    virtual QColor getColor() { return pen_.color(); };
    void           setOnDrawing(bool status) { on_drawing_ = status; };
    auto           isOnDrawing() { return on_drawing_; };
    void           drawLayer();
    virtual void   setZ(int z) override;

    std::vector<open_edi::db::Object*>* getObjVectors() { return obj_vectors_; };

    std::vector<open_edi::db::Object*>* getObjVector(int index) { return index < kSplitNum ? &obj_vectors_[index] : nullptr; };

  private:
    bool                 on_drawing_{false};
    LGI_Layer*           item_;
    open_edi::db::Layer* db_layer_;

    std::vector<open_edi::db::Object*> obj_vectors_[kSplitNum];
};
} // namespace gui
} // namespace open_edi

#endif