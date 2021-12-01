#ifndef EDI_GUI_COMPONENTS_WIDGET_H_
#define EDI_GUI_COMPONENTS_WIDGET_H_

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QList>
#include <QMap>
#include <QPaintEvent>
#include <QPainter>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

#include "components_listener.h"
#include "tree_widget_base.h"

namespace open_edi {
namespace gui {

class ComponentsWidget : public TreeWidgetBase {
    Q_OBJECT
  public:
    explicit ComponentsWidget(QWidget* parent = nullptr);
    ~ComponentsWidget();
    void addComponentListener(ComponentListener* listener);
    void refreshComponentAttributes();

  private:
    QList<ComponentListener*> component_listener_list_;
    void                      __setAllSubComponentVisibleAttribute(ComponentListener* listener, QTreeWidgetItem* item);
    bool                      initiated_{false};
  private slots:
    void slotItemClicked(QTreeWidgetItem* item, int column);
    void slotItemStyleChange(const char* item_name, QBrush brush);
};

} // namespace gui
} // namespace open_edi

#endif // LAYER_WIDGET_H
