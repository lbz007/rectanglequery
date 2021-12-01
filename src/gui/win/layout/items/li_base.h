#ifndef LI_BASE_H
#define LI_BASE_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QRunnable>
#include <qmath.h>
#include <utility>

#include "../layout_def.h"
#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"
#include "li_manager.h"
#include "util/util.h"

namespace open_edi {
namespace gui {

#define NO_DRAW   0
#define IMG_MODE  1
#define PATH_MODE 2
#define DRAW_MODE IMG_MODE

class LGI_Base;

class LI_Base : public QRunnable {
  public:
    // LI_Base(const LI_Base& other) = delete;
    // LI_Base& operator=(const LI_Base& rhs) = delete;

    LI_Base() { setAutoDelete(false); };
    explicit LI_Base(QString name) {
        name_ = name;
        setAutoDelete(false);
    };
    explicit LI_Base(ScaleFactor* scale_factor) {
        scale_factor_ = scale_factor;
        setAutoDelete(false);
    };
    ~LI_Base(){};

    virtual void      preDraw() = 0;
    virtual void      refreshBoundSize();
    virtual void      setVisible(bool visible);
    virtual LGI_Base* getGraphicItem() = 0;
    virtual void      draw(QPainter* painter);
    virtual bool      hasSubLI() { return false; };
    virtual void      update();
    virtual bool      isMainLI() { return false; };
    virtual bool      isHighlightLI() { return false; };
    virtual QString   getName();
    virtual void      setName(QString name) { name_ = name; };
    virtual bool      isVisible() { return visible_; };
    virtual QBrush    getBrush() { return brush_; };
    virtual void      setBrush(QBrush brush) { brush_ = brush; };
    virtual void      setBrushStyle(Qt::BrushStyle brush_style) { brush_.setStyle(brush_style); };
    template <typename T>
    void             setPen(T&& pen) { pen_ = pen; };
    virtual QPen     getPen() { return pen_; };
    virtual QColor   getColor() { return pen_.color(); };
    virtual void     run() override;
    virtual void     setOffsetX(int offset_x) { offset_x_ = offset_x; };
    virtual void     setOffsetY(int offset_y) { offset_y_ = offset_y; };
    virtual void     setOffset(int offset_x, int offset_y) { offset_x_ = offset_x, offset_y_ = offset_y; };
    virtual int      getOffsetX() { return offset_x_; };
    virtual int      getOffsetY() { return offset_y_; };
    void             setSelectedArea(int llx, int lly, int urx, int ury);
    void             setSelectedArea(open_edi::db::Box& area) { selected_area_.setBox(area); };
    virtual QPixmap* refreshItemMap();
    int              getSceneHight() { return scene_h_; };
    void             setDieAreaSize(int w, int h) { die_area_w_ = w, die_area_h_ = h; };
    void             fetchDB(open_edi::db::Box area, std::vector<open_edi::db::Object*>* result);
    virtual void     setZ(int z) { z_ = z; };
    //get item Z value
    virtual int getZ() { return z_; };
    auto        type() { return type_; };

    enum class ObjType {
        kDieArea,
        kInstance,
        kPin,
        kNet,
        kWire,
        kLayer,
        kHighlight
    };

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

  protected:
    int               scene_w_{1};
    int               scene_h_{1};
    bool              visible_{false};
    ScaleFactor*      scale_factor_{nullptr};
    QPainterPath      painter_path_;
    QString           name_{""};
    bool              main_li{false};
    LI_Manager*       li_mgr_{LI_MANAGER};
    QBrush            brush_;
    QPen              pen_;
    int               offset_x_{0};
    int               offset_y_{0};
    open_edi::db::Box selected_area_;
    int               die_area_w_{0};
    int               die_area_h_{0};
    int               z_;
    ObjType           type_;

    template <typename T>
    LGIBox __translateDbBoxToQtBox(T&& db_box) {

        auto              s = *scale_factor_;
        open_edi::db::Box li_box;
        li_box.setLLX(qFloor((db_box.getLLX() + offset_x_) * s));
        li_box.setLLY(qFloor((db_box.getLLY() + offset_y_) * s));
        li_box.setURX(qFloor((db_box.getURX() + offset_x_) * s));
        li_box.setURY(qFloor((db_box.getURY() + offset_y_) * s));

        LGIBox box;
        auto   sene_h = scene_h_;
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

    void __drawBox(QPainter* painter, LGIBox& box);
    void __drawBox(QPainter* painter, int lux, int luy, int w, int h);
    void __fillBox(QPainter* painter, QBrush& brush, LGIBox& box);
    void __fillBox(QPainter* painter, QBrush& brush, int lux, int luy, int w, int h);
};

} // namespace gui
} // namespace open_edi

#endif