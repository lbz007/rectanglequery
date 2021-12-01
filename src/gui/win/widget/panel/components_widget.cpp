#include "components_widget.h"

#include "../../layout/layout_def.h"

namespace open_edi {
namespace gui {

ComponentsWidget::ComponentsWidget(QWidget* parent) : TreeWidgetBase(parent) {

    QList<QBrush> style_array{
      QBrush("Black", Qt::SolidPattern),
      QBrush("Black", Qt::Dense2Pattern),
      QBrush("Black", Qt::Dense4Pattern),
      QBrush("Black", Qt::Dense6Pattern),
      QBrush("Black", Qt::HorPattern),
      QBrush("Black", Qt::VerPattern),
      QBrush("Black", Qt::CrossPattern),
      QBrush("Black", Qt::BDiagPattern),
      QBrush("Black", Qt::FDiagPattern),
      QBrush("Black", Qt::DiagCrossPattern),
    };

    pal = new Palette(std::move(style_array));

    pal->setName("Pattern:  ");

    auto top_item = __createTopItem(this, "Cell");

    __createSubItems(top_item,
                     {{ItemAttributes{kLiPinName, style_array[1]}},
                      {ItemAttributes{"OBS", style_array[2]}}});

    top_item->setCheckState(kVisible, Qt::Checked);

    top_item = __createTopItem(this, kLiInstanceName);

    __createSubItems(__createSubItem(top_item, "Physical Cell"),
                     {{ItemAttributes{"Filler", style_array[1]}},
                      {ItemAttributes{"End-cap", style_array[2]}},
                      {ItemAttributes{"Well-tap", style_array[3]}},
                      {ItemAttributes{"De-cap", style_array[4]}},
                      {ItemAttributes{"Padding", style_array[5]}}});
    __createSubItems(top_item,
                     {{ItemAttributes{"IO", style_array[1]}}});

    __createSubItems(top_item,
                     {{ItemAttributes{"Standard cell", style_array[1]}}});

    __createSubItems(top_item,
                     {{ItemAttributes{"Block", style_array[1]}}});

    __createSubItems(top_item,
                     {{ItemAttributes{"Cover cell", style_array[1]}}});

    __createSubItems(__createSubItem(top_item, "Place status"),
                     {{ItemAttributes{"Unplaced", style_array[1]}},
                      {ItemAttributes{"Placed", style_array[2]}},
                      {ItemAttributes{"Fixed", style_array[3]}},
                      {ItemAttributes{"Cover", style_array[4]}}});

    top_item->setCheckState(kVisible, Qt::Checked);

    // __createSubItems(__createSubItem(top_item, "Power"),
    //                  {{ItemAttributes{"Power Switch", style_array[1]}},
    //                   {ItemAttributes{"Level Shifter", style_array[2]}},
    //                   {ItemAttributes{"Isolation", style_array[3]}}});

    top_item = __createTopItem(this, kLiNetName);

    __createSubItems(top_item,
                     {
                       {ItemAttributes{"Signal", style_array[1]}},
                       {ItemAttributes{"Tie Hi / Lo", style_array[2]}},
                       {ItemAttributes{"Clock", style_array[3]}},
                       {ItemAttributes{"Power", style_array[4]}},
                       {ItemAttributes{"Ground", style_array[5]}},
                     });
    top_item->setCheckState(kVisible, Qt::Checked);
    top_item = __createTopItem(this, "Route");

    __createSubItems(top_item,
                     {
                       {ItemAttributes{kLiWireName, style_array[1]}},
                       {ItemAttributes{"Via", style_array[2]}},
                       {ItemAttributes{"Patch wire", style_array[5]}},
                       {ItemAttributes{"Shield ", style_array[6]}},
                     });
    top_item->setCheckState(kVisible, Qt::Checked);
    top_item = __createTopItem(this, "Track");

    __createSubItems(top_item,
                     {{ItemAttributes{"Preferred", style_array[1]}},
                      {ItemAttributes{"Non-Preferred", style_array[2]}}});

    top_item = __createTopItem(this, "Row");

    __createSubItems(top_item,
                     {{ItemAttributes{"Standard-Row", style_array[1]}},
                      {ItemAttributes{"Site", style_array[2]}},
                      {ItemAttributes{"IO-Row", style_array[3]}}});

    top_item = __createTopItem(this, "Blockage");

    __createSubItems(__createSubItem(top_item, "Placement"),
                     {
                       {ItemAttributes{"Soft Blockage", style_array[1]}},
                       {ItemAttributes{"Hard Blockage", style_array[2]}},
                       {ItemAttributes{"Partial Blockage", style_array[3]}},
                     });

    __createSubItems(__createSubItem(top_item, "Routing"),
                     {
                       {ItemAttributes{"Guide", style_array[1]}},
                       {ItemAttributes{"Halo", style_array[2]}},
                       {ItemAttributes{"Blockage", style_array[3]}},
                     });

    top_item = __createTopItem(this, "Map");

    __createSubItems(top_item,
                     {{ItemAttributes{"Cell Density Map", style_array[1]}},
                      {ItemAttributes{"Congestion Map", style_array[2]}},
                      {ItemAttributes{"Pin Density Map", style_array[3]}}});

    top_item = __createTopItem(this, "Grid");
    __createSubItems(top_item,
                     {{ItemAttributes{"Manufacture", style_array[1]}},
                      {ItemAttributes{"Placement", style_array[4]}},
                      {ItemAttributes{"User Defined", style_array[2]}},
                      {ItemAttributes{"Gcell", style_array[3]}}});

    connect(this, &ComponentsWidget::itemClicked, this, &ComponentsWidget::slotItemClicked);
    connect(pal, &Palette::signalBtnOKClicked, this, &ComponentsWidget::slotItemStyleChange);
}

ComponentsWidget::~ComponentsWidget() {
}

void ComponentsWidget::addComponentListener(ComponentListener* listener) {
    component_listener_list_.append(listener);
}

void ComponentsWidget::refreshComponentAttributes() {

    initiated_ = true;
    // auto count = topLevelItemCount();

    // for (auto i = 0; i < count; i++) {
    //     auto item = topLevelItem(i);
    //     for (auto listener : component_listener_list_) {
    //         listener->setComponentVisible(item->text(kName), item->checkState(kVisible));
    //         __setAllSubComponentVisibleAttribute(listener, item);
    //         // listener->componentUpdate();
    //     }
    // }
}

void ComponentsWidget::__setAllSubComponentVisibleAttribute(ComponentListener* listener, QTreeWidgetItem* item) {
    auto index = item->childCount();
    while (index) {
        index--;
        auto sub_item = item->child(index);
        listener->setComponentVisible(sub_item->text(kName), sub_item->checkState(kVisible));
        if (sub_item->childCount()) {
            __setAllSubComponentVisibleAttribute(listener, sub_item);
        }
    }
}

void ComponentsWidget::slotItemClicked(QTreeWidgetItem* item, int column) {
    if (!initiated_) {
        return;
    }
    switch (column) {
    case kName:
        break;
    case kColor: {
        auto name = item->text(kName).toLocal8Bit().constData();
        pal->getItemName(name);
        pal->exec();
    } break;
    case kVisible:
        for (auto listener : component_listener_list_) {
            listener->setComponentVisible(item->text(kName), item->checkState(column));
            __setAllSubComponentVisibleAttribute(listener, item);
            listener->componentUpdate();
        }
        break;
    case kSelectable:
        break;
    default:
        break;
    }
}

void ComponentsWidget::slotItemStyleChange(const char* item_name, QBrush brush) {
    QString name(item_name);
    if (lable_map_.find(name) != lable_map_.end()) {
        lable_map_[name]->setBrush(brush);
        lable_map_[name]->update();
        for (auto list : component_listener_list_) {
            list->setComponentBrushStyle(item_name, brush.style());
        }
    }
}

} // namespace gui
} // namespace open_edi