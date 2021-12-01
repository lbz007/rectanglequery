#include "world_view.h"
#include <QColor>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QLineF>
#include <QPoint>

namespace open_edi {
namespace gui {

WorldSence::WorldSence(QObject* parent) {
    selection_rect_item_ = new QGraphicsRectItem;
    selection_rect_item_->setPen(QPen(QColor("green"), 0));
    selection_rect_item_->setBrush(QBrush(QColor(0, 200, 0, 80), Qt::SolidPattern));
    selection_rect_item_->setZValue(1);
    QGraphicsScene::addItem(selection_rect_item_);

    die_area_rect_item_ = new QGraphicsRectItem;
    die_area_rect_item_->setPen(QPen(QColor(120, 170, 255, 255), 0));
    die_area_rect_item_->setBrush(QBrush(QColor(150, 200, 255, 80), Qt::SolidPattern));
    die_area_rect_item_->setZValue(1);
    QGraphicsScene::addItem(die_area_rect_item_);

    focus_item_w_ = new QGraphicsLineItem;
    focus_item_w_->setVisible(true);
    focus_item_w_->setPen(QPen(QColor("green")));
    focus_item_w_->setVisible(true);
    focus_item_w_->setZValue(2);
    QGraphicsScene::addItem(focus_item_w_);

    focus_item_h_ = new QGraphicsLineItem;
    focus_item_h_->setVisible(true);
    focus_item_h_->setPen(QPen(QColor("green")));
    focus_item_h_->setVisible(true);
    focus_item_h_->setZValue(2);
    QGraphicsScene::addItem(focus_item_h_);
}

void WorldSence::setSelectionRect(QPointF p1, QPointF p2) {
    selection_rect_item_->setRect(QRectF(p1, p2));
}

void WorldSence::setSelectionRect(QRectF rect) {
    selection_rect_item_->setRect(rect);
}

void WorldSence::setFocusPoint(QPointF p) {
    QPointF p1;
    p1.setX(p.rx() - 15);
    p1.setY(p.ry());
    QPointF p2;
    p2.setX(p.rx() + 15);
    p2.setY(p.ry());
    QLineF line_h(p1, p2);
    focus_item_w_->setLine(line_h);

    p1.setX(p.rx());
    p1.setY(p.ry() - 15);
    p2.setX(p.rx());
    p2.setY(p.ry() + 15);
    QLineF line_v(p1, p2);
    focus_item_h_->setLine(line_v);
}

void WorldSence::refreshDieAreaRect() {
    die_area_rect_item_->setRect(QRectF(-width() / 2, -height() / 2, width(), height()));
}

void WorldSence::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QBrush(QColor("#393939")));
}

WorldView::WorldView() {
    scene_ = new WorldSence(this);

    setScene(scene_);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void WorldView::initDieArea(int w, int h) {
    die_area_w_ = w, die_area_h_ = h;
    __caculateScaleFactor(width(), height());
    scene_->refreshDieAreaRect();
}

void WorldView::slotSyncPosition(open_edi::db::Box area) {
    __translateToSceneSize(area);
}

void WorldView::resizeEvent(QResizeEvent* event) {
    __caculateScaleFactor(event->size().width(), event->size().height());
    scene_->refreshDieAreaRect();
    QGraphicsView::resizeEvent(event);
}

ScaleFactor WorldView::__caculateScaleFactor(int w, int h) {
    ScaleFactor sw = w / (decltype(s_))die_area_w_;
    ScaleFactor sh = h / (decltype(s_))die_area_h_;

    s_ = sw > sh ? sh : sw;

    auto width  = die_area_w_ * s_;
    auto height = die_area_h_ * s_;

    //set scene and item size
    scene_->setSceneRect((-width) / 2,
                         (-height) / 2,
                         width,
                         height);

    return s_;
}

void WorldView::__translateToSceneSize(open_edi::db::Box area) {

    auto box = __translateDbBoxToQtBox(area);
    auto lux = -scene_->width() / 2 + box.getLUX();
    auto luy = -scene_->height() / 2 + box.getLUY();

    if (box.getWidth() == 0 || box.getHeight() == 0) {
        return;
    }

    QRectF rect(lux, luy, box.getWidth(), box.getHeight());

    scene_->setSelectionRect(rect);
    auto x = -scene_->width() / 2 + (box.llx + box.getWidth() / 2);
    auto y = -scene_->height() / 2 + (box.ury + box.getHeight() / 2);
    scene_->setFocusPoint(QPointF(x, y));

    selected_box_ = area;
    drawed_box_   = rect;
}

open_edi::db::Box WorldView::__translateFromPoint(QPoint point) {
    auto scene_point = mapToScene(point);

    auto offset_x = (scene_point.rx() + scene_->width() / 2);
    auto offset_y = (-scene_point.ry() + scene_->height() / 2);

    auto width  = drawed_box_.width();
    auto height = drawed_box_.height();
    drawed_box_.setX(-drawed_box_.width() / 2 + scene_point.rx());
    drawed_box_.setY(-drawed_box_.height() / 2 + scene_point.ry());
    drawed_box_.setWidth(width);
    drawed_box_.setHeight(height);

    auto llx = qFloor((offset_x - width / 2) / s_);
    auto lly = qFloor((offset_y - height / 2) / s_);
    auto urx = llx + selected_box_.getWidth();
    auto ury = lly + selected_box_.getHeight();

    selected_box_.setLLX(llx);
    selected_box_.setLLY(lly);
    selected_box_.setURX(urx);
    selected_box_.setURY(ury);

    scene_->setSelectionRect(drawed_box_);

    emit sendSelectedBox(selected_box_);

    return selected_box_;
}

void WorldView::mouseMoveEvent(QMouseEvent* event) {
    QPoint point = event->pos();

    __translateFromPoint(point);
    scene_->setFocusPoint(mapToScene(point));
}

void WorldView::mousePressEvent(QMouseEvent* event) {
    QPoint point = event->pos();

    __translateFromPoint(point);
    scene_->setFocusPoint(mapToScene(point));
}
} // namespace gui
} // namespace open_edi