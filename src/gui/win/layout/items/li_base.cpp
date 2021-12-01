#include "li_base.h"

#include <QDebug>

#include "../graphicitems/lgi_base.h"

namespace open_edi {
namespace gui {

void LI_Base::refreshBoundSize() {
    if (scene_w_ != getGraphicItem()->scene()->width()
        || scene_h_ != getGraphicItem()->scene()->height()) {
        scene_w_ = getGraphicItem()->scene()->width();
        scene_h_ = getGraphicItem()->scene()->height();

        QTransform matrix;
        matrix.translate(-(scene_w_ >> 1), -(scene_h_ >> 1));
        getGraphicItem()->setTransform(matrix);
#if DRAW_MODE == 1
        getGraphicItem()->resizeMap(scene_w_, scene_h_);
#endif
    }

    getGraphicItem()->setItemSize(scene_w_, scene_h_);
}

void LI_Base::setVisible(bool visible) {
    visible_ = visible;
    getGraphicItem()->setVisible(visible_);
}

void LI_Base::draw(QPainter* painter) {
#if DRAW_MODE == 1
    // qDebug()<<name_;
    auto img = getGraphicItem()->getMap();
    if (img && isVisible()) {
        painter->drawPixmap(0, 0, *img);
    }
#elif DRAW_MODE == 2
    painter->drawPath(painter_path_);
#endif
    auto draw_mode = DRAW_MODE;
    // printf("draw mode %s\n",
    //        draw_mode == IMG_MODE ? "image mode" :
    //                                draw_mode == PATH_MODE ? "path mode" :
    //                                                         "no draw mode");

    
}

void LI_Base::update() {
    getGraphicItem()->update();
}

QString LI_Base::getName() {
    return name_;
}

void LI_Base::run() {
    preDraw();
}

void LI_Base::setSelectedArea(int llx, int lly, int urx, int ury) {

    selected_area_.setLLX(llx);
    selected_area_.setLLY(lly);
    selected_area_.setURX(urx);
    selected_area_.setURY(ury);
}

QPixmap* LI_Base::refreshItemMap() {
    return getGraphicItem()->fillMapToTransparency();
}

void LI_Base::fetchDB(open_edi::db::Box area, std::vector<open_edi::db::Object*>* result) {
    open_edi::db::fetchDB(selected_area_, result);
}

void LI_Base::__drawBox(QPainter* painter, int lux, int luy, int w, int h) {
    if (w == 0 && h == 0) {
        return;
    }
    painter->drawRect(lux, luy, w, h);
}

void LI_Base::__drawBox(QPainter* painter, LGIBox& box) {

    if (box.getWidth() == 0 && box.getHeight() == 0) {
        return;
    }

    painter->drawRect(box.getLUX(), box.getLUY(), box.getWidth(), box.getHeight());
}

void LI_Base::__fillBox(QPainter* painter, QBrush& brush, LGIBox& box) {
    if (box.getWidth() == 0 && box.getHeight() == 0) {
        return;
    }
    painter->fillRect(QRect(box.getLUX(), box.getLUY(), box.getWidth(), box.getHeight()), brush_);
}

void LI_Base::__fillBox(QPainter* painter, QBrush& brush, int lux, int luy, int w, int h) {

    if (w == 0 && h == 0) {
        return;
    }

    painter->fillRect(QRect(lux, luy, w, h), brush_);
}


} // namespace gui
} // namespace open_edi