
#include "layer_widget.h"

#include <QDebug>
#include "../../layout/items/li_layer.h"

namespace open_edi {
namespace gui {

LayerWidget::LayerWidget(QWidget* parent) : TreeWidgetBase(parent) {

    pal = new Palette(
      {
        QBrush("White"),
        QBrush("Red"),
        QBrush("Green"),
        QBrush("Blue"),
        QBrush("Yellow"),
        QBrush("#FF00FF"),
        QBrush("#00FFFF"),
        QBrush("#780078"),
        QBrush("#FFC0C0"),
        QBrush("#FFA800"),
      },
      true);

    pal->setName("Color: ");

    layer_top_item_ = __createTopItem(this, "Layer");

    layer_masterslice_item_ = __createSubItem(layer_top_item_, "Masterslice");
    layer_routing_item_     = __createSubItem(layer_top_item_, "Routing");
    layer_backside_item_    = __createSubItem(layer_top_item_, "Backside");

    connect(this, &LayerWidget::itemClicked, this, &LayerWidget::slotItemClicked);
    connect(pal, &Palette::signalBtnOKClicked, this, &LayerWidget::slotItemColorChange);
}

LayerWidget::~LayerWidget() {
}

void LayerWidget::addLayerListener(LayerListener* listener) {
    layer_listener_list_.append(listener);
}

void LayerWidget::slotItemColorChange(const char* item_name, QBrush brush) {
    QString name(item_name);
    if (lable_map_.find(name) != lable_map_.end()) {
        lable_map_[name]->setBrush(brush);
        lable_map_[name]->update();

        for (auto list : layer_listener_list_) {
            list->setLayerColor(item_name, brush.color());
            list->layerUpdate();
        }
    }
}

void LayerWidget::refreshTree(QList<LI_Layer*> layer_list) {

    QBrush brush_array[10]{
      QBrush("Blue"),
      QBrush("Red"),
      QBrush("Green"),
      QBrush("Orange"),
      QBrush("Yellow"),
      QBrush("#FF00FF"),
      QBrush("#00FFFF"),
      QBrush("#780078"),
      QBrush("#FFC0C0"),
      QBrush("#FFA800")};

    int i_backside  = 0;
    int i_roughing  = 0;
    int masterslice = 0;

    // isBackside
    for (auto& li_layer : layer_list) {

        auto layer = li_layer->getLayer();

        if (layer->isBackside()) {
            auto index = i_backside++ % 10;
            __createSubItems(layer_backside_item_,
                             {
                               {ItemAttributes{layer->getName(),
                                               brush_array[index]}},
                             });

            QPen pen(brush_array[index].color());
            li_layer->setPen(pen);
        } else if (layer->isRouting() || layer->isCut()) {
            auto index = i_roughing++ % 10;
            __createSubItems(layer_routing_item_,
                             {
                               {ItemAttributes{layer->getName(),
                                               brush_array[index]}},
                             });

            QPen pen(brush_array[index].color());
            li_layer->setPen(pen);

        } else {
            // qDebug()<<layer->getName()<<" "<<layer->getType();
            auto index = masterslice++ % 10;
            __createSubItems(layer_masterslice_item_,
                             {
                               {ItemAttributes{layer->getName(),
                                               brush_array[index]}},
                             });

            QPen pen(brush_array[index].color());
            li_layer->setPen(pen);
        }
    }

    layer_top_item_->setExpanded(true);
    layer_routing_item_->setCheckState(kVisible, Qt::Checked);
}

void LayerWidget::__setAllSubLayerVisibleAttribute(LayerListener* listener, QTreeWidgetItem* item) {
    auto index = item->childCount();
    while (index) {
        index--;
        auto sub_item = item->child(index);
        listener->setLayerVisible(sub_item->text(kName), sub_item->checkState(kVisible));
        if (sub_item->childCount()) {
            __setAllSubLayerVisibleAttribute(listener, sub_item);
        }
    }
}

void LayerWidget::slotItemClicked(QTreeWidgetItem* item, int column) {

    switch (column) {
    case kName:
        break;
    case kColor: {
        auto name = item->text(kName).toLocal8Bit().constData();
        pal->getItemName(name);
        pal->exec();
    } break;
    case kVisible:
        for (auto listener : layer_listener_list_) {
            listener->setLayerVisible(item->text(kName), item->checkState(column));
            __setAllSubLayerVisibleAttribute(listener, item);
            listener->layerUpdate();
        }
        break;
    case kSelectable:
        break;
    default:
        break;
    }
}

} // namespace gui
} // namespace open_edi