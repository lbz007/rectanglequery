#include "li_highlight.h"

#include <QPainter>

#include "../layout_def.h"
#include "li_layer.h"
#include "li_wire.h"

namespace open_edi {
namespace gui {

LI_HighLight::LI_HighLight(ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_HightLight;
    item_->setLiBase(this);
    pen_.setColor(QColor("White"));
    pen_.setWidth(1);
    name_ = kLiHighlightName;
    type_ = ObjType::kHighlight;
    // li_mgr_->addLI(this);
    setVisible(true);
    item_->setVisible(true);
    setZ(0);
}

LI_HighLight::~LI_HighLight() {
}

void LI_HighLight::preDraw() {
    refreshBoundSize();

    auto map = getGraphicItem()->getMap();

    // QPainter painter(refreshItemMap());

    auto factor = *scale_factor_;

    std::vector<open_edi::db::Object*> result;
    open_edi::db::fetchDB(selected_area_, &result);
    int                   top_layer = 0;
    open_edi::db::Object* ret       = nullptr;

    if (result.size()) {
        QColor color("#393939");
        color.setAlpha(mask_value_);
        map->fill(color);
    } else {
        refreshItemMap();
    }

    QPainter painter(map);
    painter.setPen(pen_);

    //for debug
    // auto selected_box = __translateDbBoxToQtBox(selected_area_, getSceneHight());
    // painter.drawRect(QRectF(
    //   (selected_box.getLUX()),
    //   (selected_box.getLUY()),
    //   selected_box.getWidth(),
    //   selected_box.getHeight()));

    for (auto obj : result) {
        if (obj->getObjectType() == open_edi::db::ObjectType::kObjectTypeWire) {
            auto li = li_mgr_->getLiByName(static_cast<open_edi::db::Wire*>(obj)->getLayer()->getName());
            if (li->isVisible() && li->getZ() >= top_layer) {
                top_layer = li->getZ();
                ret       = obj;
            }
        } else if (obj->getObjectType() == open_edi::db::ObjectType::kObjectTypeInst) {
            auto li = li_mgr_->getLiByName(kLiInstanceName);
            if (li->isVisible() && li->getZ() >= top_layer) {
                top_layer = li->getZ();
                ret       = obj;
            }
        }
    }

    if (ret) {
        auto obj_type = ret->getObjectType();
        if (open_edi::db::ObjectType::kObjectTypeInst == obj_type) {
            auto obj        = static_cast<open_edi::db::Inst*>(ret);
            auto box        = obj->getBox();
            auto screen_box = __translateDbBoxToQtBox(box);

            __drawBox(&painter, screen_box);

            // if (screen_box.getWidth() && screen_box.getHeight()) {

            //     painter.drawRect(QRectF(
            //       (screen_box.getLUX()),
            //       (screen_box.getLUY()),
            //       screen_box.getWidth(),
            //       screen_box.getHeight()));
            // }
        }

        else if (open_edi::db::ObjectType::kObjectTypeWire == obj_type) {
            auto obj = static_cast<open_edi::db::Wire*>(ret);

            auto net = obj->getNet();
            if (net) {
                auto array = net->getWireArray();
                if (array) {
                    for (auto id : *array) {
                        auto wire    = open_edi::db::Object::addr<open_edi::db::Wire>(id);
                        auto box     = wire->getBBox();

                        // auto head = wire->getHead();
                        // auto tail = wire->getTail();
                        if (wire->isVertical()) {
                            auto lly = box.getLLY();
                            lly      = lly - box.getWidth() / 2;
                            auto ury = box.getURY();
                            ury      = ury + box.getWidth() / 2;
                            box.setLLY(lly);
                            box.setURY(ury);
                        } else {
                            auto llx = box.getLLX();
                            llx      = llx - box.getHeight() / 2;
                            auto urx = box.getURX();
                            urx      = urx + box.getHeight() / 2;
                            box.setLLX(llx);
                            box.setURX(urx);
                        }
                        auto screen_box = __translateDbBoxToQtBox(box);

                        // if (box.getWidth() && box.getHeight()) {
                        auto li_layer = dynamic_cast<LI_Layer*>(li_mgr_->getLayerByName(wire->getLayer()->getName()));
                        auto li_wire  = dynamic_cast<LI_Wire*>(li_mgr_->getLiByName(kLiWireName));

                        auto brush = li_wire->getBrush();
                        brush.setColor(li_layer->getColor());
                        painter.setBrush(brush);
                        __fillBox(&painter, brush, screen_box);
                        // painter.fillRect(QRectF(
                        //                    (screen_box.getLUX()),
                        //                    (screen_box.getLUY()),
                        //                    screen_box.getWidth(),
                        //                    screen_box.getHeight()),
                        //                  brush);

                        painter.setPen(li_layer->getPen());
                        __drawBox(&painter, screen_box);
                        // painter.drawRect(QRectF(
                        //   (screen_box.getLUX()),
                        //   (screen_box.getLUY()),
                        //   screen_box.getWidth(),
                        //   screen_box.getHeight()));

                        auto color = pen_.color();
                        color.setAlpha(255 - mask_value_);
                        QPen pen(color);
                        painter.setPen(pen);
                        // painter.drawRect(QRectF(
                        //   (screen_box.getLUX()),
                        //   (screen_box.getLUY()),
                        //   screen_box.getWidth(),
                        //   screen_box.getHeight()));
                        __drawBox(&painter, screen_box);
                        // }
                    }
                }
            }
        }
    }
}

LGI_HightLight* LI_HighLight::getGraphicItem() {
    return item_;
}

void LI_HighLight::setZ(int z) {
    z_ = (int)(LayerZ::kHighlightItemZ);
    item_->setZValue(z_);
}

void LI_HighLight::draw(QPainter* painter) {
    painter->setPen(pen_);
    LI_Base::draw(painter);
}

} // namespace gui
} // namespace open_edi