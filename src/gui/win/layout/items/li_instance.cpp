#include "li_instance.h"

#include <QDebug>

#include "../layout_def.h"

namespace open_edi {
namespace gui {

LI_Instance::LI_Instance(ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_Instance;
    item_->setLiBase(this);
    pen_.setColor(QColor("#909090"));
    brush_.setColor(pen_.color());
    brush_.setStyle(Qt::Dense7Pattern);
    type_ = ObjType::kInstance;
    name_ = kLiInstanceName;
    // li_mgr_->addLI(this);

    //defualt visible
    visible_ = true;
    setZ(0);
}

LI_Instance::~LI_Instance() {
}

void LI_Instance::setVisible(bool visible) {
    visible_ = visible;
    getGraphicItem()->setVisible(visible_);
}

LGI_Instance* LI_Instance::getGraphicItem() {
    return item_;
}

bool LI_Instance::isMainLI() {
    return true;
}

void LI_Instance::draw(QPainter* painter) {
    painter->setPen(pen_);
    LI_Base::draw(painter);
}

void LI_Instance::preDraw() {

    if (!visible_) {
        return;
    }

    refreshBoundSize();
    QPainter painter(refreshItemMap());
    painter.setPen(pen_);

    auto factor = *scale_factor_;
    // printf("COMPONENTS %d ;\n", num_components);
    setOffset(-selected_area_.getLLX(), -selected_area_.getLLY());

    for (auto& obj_v : obj_vectors_) {
        for (auto obj : obj_v) {
            auto obj_type = obj->getObjectType();
            if (open_edi::db::ObjectType::kObjectTypeInst == obj_type) {
                auto instance = static_cast<open_edi::db::Inst*>(obj);

                auto insbox = instance->getBox();

                auto li_box = __translateDbBoxToQtBox(insbox);
                auto insllx = li_box.getLLX();
                auto inslly = li_box.getLLY();
                auto insurx = li_box.getURX();
                auto insury = li_box.getURY();
                auto width  = li_box.getWidth();
                auto height = li_box.getHeight();

                if (width == 0 || height == 0) {
                    continue;
                }

                __fillBox(&painter, brush_, li_box);
                __drawBox(&painter, li_box);

                // if (width >= (4) && height >= (4)) {
                //     li_pins->drawPins(*instance);
                // }

                if (width >= (4) && height >= (4)) {

                    switch (instance->getOrient()) {
                    case open_edi::util::Orient::kN /* North, R0 in OpenAccess */:
                    case open_edi::util::Orient::kFW /* Flipped West, MX90 in OpenAccess */:
                        painter.drawLine(QPoint(insllx, inslly - (height >> 2)), QPoint(insllx + (width >> 2), inslly));
                        break;
                    case open_edi::util::Orient::kS /* South, R180 in OpenAcces */:
                    case open_edi::util::Orient::kFE /* Flipped East, MY90 in OpenAccess */:
                        painter.drawLine(QPoint(insurx, insury + (height >> 2)), QPoint(insurx - (width >> 2), insury));
                        break;
                    case open_edi::util::Orient::kW /* East, R270 in OpenAccess */:
                    case open_edi::util::Orient::kFS /* Flipped South, MX in OpenAccess */:
                        painter.drawLine(QPoint(insllx, insury + (height >> 2)), QPoint(insllx + (width >> 2), insury));
                        break;
                    case open_edi::util::Orient::kE /* West, R90 in OpenAccess */:
                    case open_edi::util::Orient::kFN /* Flipped north, MY in OpenAccess */:
                        painter.drawLine(QPoint(insurx, inslly - (height >> 2)), QPoint(insurx - (width >> 2), inslly));
                        break;

                    default:
                        printf("error: instance orient unknow\n");
                        break;
                    }
                }

                if (width >= (16) && height >= (16)) {
                    //draw name
                    auto name = instance->getName().c_str();
                    // painter.drawText(insllx + (width >> 2), inslly + (height >> 1), QString(name));
                    painter.drawText(QRectF(
                                       (insllx),
                                       (insury),
                                       width,
                                       height),
                                     0,
                                     name);
                }
            }
        }
    }

    for (auto& obj : obj_vectors_) {
        obj.clear();
    }
}

bool LI_Instance::hasSubLI() {
    return true;
}

void LI_Instance::setZ(int z) {
    z_ = z + (int)(LayerZ::kBase);
    item_->setZValue(z_);
}

} // namespace gui
} // namespace open_edi
