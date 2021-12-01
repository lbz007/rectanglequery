#include "tree_widget_base.h"

namespace open_edi {
namespace gui {

TreeWidgetBase::Label::Label(QWidget* parent) : QLabel(parent) {
}

void TreeWidgetBase::Label::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setBrush(brush_);
    painter.fillRect(e->rect(), brush_);
}

TreeWidgetBase::TreeWidgetBase(QWidget* parent) : QTreeWidget(parent) {

    setColumnCount(kSelectable + 1);

    auto treewidgetitem = headerItem();
    treewidgetitem->setText(kName, "Name");
    treewidgetitem->setText(kColor, "");
    treewidgetitem->setText(kVisible, "V");
    treewidgetitem->setText(kSelectable, "S");

    // setColumnWidth(kName, 50);
    header()->setSectionResizeMode(kName, QHeaderView::Stretch);
    setColumnWidth(kColor, 20);
    header()->setSectionResizeMode(kColor, QHeaderView::Fixed);
    setColumnWidth(kVisible, 20);
    header()->setSectionResizeMode(kVisible, QHeaderView::Fixed);
    setColumnWidth(kSelectable, 20);
    header()->setSectionResizeMode(kSelectable, QHeaderView::Fixed);
    header()->setStretchLastSection(false);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setStyleSheet("QTreeView::item:selected{background-color: rgba(0,0,255,10%)}");
}

TreeWidgetBase::~TreeWidgetBase() {
}

// void TreeWidgetBase::slotItemClicked(QTreeWidgetItem* item, int column) {

//     switch (column) {
//     case kName:
//         break;
//     case kColor: {
//         auto name = item->text(kName).toLocal8Bit().constData();
//         pal->getItemName(name);
//         pal->show();
//     }
//     // pal->getItemStyle(item, column);
//     break;
//     case kVisible:
//         break;
//     case kSelectable:
//         break;
//     default:
//         break;
//     }
// }

void TreeWidgetBase::__createSubItems(QTreeWidgetItem*               parent_item,
                                      QList<struct ItemAttributes>&& item_attr) {
    for (auto attr : item_attr) {
        auto sub_item = new QTreeWidgetItem(parent_item);
        parent_item->addChild(sub_item);
        sub_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate | Qt::ItemIsUserCheckable);
        sub_item->setText(kName, attr.name);
        sub_item->setCheckState(kVisible, Qt::Unchecked);
        sub_item->setCheckState(kSelectable, Qt::Unchecked);
        sub_item->setExpanded(true);
        auto label            = new Label(this);
        lable_map_[attr.name] = label;
        lable_map_[attr.name]->setBrush(attr.brush);
        setItemWidget(sub_item, kColor, label);
    }
}

QTreeWidgetItem* TreeWidgetBase::__createSubItem(QTreeWidgetItem* parent_item, const char* name) {
    auto sub_item = new QTreeWidgetItem(parent_item);
    parent_item->addChild(sub_item);
    sub_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate | Qt::ItemIsUserCheckable);
    sub_item->setText(kName, name);
    sub_item->setCheckState(kVisible, Qt::Unchecked);
    sub_item->setCheckState(kSelectable, Qt::Unchecked);
    sub_item->setExpanded(true);
    return sub_item;
}

QTreeWidgetItem* TreeWidgetBase::__createTopItem(QTreeWidget* parent_item, const char* name) {
    auto item = new QTreeWidgetItem(parent_item);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate | Qt::ItemIsUserCheckable);
    item->setText(kName, tr(name));
    item->setCheckState(kVisible, Qt::Unchecked);
    item->setCheckState(kSelectable, Qt::Unchecked);
    item->setExpanded(true);
    return item;
}

} // namespace gui
} // namespace open_edi