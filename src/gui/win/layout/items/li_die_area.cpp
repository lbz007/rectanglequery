#include "li_die_area.h"

#include <QDebug>
#include <QPainter>

#include "../layout_def.h"

namespace open_edi {
namespace gui {

LI_DieArea::LI_DieArea(ScaleFactor* scale_factor) : LI_Base(scale_factor) {
    item_ = new LGI_DieArea;
    item_->setLiBase(this);
    pen_.setColor(QColor("#909090"));
    name_ = kLiDieAreaName;
    type_ = ObjType::kDieArea;
    // li_mgr_->addLI(this);
    setVisible(true);
    item_->setVisible(true);
    setZ(0);
}

LI_DieArea::~LI_DieArea() {
}

void LI_DieArea::preDraw() {
    refreshBoundSize();
#if DRAW_MODE == 1
    QPainter painter(refreshItemMap());
    painter.setPen(pen_);

    auto factor = *scale_factor_;

    offset_x_ = -selected_area_.getLLX();
    offset_y_ = -selected_area_.getLLY();

    open_edi::db::Box box(0, 0, die_area_w_, die_area_h_);

    auto q_box = __translateDbBoxToQtBox(box);

    __drawBox(&painter, q_box);

#elif DRAW_MODE == 2
    painter_path_ = QPainterPath();
    painter_path_.addRect(QRectF(0, 0, scene_w_, scene_h_));
#endif
}

LGI_DieArea* LI_DieArea::getGraphicItem() {
    return item_;
}

bool LI_DieArea::isMainLI() {
    return true;
}

void LI_DieArea::setZ(int z) {
    z_ = z + (int)(LayerZ::kBase);
    item_->setZValue(z_);
}

void LI_DieArea::draw(QPainter* painter) {
    painter->setPen(pen_);
    LI_Base::draw(painter);
}

} // namespace gui
} // namespace open_edi