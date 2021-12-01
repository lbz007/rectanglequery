#include "layout.h"
#include "./items/li_die_area.h"
#include "layout_def.h"

namespace open_edi {
namespace gui {
Layout::~Layout() {
}

void Layout::initAllLayers() {
    auto tc         = open_edi::db::getTopCell();
    auto tech_lib   = tc->getTechLib();
    auto num_layers = tech_lib->getNumLayers();
    // printf("number of laryers: %d\n", num_layers);
    for (int i = 0; i < num_layers; i++) {
        auto layer = tech_lib->getLayer(i);
        li_manager_->addLayer(layer, &scale_factor_);
    }
}

void Layout::refreshAllComponents() {
    QTime time;
    time.start();

    li_manager_->preDrawAllItems();

    auto time_elapsed = time.elapsed();
    printf("total elapsed time %d (ms)\n", time_elapsed);
}

open_edi::db::Box Layout::getCurSelectedBox() {
    return li_manager_->getSelectedAreaBox();
}

void Layout::createLayoutItems() {
    li_manager_->createLI(new LI_DieArea(&scale_factor_));

    LI_Instance* li_inst = new LI_Instance(&scale_factor_);
    li_manager_->createLI(li_inst);
    for (auto& reader : *(li_manager_->getDataReaders())) {
        reader.setLiInst(li_inst);
    }

    LI_Pin* li_pin = new LI_Pin(&scale_factor_);
    li_manager_->createLI(li_pin);
    for (auto& reader : *(li_manager_->getDataReaders())) {
        reader.setLiPin(li_pin);
    }

    auto li_net = new LI_Net(&scale_factor_);
    li_manager_->createLI(li_net);
    for (auto& reader : *(li_manager_->getDataReaders())) {
        reader.setLiNet(li_net);
    }
    li_manager_->createLI(new LI_HighLight(&scale_factor_));

    auto li_wire = new LI_Wire((&scale_factor_));
    li_manager_->createLI(li_wire);
    for (auto& reader : *(li_manager_->getDataReaders())) {
        reader.setLiWire(li_wire);
    }
}
/**
 * Layout 
 * move anchor(llx lly). It does not impact selected width and height
 * @param  {int} db_offset_x : offset x
 * @param  {int} db_offset_y : offset y
 */
void Layout::moveAnchor(int db_offset_x, int db_offset_y) {

    open_edi::db::Box db_box = getCurSelectedBox();

    open_edi::db::Box area;
    area.setLLX(qFloor((db_box.getLLX() + db_offset_x)));
    area.setLLY(qFloor((db_box.getLLY() + (-db_offset_y))));
    area.setURX(qFloor((db_box.getURX() + db_offset_x)));
    area.setURY(qFloor((db_box.getURY() + (-db_offset_y))));

    setSelectedArea(area);
}

void Layout::setAnchor(int center_x, int center_y) {
    open_edi::db::Box db_box = getCurSelectedBox();
    auto              llx    = center_x /* - db_box.getWidth() / 2 */;
    auto              lly    = center_y /* - db_box.getHeight() / 2 */;
    open_edi::db::Box area;
    area.setLLX(llx);
    area.setLLY(lly);
    area.setURX(llx + db_box.getWidth());
    area.setURY(lly + db_box.getHeight());

    // qDebug() << area.getLLX() << area.getLLY();

    setSelectedArea(area);
}

/**
 * Layout 
 * calculate scale factor according screen size
 * @param  {int} screen_w  : screen width
 * @param  {int} screen_h  : screen height
 * @param  {int} db_area_w : selected db area width
 * @param  {int} db_area_h : selected db area height
 * @return {ScaleFactor}           : scale factor
 */
ScaleFactor Layout::calculateScaleFactor(int screen_w, int screen_h, int db_area_w, int db_area_h) {

    //calculate scale factor according current sceen size
    auto factor_x = (screen_w) / ((decltype(scale_factor_))db_area_w);
    auto factor_y = (screen_h) / ((decltype(scale_factor_))db_area_h);

    //keep same ratio for both x and y
    scale_factor_ = factor_x > factor_y ?
                      factor_y :
                      factor_x;

    //display size , the max is screen size
    screen_w_size_ = qFloor((db_area_w)*scale_factor_);
    screen_h_size_ = qFloor((db_area_h)*scale_factor_);

    return scale_factor_;
}

std::tuple<int, int> Layout::initDieArea() {
    auto tc = open_edi::db::getTopCell();

    auto  poly = tc->getFloorplan()->getDieAreaPolygon();
    float factor_x, factor_y;

    auto numPoints = poly->getNumPoints();

    die_area_w_ = die_area_h_ = 0;

    //find die area
    for (int i = 0; i < numPoints; i++) {
        auto point   = poly->getPoint(i);
        auto point_x = point.getX();
        auto point_y = point.getY();

        if (point_x && point_y) {
            die_area_w_ = point_x;
            die_area_h_ = point_y;
            for (auto li : li_manager_->getLiList()) {
                li->setDieAreaSize(die_area_w_, die_area_h_);
            }
        }
    }

    return std::tuple<int, int>(die_area_w_, die_area_h_);
}
/**
 * Layout 
 * 
 * @param  {QPointF} p1 : 
 * @param  {QPointF} p2 : 
 */
void Layout::setSelectedArea(QPointF p1, QPointF p2) {
    li_manager_->setSelectedArea(p1, p2);
}
/**
 * Layout 
 * select an area in dbu 
 * @param  {int} x : llx
 * @param  {int} y : lly
 * @param  {int} w : width
 * @param  {int} h : height
 */
void Layout::setSelectedArea(int x, int y, int w, int h) {
    li_manager_->setSelectedArea(x, y, w, h);
    auto p_w = w * getScaleFactor();
    setScreenW(p_w);
    auto p_h = h * getScaleFactor();
    setScreenH(p_h);
}
/**
 * Layout 
 * select an area in dbu by box
 * @param  {open_edi::db::Box} area : a db box area
 */
void Layout::setSelectedArea(open_edi::db::Box& area) {
    li_manager_->setSelectedArea(area);
}

void Layout::setHighlightSelection(open_edi::db::Box& area) {
    li_manager_->setHighlightSelectionArea(area);
}

open_edi::db::Box Layout::translateTofitScreen(open_edi::db::Box& box,
                                               int                screen_w,
                                               int                screen_h) {
    //calculate scale factor according current sceen size
    auto factor_w = screen_w / (decltype(scale_factor_))box.getWidth();
    auto factor_h = screen_h / (decltype(scale_factor_))box.getHeight();

    //get max factor as scale factor for keeping same ratio for both x and y
    auto factor = factor_w > factor_h ?
                    factor_h :
                    factor_w;

    scale_factor_ = factor;

    //get new screen
    auto ns_w = box.getWidth() * factor;
    auto ns_h = box.getHeight() * factor;

    //get adjsust w and h
    auto box_w = screen_w / factor;
    auto box_h = screen_h / factor;

    //display size , the max is screen size
    screen_w_size_ = screen_w;
    screen_h_size_ = screen_h;

    int llx = box.getLLX();
    int lly = box.getLLY();

    if (screen_w - ns_w > 10) {
        auto adj = (screen_w - ns_w) / scale_factor_;
        llx      = llx - adj / 2;
    }

    if (screen_h - ns_h > 10) {
        auto adj = (screen_h - ns_h) / scale_factor_;
        lly      = lly - adj / 2;
    }

    llx += li_manager_->getAnchorLLX();
    lly += li_manager_->getAnchorLLY();

    open_edi::db::Box new_box(llx, lly, llx + box_w, lly + box_h);

    return new_box;
}

open_edi::db::Object* Layout::getSelectedObj(open_edi::db::Box& area) {
    std::vector<open_edi::db::Object*> result;
    open_edi::db::fetchDB(area, &result);
    int                   top_layer = 0;
    open_edi::db::Object* ret       = nullptr;
    for (auto obj : result) {
        if (obj->getObjectType() == open_edi::db::ObjectType::kObjectTypeWire) {
            auto li = li_manager_->getLiByName(static_cast<open_edi::db::Wire*>(obj)->getLayer()->getName());
            if (li->isVisible() && li->getZ() >= top_layer) {
                top_layer = li->getZ();
                ret       = obj;
            }
        } else if (obj->getObjectType() == open_edi::db::ObjectType::kObjectTypeInst) {
            auto li = li_manager_->getLiByName(kLiInstanceName);
            if (li->isVisible() && li->getZ() >= top_layer) {
                top_layer = li->getZ();
                ret       = obj;
            }
        }
    }
    return ret;
}

void Layout::selectWholeArea() {
    li_manager_->setSelectedArea(QPointF(0, 0), QPointF(die_area_w_, die_area_h_));
}

void Layout::viewFitArea() {
    selectWholeArea();
    refreshAllComponents();
}

void Layout::setHighlightMaskLevel(int v) {
    auto li_highlight = dynamic_cast<LI_HighLight*>(li_manager_->getHighlightLI());
    li_highlight->setMaskValue(v);
    li_highlight->preDraw();
    li_highlight->update();
}
/**
 * Layout 
 * resize layout window 
 * @param  {int} w         : screen width
 * @param  {int} h         : screen height
 * @param  {ScaleFactor} s : 
 */
void Layout::resizeLayoutWindow(int w, int h, ScaleFactor s) {

    setScaleFactor(s);
    // new width in dbu
    auto db_w = w / s;
    // db_w      = db_w > getDieAreaW() ? getDieAreaW() : db_w;
    // new height in dbu
    auto db_h = h / s;
    // db_h      = db_h > getDieAreaH() ? getDieAreaH() : db_h;

    auto anchor_point = getAnchorPoint();
    auto llx          = std::get<0>(anchor_point);
    auto lly          = std::get<1>(anchor_point);

    // calculate new offset x
    auto offset_x = (int)(getSelectedAreaW() - db_w) / 2 + llx;

    // calculate new offset y
    auto offset_y = (int)(getSelectedAreaH() - db_h) / 2 + lly;

    setSelectedArea(offset_x, offset_y, db_w, db_h);
}

void Layout::setLayerVisible(QString name, bool v) {
    auto li_layer = li_manager_->getLayerByName(name);
    if (li_layer) {
        li_layer->setVisible(v);
        // refreshAllComponents();
    }
}

void Layout::setLayerColor(QString name, QColor color) {

    auto li_layer = li_manager_->getLayerByName(name);
    if (li_layer) {
        li_layer->setPen(QPen(color));
        // refreshAllComponents();
    }
}

void Layout::setLayerSelectable(QString name, bool v) {
}

void Layout::layerUpdate() {
    refreshAllComponents();
}

void Layout::setComponentVisible(QString name, bool v) {

    if (li_manager_->getLiByName(name)) {

        li_manager_->setLiVisibleByName(name, v);
        // refreshAllComponents();
    }
}

void Layout::setComponentSelectable(QString name, bool v) {
}

void Layout::setComponentBrushStyle(QString name, Qt::BrushStyle brush_style) {

    if (li_manager_->getLiByName(name)) {

        li_manager_->setLiBrushStyleByName(name, brush_style);
        refreshAllComponents();
    }
}

void Layout::componentUpdate() {
    refreshAllComponents();
}

Layout* Layout::inst_ = nullptr;
} // namespace gui
} // namespace open_edi