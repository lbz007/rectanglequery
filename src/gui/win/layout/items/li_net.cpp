#include "li_net.h"

#include "../layout_def.h"


namespace open_edi {
namespace gui {

LI_Net::LI_Net(ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_Net;
    item_->setLiBase(this);
    type_     = ObjType::kNet;
    name_    = kLiNetName;
    setVisible(true);
    setZ(0);
}

LI_Net::~LI_Net() {
}

LGI_Net* LI_Net::getGraphicItem() {
    return item_;
}

bool LI_Net::hasSubLI() {
    return true;
}

bool LI_Net::isMainLI() {
    return false;
}

void LI_Net::setVisible(bool visible) {
    visible_ = visible;
    item_->setVisible(visible);
    // li_wires->setVisible(visible);
}

void LI_Net::setZ(int z) {
    z_ = z + (int)(LayerZ::kBase);
    item_->setZValue(z_);
}

void LI_Net::draw(QPainter* painter) {
    painter->setPen(pen_);
    LI_Base::draw(painter);
}

void LI_Net::preDraw() {
    if (!visible_) {
        return;
    }

    // std::vector<open_edi::db::Object*> result;
    // open_edi::db::fetchDB(selected_area_, &result);

    // auto factor = *scale_factor_;

    // li_wires->refreshBoundSize();

    // QPainter painter(refreshItemMap());
    // painter.setPen(pen_);
    // painter.setWindow(0,
    //                   scene_h_,
    //                   scene_w_ + VIEW_SPACE,
    //                   -scene_h_ - VIEW_SPACE);

    // auto     tc       = open_edi::db::getTopCell();
    // uint64_t num_nets = tc->getNumOfNets();
    // // printf("Nets %d ;\n", num_nets);

    // auto factor = *scale_factor_;

    // auto arr_ptr = tc->getNetArray();
    // if (arr_ptr) {
    //     for (auto iter = arr_ptr->begin(); iter != arr_ptr->end(); iter++) {
    //         auto net = open_edi::db::Object::addr<open_edi::db::Net>(*iter);
    //         if (!net) continue;
    //         // printf("net in\n");
    //         // counting wire
    //         if (li_wires->isVisible()) {
    //             auto wire_array = net->getWireArray();
    //             if (wire_array) {
    //                 // printf("wire_array in\n");
    //                 for (auto wire_iter = wire_array->begin(); wire_iter != wire_array->end(); ++wire_iter) {
    //                     auto wire = open_edi::db::Object::addr<open_edi::db::Wire>(*wire_iter);
    //                     li_wires->drawWires(*wire);
    //                 }
    //             }
    //         }

    //         // printf("net in\n");
    //     }
    // }
}

} // namespace gui
} // namespace open_edi