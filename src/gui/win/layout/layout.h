#ifndef OPEN_EDI_LAYOUT_LAYOUT_H_
#define OPEN_EDI_LAYOUT_LAYOUT_H_

#include <QColor>
#include <QList>
#include <QString>
#include <QTime>
#include <qmath.h>

#include "../widget/panel/components_listener.h"
#include "../widget/panel/layer_listener.h"
#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"
#include "graphics_scene.h"
#include "items/li_die_area.h"
#include "items/li_highlight.h"
#include "items/li_instance.h"
#include "items/li_manager.h"
#include "items/li_net.h"
#include "util/util.h"

namespace open_edi {
namespace gui {

#define LAYOUT_INSTANCE (Layout::getInstance())

class Layout : public LayerListener,
               public ComponentListener {
  public:
    ~Layout();

    static Layout* getInstance() {
        if (!inst_) {
            inst_ = new Layout;
        }
        return inst_;
    }

    void setViewSize(int w, int h);
    void initAllLayers();
    void refreshAllComponents();
    //get die area w in dbu
    auto getDieAreaW() { return die_area_w_; };
    //get die area h in dbu
    auto              getDieAreaH() { return die_area_h_; };
    auto              getScaleFactor() { return scale_factor_; };
    void              setScaleFactor(ScaleFactor sf) { scale_factor_ = sf; };
    void              setScreenW(int w) { screen_w_size_ = w; };
    auto              getScreenW() { return screen_w_size_; };
    void              setScreenH(int h) { screen_h_size_ = h; };
    auto              getScreenH() { return screen_h_size_; };
    auto              getAnchorPoint() { return std::tuple<int, int>(li_manager_->getAnchorLLX(), li_manager_->getAnchorLLY()); };
    open_edi::db::Box getCurSelectedBox();
    void              createLayoutItems();
    void              moveAnchor(int db_offset_x, int db_offset_y);
    void              setAnchor(int llx, int lly);
    auto              getLayerList() { return li_manager_->getLayerList(); };
    auto              getLiManager() { return li_manager_; };

    //calculate scale factor according screen size
    ScaleFactor calculateScaleFactor(int screen_w, int screen_h, int db_area_w, int db_area_h);

    std::tuple<int, int> initDieArea();
    void                 setSelectedArea(QPointF p1, QPointF p2);
    void                 setSelectedArea(int x, int y, int w, int h);
    void                 setSelectedArea(open_edi::db::Box& area);
    void                 setHighlightSelection(open_edi::db::Box& area);
    open_edi::db::Box    translateTofitScreen(open_edi::db::Box& box,
                                              int                screen_w,
                                              int                screen_h);

    open_edi::db::Object* getSelectedObj(open_edi::db::Box& area);
    // get width of selected area in db uint for caculating scale factor
    auto getSelectedAreaW() { return getCurSelectedBox().getWidth(); };
    // get height of selected area in db uint for caculating scale factor
    auto getSelectedAreaH() { return getCurSelectedBox().getHeight(); };
    // select whole db area
    void selectWholeArea();
    // view fit area
    void viewFitArea();
    //set highlight mask level
    void setHighlightMaskLevel(int v);
    //resize layout window
    void resizeLayoutWindow(int w, int h, ScaleFactor s);

    virtual void setLayerVisible(QString name, bool v) override;
    virtual void setLayerColor(QString name, QColor color) override;
    virtual void setLayerSelectable(QString name, bool v) override;
    virtual void layerUpdate() override;
    virtual void setComponentVisible(QString name, bool v) override;
    virtual void setComponentSelectable(QString name, bool v) override;
    virtual void setComponentBrushStyle(QString name, Qt::BrushStyle brush_style) override;
    virtual void componentUpdate() override;

  private:
    //real chip area width in db uint
    int die_area_w_{0};
    //real chip area height in db uint
    int die_area_h_{0};
    //physical screen view width size
    int screen_w_size_{0};
    //physical screen view height size
    int screen_h_size_{0};
    //scale factor from db to screen
    ScaleFactor scale_factor_{1};
    //singleton instance
    static Layout* inst_;
    //layout item manager
    LI_Manager* li_manager_{LI_MANAGER};
};
} // namespace gui
} // namespace open_edi

#endif