#include "li_wire.h"

#include <QDebug>

#include "../layout_def.h"
#include "li_layer.h"

namespace open_edi {
namespace gui {
LI_Wire::LI_Wire(ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_Wire;
    item_->setLiBase(this);
    pen_.setColor(QColor(0xff, 0, 0, 0xff));
    brush_   = QBrush(QColor(0xff, 0, 0, 0xff), Qt::Dense5Pattern);
    type_    = ObjType::kWire;
    name_    = kLiWireName;
    visible_ = true;
    setZ(0);
}

LI_Wire::~LI_Wire() {
}

LGI_Wire* LI_Wire::getGraphicItem() {
    return item_;
}

void LI_Wire::draw(QPainter* painter) {
    painter->setPen(pen_);
    painter->setBrush(brush_);
    LI_Base::draw(painter);
}

void LI_Wire::drawWires(open_edi::db::Wire& wire) {

    auto layer = dynamic_cast<LI_Layer*>(li_mgr_->getLayerByName(wire.getLayer()->getName()));

    if (!layer->isVisible()) {
        return;
    }

    QPainter painter(layer->getGraphicItem()->getMap());
    pen_.setColor(layer->getColor());
    brush_.setColor(layer->getColor());

    auto box = __translateDbBoxToQtBox(wire.getBBox());
    painter.setBrush(brush_);
    painter.setPen(pen_);
    __fillBox(&painter, brush_, box);
    __drawBox(&painter, box);
}

void LI_Wire::drawWires(open_edi::db::Wire* wire) {
    // auto layer = dynamic_cast<LI_Layer*>(li_mgr_->getLayerByName(wire->getLayer()->getName()));
    // if (!layer->isVisible()) {
    //     return;
    // }
    // layer->obj_vectors_.push_back(wire);
}

bool LI_Wire::isMainLI() {
    return false;
}

void LI_Wire::setZ(int z) {
    z_ = z + (int)(LayerZ::kBase);
    item_->setZValue(z_);
} // namespace gui

void LI_Wire::preDraw() {

    auto net_v = li_mgr_->getLiByName(kLiNetName)->isVisible();
    // if (!net_v) {
    //     return;
    // }
    // if (!visible_) {
    //     return;
    // }
    refreshBoundSize();
    refreshItemMap();

    std::vector<open_edi::db::Object*> result;
    open_edi::db::fetchDB(selected_area_, &result);
    auto factor = *scale_factor_;
    // printf("COMPONENTS %d ;\n", num_components);

    setOffset(-selected_area_.getLLX(), -selected_area_.getLLY());
    open_edi::db::Wire* wire;

    QPainter painter;
    for (auto obj : result) {
        auto obj_type = obj->getObjectType();
        if (open_edi::db::ObjectType::kObjectTypeWire == obj_type) {
            wire = static_cast<open_edi::db::Wire*>(obj);
            // drawWires(*wire);
            drawWires(wire);
        }

        if (open_edi::db::ObjectType::kObjectTypeVia == obj_type) {
            qDebug() << "via";
        }
    }
}

} // namespace gui
} // namespace open_edi