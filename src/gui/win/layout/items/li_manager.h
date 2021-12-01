#ifndef EDI_GUI_LI_MANAGER_H_
#define EDI_GUI_LI_MANAGER_H_

#include <QList>
#include <QMap>
#include <QPoint>
#include <QRunnable>
#include <QString>
#include <QTimer>
#include "../layout_def.h"
#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/core/db_init.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"

namespace open_edi {
namespace gui {

#define LI_MANAGER (LI_Manager::getInstance())

class LI_Base;
class LI_Layer;
class LI_Manager;
class LI_Instance;
class LI_Pin;
class LI_Wire;
class LI_Net;

class DataReader : public QRunnable {
  public:
    DataReader() { setAutoDelete(false); };
    ~DataReader(){};
    virtual void run() override;

    void setBox(open_edi::db::Box& box) { box_ = box; };
    void setIndex(int index) { index_ = index; };
    void setLiManager(LI_Manager* li_manger) { li_manger_ = li_manger; };
    void setLiInst(LI_Instance* li_inst) { li_inst_ = li_inst; };
    void setLiPin(LI_Pin* li_pin) { li_pin_ = li_pin; };
    void setLiWire(LI_Wire* li_wire) { li_wire_ = li_wire; };
    void setLiNet(LI_Net* li_net) { li_net_ = li_net; };

  private:
    int               index_{0};
    open_edi::db::Box box_;
    LI_Wire*          li_wire_{nullptr};
    LI_Manager*       li_manger_{nullptr};
    LI_Instance*      li_inst_{nullptr};
    LI_Pin*           li_pin_{nullptr};
    LI_Net*           li_net_{nullptr};
};

class LI_Manager /* : public QObject */ {
    // Q_OBJECT
  public:
    ~LI_Manager(){};
    static LI_Manager* getInstance() {
        if (!inst_) {
            inst_ = new LI_Manager;
        }
        return inst_;
    }

    void            preDrawAllItems();
    QList<LI_Base*> getLiList();

    template <typename T>
    auto createLI(T&& li) {
        li_map_[li->getName()] = li;
    };
    void              setLiVisibleByName(QString name, bool v);
    void              setLiBrushStyleByName(QString name, Qt::BrushStyle brush_style);
    LI_Base*          getLiByName(QString name);
    void              addLayer(open_edi::db::Layer* layer, ScaleFactor* scale_factor);
    void              setLayerVisibleByName(QString name, bool v);
    void              setLayerColorByName(QString name, QColor color);
    LI_Base*          getLayerByName(QString name);
    QList<LI_Layer*>  getLayerList();
    void              setSelectedArea(QPointF p1, QPointF p2);
    void              setSelectedArea(int x, int y, int w, int h);
    void              setSelectedArea(open_edi::db::Box area);
    open_edi::db::Box getSelectedAreaBox() { return selected_box_; };
    void              setHighlightSelectionArea(open_edi::db::Box area);
    int               getAnchorLLX() { return selected_box_.getLLX(); };
    int               getAnchorLLY() { return selected_box_.getLLY(); };
    void              resetSelectedBox();
    LI_Base*          getHighlightLI();
    void              addRoutingLayer(LI_Layer* layer) { routing_layer_list_.append(layer); };
    QList<LI_Layer*>& getRoutingLayersList() { return routing_layer_list_; };
    auto              getDataReaders() { return &readers_; };

  private:
    LI_Manager();
    static LI_Manager*      inst_;
    QMap<QString, LI_Base*> li_map_;
    int*                    scale_factor_;
    int                     llx_{0};
    int                     lly_{0};
    open_edi::db::Box       selected_box_;
    QList<LI_Layer*>        routing_layer_list_;
    DataReader              readers_[kSplitNum];
    QTimer*                 timer_{nullptr};

    void __split_area();

    // private slots:
    //   void slot_handle_timer();
};
} // namespace gui
} // namespace open_edi

#endif