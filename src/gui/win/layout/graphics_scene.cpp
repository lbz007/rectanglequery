
#include "graphics_scene.h"
#include "layout_def.h"

namespace open_edi {
namespace gui {

GraphicsScene::GraphicsScene(QObject* parent) {
    selectionRectItem_ = new QGraphicsRectItem;
    selectionRectItem_->setPen(QPen(QColor(120, 170, 255, 255), 0));
    selectionRectItem_->setBrush(QBrush(QColor(150, 200, 255, 80), Qt::SolidPattern));
    selectionRectItem_->setZValue((qreal)LayerZ::kSelectionItemZ);
    QGraphicsScene::addItem(selectionRectItem_);
}

GraphicsScene::~GraphicsScene() {
}

void GraphicsScene::setSelectionRect(QPointF p1, QPointF p2) {
    selectionRectItem_->setRect(QRectF(p1, p2));
}

void GraphicsScene::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QBrush(QColor("#393939")));
}

} // namespace gui
} // namespace open_edi
