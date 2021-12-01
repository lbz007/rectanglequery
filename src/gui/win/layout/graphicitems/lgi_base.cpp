#include <QPainter>
#include <QTime>

#include "lgi_base.h"

namespace open_edi {
namespace gui {

LGI_Base::LGI_Base(/* args */) {
}

LGI_Base::~LGI_Base() {
}

void LGI_Base::setItemSize(int w, int h) {
    this->w = w;
    this->h = h;
}

void LGI_Base::setMap(QPixmap* img) {
    this->img_ = img;
}

void LGI_Base::setLiBase(LI_Base* li_base) {
    this->li_base_ = li_base;
}

void LGI_Base::resizeMap(int w, int h) {
    if (img_) {
        if (w != img_->width() || h != img_->height()) {
            delete img_;
            img_ = new QPixmap(w, h);
        }
    } else {
        img_ = new QPixmap(w, h);
    }
}

QPixmap* LGI_Base::fillMapToTransparency() {
    if (img_) {
        img_->fill(Qt::transparent);
    }
    return img_;
}

QRectF LGI_Base::boundingRect() const {
    return QRectF(0, 0, w, h);
}

void LGI_Base::paint(QPainter*                       painter,
                     const QStyleOptionGraphicsItem* option,
                     QWidget*                        widget) {
    // QTime time;
    // time.start();

    li_base_->draw(painter);
    // auto time_elapsed = time.elapsed();
    // printf("paint elapsed time %d (ms)\n", time_elapsed);
}

} // namespace gui
} // namespace open_edi
