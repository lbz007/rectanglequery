#ifndef EDI_GUI_LI_INSTANCES_H_
#define EDI_GUI_LI_INSTANCES_H_

#include <QPainter>
#include <qmath.h>
#include "../graphicitems/lgi_instance.h"
#include "../graphics_scene.h"
#include "li_base.h"
#include "li_pin.h"

namespace open_edi {
namespace gui {
class LI_Instance : public LI_Base {
  public:
    explicit LI_Instance(ScaleFactor* scale_factor);
    LI_Instance(const LI_Instance& other) = delete;
    LI_Instance& operator=(const LI_Instance& rhs) = delete;
    ~LI_Instance();

    LI_Pin* li_pins;

    virtual void  preDraw() override;
    virtual bool  hasSubLI() override;
    LGI_Instance* getGraphicItem() override;
    virtual bool  isMainLI() override;
    auto          addInst(open_edi::db::Inst* inst) { inst_vec_.push_back(inst); };
    auto          cleaInst() { inst_vec_.clear(); };
    virtual void  setZ(int z) override;
    virtual void  setVisible(bool visible);

    std::vector<open_edi::db::Object*>* getObjVectors() { return obj_vectors_; };

    std::vector<open_edi::db::Object*>* getObjVector(int index) { return index < kSplitNum ? &obj_vectors_[index] : nullptr; };

  protected:
    virtual void draw(QPainter* painter);

  private:
    LGI_Instance*                    item_;
    std::vector<open_edi::db::Inst*> inst_vec_;

    std::vector<open_edi::db::Object*> obj_vectors_[kSplitNum];
};
} // namespace gui
} // namespace open_edi

#endif
