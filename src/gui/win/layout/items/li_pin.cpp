#include "li_pin.h"

#include <QDebug>

#include "../layout_def.h"
#include "li_layer.h"

namespace open_edi {
namespace gui {
LI_Pin::LI_Pin(ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_Pin;
    item_->setLiBase(this);
    pen_.setColor(QColor(0, 0, 0xff, 0xff));
    brush_ = QBrush(QColor(0, 0, 0xff, 0xff), Qt::Dense5Pattern);
    type_  = ObjType::kPin;
    name_  = kLiPinName;
    //defualt visible
    visible_ = true;
    setZ(0);
}

LI_Pin::~LI_Pin() {
}

LGI_Pin* LI_Pin::getGraphicItem() {
    return item_;
}

void LI_Pin::draw(QPainter* painter) {
    LI_Base::draw(painter);
}

void LI_Pin::drawPins(open_edi::db::Inst& ins) {
    if (!visible_) {
        return;
    }

    auto box = ins.getBox();

    auto factor = *scale_factor_;

    auto li_inst_box = __translateDbBoxToQtBox(box);

    // Todo!!
    if (!((li_inst_box.getWidth() > 4) && li_inst_box.getHeight() > 4)) {
        return;
    }

    if (ins.numPins()) {
        // printf("instance number of pins %d\n", ins.numPins());

        std::vector<open_edi::db::Box> box_vector;

        auto pins        = ins.getPins();
        auto location    = ins.getLocation();
        auto pins_vector = open_edi::db::Object::addr<open_edi::db::ArrayObject<open_edi::db::ObjectId>>(pins);
        auto factor      = *scale_factor_;

        for (auto iter = pins_vector->begin(); iter != pins_vector->end(); ++iter) {
            auto pin = open_edi::db::Object::addr<open_edi::db::Pin>(*iter);
            if (!pin) {
                continue;
            }

            //transform and filp according instance orient
            pin->getBoxVector(box_vector);

            if (!binding_layer_) {
                binding_layer_ = getLayer(pin);
            }

            if (!binding_layer_->isVisible()) {
                return;
            }

            QPainter painter(getGraphicItem()->getMap());

            pen_.setColor(binding_layer_->getColor());

            painter.setPen(pen_);

            brush_.setColor(pen_.color());

            int pin_llx, pin_lly, pin_urx, pin_ury;

            for (int index = 0; index < box_vector.size(); ++index) {

                if (!(box_vector[index].getWidth() >= 1 || box_vector[index].getHeight() >= 1)) continue;

                auto pin_box = __translateDbBoxToQtBox(box_vector[index]);

                __fillBox(&painter, brush_, pin_box);
                __drawBox(&painter, pin_box);
            }

            box_vector.clear();
        }
    }
}

void LI_Pin::drawPins(open_edi::db::Pin* pin) {
    if (!visible_) {
        return;
    }
    auto ins = pin->getInst();
    if (ins == nullptr) {
        return;
    }
    auto box    = ins->getBox();
    auto factor = *scale_factor_;

    auto li_inst_box = __translateDbBoxToQtBox(box);
    // Todo!!
    if (!((li_inst_box.getWidth() > 4) && li_inst_box.getHeight() > 4)) {
        return;
    }
    std::vector<open_edi::db::Box> box_vector;
    //transform and filp according instance orient
    pin->getBoxVector(box_vector);

    if (!binding_layer_) {
        binding_layer_ = getLayer(pin);
    }

    if (!binding_layer_->isVisible()) {
        return;
    }

    QPainter painter(getGraphicItem()->getMap());

    pen_.setColor(binding_layer_->getColor());

    painter.setPen(pen_);

    brush_.setColor(pen_.color());

    int pin_llx, pin_lly, pin_urx, pin_ury;

    for (int index = 0; index < box_vector.size(); ++index) {

        if (!(box_vector[index].getWidth() >= 1 || box_vector[index].getHeight() >= 1)) continue;

        auto pin_box = __translateDbBoxToQtBox(box_vector[index]);

        __fillBox(&painter, brush_, pin_box);
        __drawBox(&painter, pin_box);
    }

    box_vector.clear();
}

LI_Layer* LI_Pin::getLayer(open_edi::db::Pin* pin) {
    auto term = pin->getTerm();
    for (int i = 0; i < term->getPortNum(); i++) {
        auto port = term->getPort(i);
        for (int j = 0; j < port->getLayerGeometryNum(); j++) {
            auto geo      = port->getLayerGeometry(j);
            auto li_layer = dynamic_cast<LI_Layer*>(li_mgr_->getLayerByName(geo->getLayer()->getName()));
            return li_layer;
        }
    }
    return nullptr;
}

void LI_Pin::setVisible(bool visible) {
    visible_ = visible;
    getGraphicItem()->setVisible(visible_);

    // if (binding_layer_ && (!visible_)) {
    //     binding_layer_->refreshBoundSize();
    //     binding_layer_->refreshItemMap();
    //     binding_layer_->update();
    // }
}

void LI_Pin::setZ(int z) {
    z_ = z + (int)(LayerZ::kRouting);
    item_->setZValue(z_);
}

void LI_Pin::preDraw() {
    if (!visible_) {
        return;
    }
    refreshBoundSize();
    refreshItemMap();

    auto factor = *scale_factor_;

    setOffset(-selected_area_.getLLX(), -selected_area_.getLLY());
    for (auto& obj_v : obj_vectors_) {
        for (auto obj : obj_v) {
            drawPins(static_cast<open_edi::db::Pin*>(obj));

            // auto obj_type = obj->getObjectType();

            // switch (obj_type) {
            // case open_edi::db::ObjectType::kObjectTypeInst:
            //     drawPins(*static_cast<open_edi::db::Inst*>(obj));
            //     break;
            // case open_edi::db::ObjectType::kObjectTypePin:
            //     break;

            // default:
            //     break;
            // }
        }
    }

    for (auto& obj : obj_vectors_) {
        obj.clear();
    }
}

} // namespace gui
} // namespace open_edi