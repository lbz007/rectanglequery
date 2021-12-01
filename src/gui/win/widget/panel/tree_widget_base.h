#ifndef EDI_GUI_TREE_WIDGET_BASE_H_
#define EDI_GUI_TREE_WIDGET_BASE_H_

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

#include "palette.h"

namespace open_edi {
namespace gui {

class TreeWidgetBase : public QTreeWidget {
    Q_OBJECT
  public:
    explicit TreeWidgetBase(QWidget* parent = nullptr);
    ~TreeWidgetBase();

  // public slots:

  //   virtual void slotItemClicked(QTreeWidgetItem* item, int column) = 0;

  protected:
    enum {
        kName,
        kColor,
        kVisible,
        kSelectable
    };

    struct ItemAttributes {
        const char* name;
        QBrush      brush;
    };

    class Label : public QLabel {
      public:
        Label(QWidget* parent = nullptr);
        void   setBrush(QBrush brush) { brush_ = brush; };
        QBrush getBrush() { return brush_; };

      protected:
        virtual void paintEvent(QPaintEvent*) override;

      private:
        QBrush brush_;
    };

    QTreeWidgetItem*          widgetitem_;
    Palette*                  pal{nullptr};
    std::map<QString, Label*> lable_map_;

    virtual void             __createSubItems(QTreeWidgetItem*               parent_item,
                                              QList<struct ItemAttributes>&& item_attr);
    virtual QTreeWidgetItem* __createSubItem(QTreeWidgetItem* parent_item, const char* name);
    virtual QTreeWidgetItem* __createTopItem(QTreeWidget* parent_item, const char* name);
};

} // namespace gui
} // namespace open_edi

#endif // LAYER_WIDGET_H
