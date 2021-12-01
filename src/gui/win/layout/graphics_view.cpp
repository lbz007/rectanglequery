#include "graphics_view.h"

#include <QGuiApplication>
#include <QSizePolicy>

#include "../dialog/inst_attribute_editor.h"
#include "../dialog/wire_attribute_editor.h"

namespace open_edi {
namespace gui {

GraphicsView::GraphicsView(QWidget* parent) : QGraphicsView(parent) {

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setMouseTracking(true);
    layout_ = LAYOUT_INSTANCE;
    layout_->createLayoutItems();

    scene_ = new GraphicsScene;

    setScene(scene_);

#if USE_OPENGL == 1
    setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

GraphicsView::~GraphicsView() {
}

void GraphicsView::__zoom(qreal value) {
    if (mouse_status_ == MouseStatus::kInitial) return;
    qreal s  = value;
    auto  sf = layout_->getScaleFactor();
    sf       = sf * s;
    if (sf == 0)
        return;
    layout_->resizeLayoutWindow(__getWidth(), __getHeight(), sf);
    //refresh all components
    refreshAllComponents();
    emit sendSelectedBox(layout_->getCurSelectedBox());
}
/**
 * calculate new coordinate after zoom in/out according to factor
 * 
 * @param  {int} x            : anchor x in db unit
 * @param  {int} y            : anchor y in db unit
 * @param  {int} w            : selection w in db unit
 * @param  {int} h            : selection h in db unit
 * @param  {qreal} factor     : scale factor calculated by zoom action
 * @return {std::tuple<int,}  : new x, y , w, h
 */
std::tuple<int, int, int, int> GraphicsView::__calcOriginPointOffset(int x, int y, int w, int h, qreal factor) {

    auto s = 1 * factor;

    // new width and height which needs to display
    auto p_w = w * s;
    auto p_h = h * s;

    //new x, y after offset
    auto p_x = (w - p_w) / 2 + x;
    auto p_y = (h - p_h) / 2 + y;

    return std::tuple<int, int, int, int>(p_x, p_y, p_w, p_h);
}

open_edi::db::Box GraphicsView::__adjustPointsToDbBox(QPoint p_start, QPoint p_end) {
    auto point_start = mapToScene(p_start);
    auto x1          = (point_start.rx() + scene_->width() / 2) / layout_->getScaleFactor();
    auto y1          = -(point_start.ry() - scene_->height() / 2) / layout_->getScaleFactor();

    auto point_end = mapToScene(p_end);
    auto x2        = (point_end.rx() + scene_->width() / 2) / layout_->getScaleFactor();
    auto y2        = -(point_end.ry() - scene_->height() / 2) / layout_->getScaleFactor();

    auto llx = x1 > x2 ? x2 : x1;
    auto lly = y1 > y2 ? y2 : y1;
    auto urx = x1 > x2 ? x1 : x2;
    auto ury = y1 > y2 ? y1 : y2;

    return open_edi::db::Box(llx, lly, urx, ury);
}

std::tuple<double, double> GraphicsView::__translateToDbPoint(QPoint point) {
    auto new_point = mapToScene(point);

    auto anchor_point = layout_->getAnchorPoint();
    auto llx          = std::get<0>(anchor_point);
    auto lly          = std::get<1>(anchor_point);

    auto x = (new_point.rx() + scene_->width() / 2) / layout_->getScaleFactor() + llx;
    auto y = -(new_point.ry() - scene_->height() / 2) / layout_->getScaleFactor() + lly;

    return std::tuple<double, double>(x, y);
}

open_edi::db::Box GraphicsView::__selecteAreaByPoint(QPoint point, int width, int height) {

    auto db_point = __translateToDbPoint(point);

    auto db_point_x = std::get<0>(db_point);
    auto db_point_y = std::get<1>(db_point);

    auto w = width;
    auto h = height;

    open_edi::db::Box db_box(db_point_x - w,
                             db_point_y - h,
                             db_point_x + w,
                             db_point_y + h);

    return db_box;
}

void GraphicsView::slotZoomIn(bool) {
    __zoom(0.8);
}

void GraphicsView::slotZoomOut(bool) {
    __zoom(1.2);
}
/**
 * GraphicsView 
 * refresh all components.
 * 
 */
void GraphicsView::
  refreshAllComponents() {

    //add one pix for display
    auto w = layout_->getScreenW();
    auto h = layout_->getScreenH();

    //set scene and item size
    scene_->setSceneRect((-w) / 2,
                         (-h) / 2,
                         w,
                         h);

    layout_->refreshAllComponents();

    // emit sendSelectedBox(layout_->getCurSelectedBox());

    viewport()->update();
}

void GraphicsView::refreshFitDraw() {

    auto fit_width  = (__getWidth() / 10) * 7;
    auto fit_height = (__getHeight() / 10) * 7;

    //shall set selected area first
    auto s = layout_->calculateScaleFactor(fit_width,
                                           fit_height,
                                           layout_->getDieAreaW(),
                                           layout_->getDieAreaH());

    layout_->getLiManager()->resetSelectedBox();
    layout_->resizeLayoutWindow(__getWidth(), __getHeight(), s);

    emit sendSelectedBox(layout_->getCurSelectedBox());
    refreshAllComponents();
    // viewport()->update();
}

void GraphicsView::initView() {

    // if(mouse_status_ != MouseStatus::kInitial)

    layout_->initDieArea();
    layout_->initAllLayers();

    auto list = layout_->getLiManager()->getLiList();

    for (auto li : list) {
        scene_->addItem(li->getGraphicItem());
    }
    // mouse_status_ = MouseStatus::kNormal;
}

void GraphicsView::refreshManually() {
    if (mouse_status_ == MouseStatus::kInitial) {
        emit initByCmd();
    } else {
        refreshAllComponents();
    }
}

void GraphicsView::slotReadLayer() {
    // layout->initAllLayers();
}

void GraphicsView::setPinsVisible(bool visible) {
    // layout_->li_pins->setVisible(visible);
}

void GraphicsView::slotSetHighlightMaskValue(int v) {
    layout_->setHighlightMaskLevel(v);
}

void GraphicsView::slotUpdatePosition(open_edi::db::Box area) {

    layout_->setAnchor(area.getLLX(), area.getLLY());
    refreshAllComponents();
}

void GraphicsView::wheelEvent(QWheelEvent* event) {
    if (mouse_status_ == MouseStatus::kInitial) return;
    qreal factor = qPow(1.1, event->delta() / 240.0);

    __zoom(factor);
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event) {

    if (mouse_status_ == MouseStatus::kInitial) return;

    auto point = __translateToDbPoint(event->pos());

    auto x = std::get<0>(point);
    auto y = std::get<1>(point);

    auto lib = open_edi::db::getTechLib();

    x = lib->dbuToMicrons(x);
    y = lib->dbuToMicrons(y);

    emit sendPos(x, y);

    if (mouse_status_ == MouseStatus::kAreaSelected) {
        QPoint point_moving_ = event->pos();
        scene_->setSelectionRect(mapToScene(point_start_), mapToScene(point_moving_));
    }
    if (mouse_status_ == MouseStatus::kMovingSelection) {

        setCursor(Qt::ClosedHandCursor);

        auto cur_point = event->pos();

        auto offset_x = point_start_.rx() - cur_point.rx();
        auto offset_y = point_start_.ry() - cur_point.ry();

        if ((qAbs(offset_x) > 10)
            || (qAbs(offset_y) > 10)) {

            point_start_ = cur_point;

            auto db_offset_x = offset_x / layout_->getScaleFactor();
            auto db_offset_y = offset_y / layout_->getScaleFactor();

            layout_->moveAnchor(db_offset_x, db_offset_y);

            refreshAllComponents();
            emit sendSelectedBox(layout_->getCurSelectedBox());
        }
    }
}

void GraphicsView::mousePressEvent(QMouseEvent* event) {
    if (mouse_status_ == MouseStatus::kInitial) return;
    if (event->button() == Qt::RightButton) {
        point_start_  = event->pos();
        mouse_status_ = MouseStatus::kAreaSelected;
    }

    if (event->button() == Qt::LeftButton) {
        auto point = event->pos();

        auto area = __selecteAreaByPoint(point,
                                         10,
                                         10);

        layout_->setHighlightSelection(area);

        mouse_status_ = MouseStatus::kNormal;
    }

    if (event->button() == Qt::MidButton) {
        setCursor(Qt::OpenHandCursor);
        mouse_status_ = MouseStatus::kMovingSelection;
        point_start_  = event->pos();
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event) {
    if (mouse_status_ == MouseStatus::kInitial) return;
    if (event->button() == Qt::RightButton) {

        mouse_status_ = MouseStatus::kNormal;

        //reset display area
        scene_->setSelectionRect(mapToScene(event->pos()), mapToScene(event->pos()));

        auto db_box = __adjustPointsToDbBox(point_start_, event->pos());

        auto area = layout_->translateTofitScreen(db_box, __getWidth(), __getHeight());

        layout_->setSelectedArea(area);

        refreshAllComponents();
        emit sendSelectedBox(layout_->getCurSelectedBox());
    }
    if (event->button() == Qt::MidButton) {
        mouse_status_ = MouseStatus::kNormal;
        setCursor(Qt::ArrowCursor);
    }
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent* event) {
    if (mouse_status_ == MouseStatus::kInitial) return;
    auto point = event->pos();

    auto area = __selecteAreaByPoint(point,
                                     5,
                                     5);

    auto obj = layout_->getSelectedObj(area);

    if (obj) {
        if (obj->getObjectType() == open_edi::db::ObjectType::kObjectTypeInst) {
            InstPropertyDlg dlg(static_cast<open_edi::db::Inst*>(obj));
            dlg.exec();
        } else if (obj->getObjectType() == open_edi::db::ObjectType::kObjectTypeWire) {
            WirePropertyDlg dlg(static_cast<open_edi::db::Wire*>(obj));
            dlg.exec();
        }
    }
}

void GraphicsView::resizeEvent(QResizeEvent* event) {
    if (mouse_status_ != MouseStatus::kInitial) {

        auto new_view_size = event->size();

        //keep scale
        auto s = layout_->getScaleFactor();

        layout_->resizeLayoutWindow(new_view_size.width(),
                                    new_view_size.height(),
                                    s);
        refreshAllComponents();
        emit sendSelectedBox(layout_->getCurSelectedBox());
    }
    QGraphicsView::resizeEvent(event);
}

GraphicsView* GraphicsView::inst_ = nullptr;
} // namespace gui
} // namespace open_edi