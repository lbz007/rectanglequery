#include "li_manager.h"
#include <QDebug>
#include <QThreadPool>
#include <QTime>
#include "li_base.h"
#include "li_die_area.h"
#include "li_highlight.h"
#include "li_instance.h"
#include "li_layer.h"
#include "li_net.h"
#include "li_wire.h"

namespace open_edi {
namespace gui {

#define USE_MUTI_THREAD 1

void DataReader::run() {
    std::vector<open_edi::db::Object*> result;

    open_edi::db::fetchDB(box_, &result);

    for (auto obj : result) {
        auto obj_type = obj->getObjectType();
        if (open_edi::db::ObjectType::kObjectTypeWire == obj_type) {
            // auto li_wire = dynamic_cast<LI_Wire*>(li_manger_->getLiByName(kLiWireName));
            if (!li_net_->isVisible()) {
                continue;
            }
            if (!li_wire_->isVisible()) {
                continue;
            }
            auto wire  = static_cast<open_edi::db::Wire*>(obj);
            auto layer = dynamic_cast<LI_Layer*>(li_manger_->getLayerByName(wire->getLayer()->getName()));
            if (!layer->isVisible()) {
                continue;
            }
            layer->getObjVector(index_)->push_back(obj);
        }
        if (open_edi::db::ObjectType::kObjectTypeInst == obj_type) {
            if (!li_inst_->isVisible()) {
                continue;
            }
            li_inst_->getObjVector(index_)->push_back(obj);
            if (!li_pin_->isVisible()) {
                continue;
            }
            auto ins  = static_cast<open_edi::db::Inst*>(obj);
            auto pins = ins->getPins();
            if (!pins) {
                continue;
            }

            auto pins_vector = open_edi::db::Object::addr<open_edi::db::ArrayObject<open_edi::db::ObjectId>>(pins);
            for (auto iter = pins_vector->begin(); iter != pins_vector->end(); ++iter) {
                auto pin = open_edi::db::Object::addr<open_edi::db::Pin>(*iter);
                if (pin) {
                    li_pin_->getObjVector(index_)->push_back(pin);
                }
            }
        }
    }
}

LI_Manager::LI_Manager() {
    int index = 0;
    for (auto& reader : readers_) {
        reader.setLiManager(this);
        reader.setIndex(index++);
    }
}

void LI_Manager::__split_area() {
    QTime time;
    time.start();
    auto width    = selected_box_.getWidth();
    auto height   = selected_box_.getHeight();
    auto offset_x = selected_box_.getLLX();
    auto offset_y = selected_box_.getLLY();

    auto li_die_area = static_cast<LI_DieArea*>(getLiByName(kLiDieAreaName));

    width    = width > li_die_area->getDieW() ? li_die_area->getDieW() : width;
    height   = height > li_die_area->getDieH() ? li_die_area->getDieH() : height;
    offset_x = offset_x > 0 ? offset_x : 0;
    offset_y = offset_y > 0 ? offset_y : 0;

    //keep divisibility
    width += width % kSplitPart;
    height += height % kSplitPart;
    auto slice_w = width / kSplitPart;
    auto slice_h = height / kSplitPart;

    int index = 0;
    for (int i = 0; i < kSplitPart; i++) {
        for (int j = 0; j < kSplitPart; j++) {
            auto llx = offset_x + slice_w * j;
            auto lly = offset_y + slice_h * i;
            auto urx = llx + slice_w;
            auto ury = lly + slice_h;

            open_edi::db::Box slice_area(llx, lly, urx, ury);
            // readers_[index].setIndex(index);
            readers_[index].setBox(slice_area);
            index++;
        }
    }

    for (auto& reader : readers_) {
        QThreadPool::globalInstance()->start(&reader);
    }
    QThreadPool::globalInstance()->waitForDone();

    auto time_elapsed = time.elapsed();
    printf("spliting total elapsed time %d (ms)\n", time_elapsed);
}

void LI_Manager::preDrawAllItems() {

    __split_area();

    //refresh all routing layers
    for (auto layer : getRoutingLayersList()) {
        layer->refreshBoundSize();
        layer->refreshItemMap();
    }

    QTime time;
    time.start();

    //draw all items
    for (auto item : getLiList()) {
        if (item->isMainLI()) {
            QThreadPool::globalInstance()->start(item);
        }
    }

    QThreadPool::globalInstance()->waitForDone();
    auto time_elapsed = time.elapsed();
    printf("muti thread total elapsed time %d (ms)\n", time_elapsed);
}

QList<LI_Base*> LI_Manager::getLiList() {
    QList<LI_Base*> li_list;
    for (auto li : li_map_) {
        li_list.append(li);
    }

    return li_list;
}

void LI_Manager::setLiVisibleByName(QString name, bool v) {

    if (li_map_.find(name) != li_map_.end()) {
        auto li = li_map_[name];
        li->setVisible(v);
    }
}

void LI_Manager::setLiBrushStyleByName(QString name, Qt::BrushStyle brush_style) {
    if (li_map_.find(name) != li_map_.end()) {
        auto li = li_map_[name];
        li->setBrushStyle(brush_style);
    }
}

LI_Base* LI_Manager::getLiByName(QString name) {
    if (li_map_.find(name) != li_map_.end()) {
        return li_map_[name];
    }
    return nullptr;
}

void LI_Manager::addLayer(open_edi::db::Layer* layer, ScaleFactor* scale_factor) {
    auto li_layer = new LI_Layer(layer, scale_factor);
    createLI(li_layer);
    if (layer->isRouting()) {
        routing_layer_list_.append(li_layer);
    }
}

void LI_Manager::setLayerVisibleByName(QString name, bool v) {
}

void LI_Manager::setLayerColorByName(QString name, QColor color) {
}

LI_Base* LI_Manager::getLayerByName(QString name) {
    auto li = getLiByName(name);
    if (li) {
        if (li->type() == LI_Base::ObjType::kLayer) {
            return li;
        }
    }

    return nullptr;
}

QList<LI_Layer*> LI_Manager::getLayerList() {
    QList<LI_Layer*> list;
    for (auto li : getLiList()) {
        if (li->type() == LI_Base::ObjType::kLayer) {
            list.append(dynamic_cast<LI_Layer*>(li));
        }
    }

    return list;
}

/**
 * LI_Manager 
 * set selected area to db, save objects to each LI 
 * 
 * @param  {QPointF} p1 : db uint coordinate of low left point
 * @param  {QPointF} p2 : db uint coordinate of up right point
 */
void LI_Manager::setSelectedArea(QPointF p1, QPointF p2) {

    open_edi::db::Box area(p1.rx(), p1.ry(), p2.rx(), p2.ry());

    setSelectedArea(area);
}

void LI_Manager::setSelectedArea(int x, int y, int w, int h) {

    open_edi::db::Box area(x, y, x + w, y + h);

    setSelectedArea(area);
}

void LI_Manager::setSelectedArea(open_edi::db::Box area) {

    selected_box_ = area;
    for (auto item : getLiList()) {
        if (item->isMainLI()) {
            if (item->isHighlightLI()) {
                auto li_highlight = dynamic_cast<LI_HighLight*>(item);
                li_highlight->setOffsetX(-area.getLLX());
                li_highlight->setOffsetY(-area.getLLY());
            } else {
                item->setSelectedArea(area);
            }
        } else {
            // if (item->type() == LI_Base::ObjType::kWire) {
            //     item->setSelectedArea(area);
            // }
        }
    }
}

void LI_Manager::setHighlightSelectionArea(open_edi::db::Box area) {
    auto li_highlight = dynamic_cast<LI_HighLight*>(getHighlightLI());
    li_highlight->setOffsetX(-getAnchorLLX());
    li_highlight->setOffsetY(-getAnchorLLY());
    li_highlight->setSelectedArea(area);
    li_highlight->preDraw();
    li_highlight->update();
}

void LI_Manager::resetSelectedBox() {
    selected_box_.setLLX(0);
    selected_box_.setLLY(0);
    auto max_width = dynamic_cast<LI_DieArea*>(getLiByName(kLiDieAreaName))->getDieW();
    selected_box_.setURX(max_width);
    auto max_height = dynamic_cast<LI_DieArea*>(getLiByName(kLiDieAreaName))->getDieH();
    selected_box_.setURY(max_height);
}

LI_Base* LI_Manager::getHighlightLI() {
    return getLiByName(kLiHighlightName);
}

LI_Manager* LI_Manager::inst_ = nullptr;
} // namespace gui
} // namespace open_edi