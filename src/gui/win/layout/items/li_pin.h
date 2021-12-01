#ifndef EDI_GUI_LI_PINS_H_
#define EDI_GUI_LI_PINS_H_

#include <QPainter>
#include <qmath.h>
#include "../graphicitems/lgi_pin.h"
#include "../graphics_scene.h"
#include "li_base.h"

namespace open_edi {
namespace gui {

class LI_Layer;

class LI_Pin : public LI_Base {
  public:
    explicit LI_Pin(ScaleFactor* scale_factor);
    LI_Pin(const LI_Pin& other) = delete;
    LI_Pin& operator=(const LI_Pin& rhs) = delete;
    ~LI_Pin();

    virtual void preDraw() override;
    LGI_Pin*     getGraphicItem();
    void         drawPins(open_edi::db::Inst& ins);
    void         drawPins(open_edi::db::Pin* pin);
    virtual bool isMainLI() { return true; };
    LI_Layer*    getLayer(open_edi::db::Pin* pin);
    LI_Layer*    getBindingLayer() { return binding_layer_; };
    virtual void setVisible(bool visible);
    virtual void setZ(int z) override;

    std::vector<open_edi::db::Object*>* getObjVectors() { return obj_vectors_; };
    std::vector<open_edi::db::Object*>* getObjVector(int index) { return index < kSplitNum ? &obj_vectors_[index] : nullptr; };

  protected:
    virtual void draw(QPainter* painter);

  private:
    LGI_Pin*  item_{nullptr};
    LI_Layer* binding_layer_{nullptr};

    std::vector<open_edi::db::Object*> obj_vectors_[kSplitNum];
};
} // namespace gui
} // namespace open_edi

#endif
