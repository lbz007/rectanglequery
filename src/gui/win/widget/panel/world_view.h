#ifndef EDI_GUI_WORLD_VIEW_H_
#define EDI_GUI_WORLD_VIEW_H_

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>
#include <QResizeEvent>

#include "../../layout/layout_def.h"

#include <qmath.h>
#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"

namespace open_edi {
namespace gui {

class WorldSence : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit WorldSence(QObject* parent = nullptr);
    ~WorldSence(){};
    void setSelectionRect(QPointF p1, QPointF p2);
    void setSelectionRect(QRectF rect);
    void setFocusPoint(QPointF p);
    void refreshDieAreaRect();

  protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

  private:
    QGraphicsRectItem* selection_rect_item_{nullptr};
    QGraphicsRectItem* die_area_rect_item_{nullptr};
    QGraphicsLineItem* focus_item_w_;
    QGraphicsLineItem* focus_item_h_;
};

class WorldView : public QGraphicsView {
    Q_OBJECT
  public:
    WorldView();
    void initDieArea(int w, int h);

  signals:
    void sendSelectedBox(open_edi::db::Box area);

  public slots:
    void slotSyncPosition(open_edi::db::Box area);

  protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

  private:
    ScaleFactor       __caculateScaleFactor(int w, int h);
    void              __translateToSceneSize(open_edi::db::Box area);
    open_edi::db::Box __translateFromPoint(QPoint point);

    WorldSence*       scene_{nullptr};
    int               die_area_w_{0};
    int               die_area_h_{0};
    ScaleFactor       s_;
    open_edi::db::Box selected_box_;
    QRectF            drawed_box_;

    class LGIBox {
      public:
        auto getLLX() { return llx; };
        auto getLLY() { return lly; };
        auto getURX() { return urx; };
        auto getURY() { return ury; };
        auto getWidth() { return w; };
        auto getHeight() { return h; };
        auto getLUX() { return lux; };
        auto getLUY() { return luy; };
        auto getRLX() { return rlx; };
        auto getRLY() { return rly; };

        int llx{0};
        int lly{0};
        int urx{0};
        int ury{0};
        int w{0};
        int h{0};

        int lux{0};
        int luy{0};
        int rlx{0};
        int rly{0};

        ~LGIBox(){};
    };

    template <typename T>
    LGIBox __translateDbBoxToQtBox(T&& db_box) {

        auto              s = s_;
        open_edi::db::Box li_box;
        li_box.setLLX(qFloor((db_box.getLLX()) * s));
        li_box.setLLY(qFloor((db_box.getLLY()) * s));
        li_box.setURX(qFloor((db_box.getURX()) * s));
        li_box.setURY(qFloor((db_box.getURY()) * s));

        LGIBox box;
        auto   sene_h = scene_->height();
        box.llx       = li_box.getLLX();
        box.lly       = sene_h - li_box.getLLY();
        box.urx       = li_box.getURX();
        box.ury       = sene_h - li_box.getURY();
        box.w         = li_box.getWidth() ? li_box.getWidth() - 1 : 0;
        box.h         = li_box.getHeight() ? li_box.getHeight() - 1 : 0;

        box.lux = box.llx;
        box.luy = box.ury;
        box.rlx = box.urx;
        box.rly = box.lly;

        return box;
    }
};
} // namespace gui
} // namespace open_edi

#endif