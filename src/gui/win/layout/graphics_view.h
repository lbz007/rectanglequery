#ifndef OPEN_EDI_LAYOUT_GRAPHICS_VIEW_H_
#define OPEN_EDI_LAYOUT_GRAPHICS_VIEW_H_

#define USE_OPENGL 0

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainterPath>
#include <QPoint>
#include <QResizeEvent>
#include <QScreen>
#include <QWheelEvent>
#if USE_OPENGL == 1
#include <QtOpenGL>
#endif
#include <qmath.h>
#include "../widget/panel/components_listener.h"
#include "../widget/panel/layer_listener.h"
#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"
#include "graphics_scene.h"
#include "layout.h"

namespace open_edi {
namespace gui {

#define LAYOUT_GRAPHICS_VIEW (GraphicsView::getInstance())

class GraphicsView : public QGraphicsView {
    Q_OBJECT
  public:
    ~GraphicsView();

    static GraphicsView* getInstance() {
        if (!inst_) {
            inst_ = new GraphicsView;
        }
        return inst_;
    }

    //enum
    enum class MouseStatus {
        kInitial,
        kNormal,
        kAreaSelected,
        kMovingSelection,
        kSingleSelected,
        KShowPropertyDlg
    };

    void refreshAllComponents();
    void refreshFitDraw();
    void initView();
    auto getLayout() { return layout_; };
    void refreshManually();
    bool isInInitial() { return mouse_status_ == MouseStatus::kInitial; };
    void setStatusToNormal() { mouse_status_ = MouseStatus::kNormal; };

  signals:
    void sendPos(double x, double y);
    void sendSelectedBox(open_edi::db::Box box);
    void initByCmd();

  public slots:
    void slotZoomIn(bool);
    void slotZoomOut(bool);

    void slotReadLayer();
    void setPinsVisible(bool);
    void slotSetHighlightMaskValue(int);
    void slotUpdatePosition(open_edi::db::Box area);

  protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    // bool eventFilter(QObject* obj, QEvent* e) override;

  private:
    //scene of this view
    GraphicsScene* scene_;
    //for singleton pattern
    static GraphicsView* inst_;
    //for getting screen resolution
    QScreen* screen_;
    //seletion start point
    QPoint point_start_;
    //seletion moving point
    QPoint point_moving_;
    //for singleton pattern
    GraphicsView(QWidget* parent = nullptr);

    Layout* layout_{nullptr};

    MouseStatus mouse_status_{MouseStatus::kInitial};
    //for zoom in / out
    void __zoom(qreal value);
    //calculate origin point offset
    std::tuple<int, int, int, int> __calcOriginPointOffset(int x, int y, int w, int h, qreal factor);
    //adjust selected point to db box
    open_edi::db::Box __adjustPointsToDbBox(QPoint p_start, QPoint p_end);
    //translate to db point
    std::tuple<double, double> __translateToDbPoint(QPoint point);
    //
    int __getWidth() { return width(); };
    int __getHeight() { return height(); };
    //
    open_edi::db::Box __selecteAreaByPoint(QPoint point, int width, int height);
};
} // namespace gui
} // namespace open_edi
#endif
