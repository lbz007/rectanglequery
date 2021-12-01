#include "li_layer.h"
#include <QDebug>
#include <QTime>
#include "../layout_def.h"
namespace open_edi {
namespace gui {

LI_Layer::LI_Layer(open_edi::db::Layer* layer, ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_Layer;
    item_->setLiBase(this);
    type_     = ObjType::kLayer;
    visible_  = true;
    db_layer_ = layer;
    name_     = layer->getName();
    setZ(layer->getZ());
    // li_mgr_->addLI(this);
    item_->setVisible(visible_);
}

LI_Layer::~LI_Layer() {
}

void LI_Layer::preDraw() {

    // QTime time;
    // time.start();
    // item_->setVisible(visible_);
    if (!visible_) {
        return;
    }
    QPainter painter(getGraphicItem()->getMap());
    pen_.setColor(getColor());
    brush_ = li_mgr_->getLiByName(kLiWireName)->getBrush();
    brush_.setColor(getColor());
    painter.setBrush(brush_);
    painter.setPen(pen_);
    setOffset(-selected_area_.getLLX(), -selected_area_.getLLY());

    for (auto& obj_v : obj_vectors_) {
        {
            for (auto obj : obj_v) {
                auto obj_type = obj->getObjectType();
                if (open_edi::db::ObjectType::kObjectTypeWire == obj_type) {
                    auto wire    = static_cast<open_edi::db::Wire*>(obj);
                    auto ext_box = wire->getBBox();

                    // auto head = wire->getHead();
                    // auto tail = wire->getTail();
                    if (wire->isVertical()) {
                        auto lly = ext_box.getLLY();
                        lly      = lly - ext_box.getWidth() / 2;
                        auto ury = ext_box.getURY();
                        ury      = ury + ext_box.getWidth() / 2;
                        ext_box.setLLY(lly);
                        ext_box.setURY(ury);
                    } else {
                        auto llx = ext_box.getLLX();
                        llx      = llx - ext_box.getHeight() / 2;
                        auto urx = ext_box.getURX();
                        urx      = urx + ext_box.getHeight() / 2;
                        ext_box.setLLX(llx);
                        ext_box.setURX(urx);
                    }

                    auto box = __translateDbBoxToQtBox(ext_box);
                    __fillBox(&painter, brush_, box);
                    __drawBox(&painter, box);
                }
            }
        }
    }

    // auto time_elapsed = time.elapsed();
    // printf("thread in total elapsed time %d (ms)\n", time_elapsed);

    for (auto& obj : obj_vectors_) {
        obj.clear();
    }
}

bool LI_Layer::hasSubLI() {
    return false;
}

LGI_Layer* LI_Layer::getGraphicItem() {
    return item_;
}

bool LI_Layer::isMainLI() {

    return db_layer_->isRouting();
}

void LI_Layer::setZ(int z) {
    z_ = z + (int)(LayerZ::kRouting);
    item_->setZValue(z_);
}

} // namespace gui
} // namespace open_edi