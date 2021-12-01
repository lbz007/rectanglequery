#ifndef LAYER_WIDGET_H
#define LAYER_WIDGET_H

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QList>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableView>
#include <QTreeWidget>
#include <QWidget>
#include <map>

#include "db/core/cell.h"
#include "db/core/db.h"
#include "db/io/write_def.h"
#include "db/util/array.h"
#include "db/util/property_definition.h"
#include "layer_listener.h"
#include "tree_widget_base.h"

namespace open_edi {
namespace gui {

class LI_Layer;

class LayerWidget : public TreeWidgetBase {
    Q_OBJECT
  public:
    explicit LayerWidget(QWidget* parent = nullptr);
    ~LayerWidget();

    void addLayerListener(LayerListener* listener);
    void refreshTree(QList<LI_Layer*> layer_list);

  private:
    QList<LayerListener*> layer_listener_list_;
    QTreeWidgetItem*      layer_top_item_;
    QTreeWidgetItem*      layer_masterslice_item_;
    QTreeWidgetItem*      layer_routing_item_;
    QTreeWidgetItem*      layer_backside_item_;

    void __setAllSubLayerVisibleAttribute(LayerListener* listener, QTreeWidgetItem * item);
  private slots:
    void slotItemClicked(QTreeWidgetItem* item, int column);
    void slotItemColorChange(const char* item_name, QBrush brush);
};

} // namespace gui
} // namespace open_edi

#endif // LAYER_WIDGET_H
