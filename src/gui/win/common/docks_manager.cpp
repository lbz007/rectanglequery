#include "docks_manager.h"
#include "../main_window.h"

namespace open_edi {
namespace gui {

DocksManager::DocksManager(QObject* parent)
  : QObject(parent) {
    main_window_ = qobject_cast<MainWindow*>(parent);
}

void DocksManager::createDockWidgets() {
    QDockWidget* layersDock = new QDockWidget(main_window_);
    layersDock->setObjectName("LayersDock");
    layersDock->setWindowTitle(tr("Layer"));
    layersDock->setAllowedAreas(Qt::RightDockWidgetArea);
    layer_widget = new LayerWidget(layersDock);
    layer_widget->addLayerListener(LAYOUT_GRAPHICS_VIEW->getLayout());
    layer_widget->setFocusPolicy(Qt::NoFocus);
    layersDock->setWidget(layer_widget);

    QDockWidget* componentDock = new QDockWidget(main_window_);
    componentDock->setObjectName("ComponentDock");
    componentDock->setWindowTitle(tr("Component"));
    componentDock->setAllowedAreas(Qt::RightDockWidgetArea);
    component_widget = new ComponentsWidget;
    component_widget->addComponentListener(LAYOUT_GRAPHICS_VIEW->getLayout());
    componentDock->setWidget(component_widget);

    // main_window_->tabifyDockWidget(componentDock,layersDock);

    QDockWidget* designBroswer = new QDockWidget(main_window_);
    designBroswer->setObjectName("DesignBroswer");
    designBroswer->setWindowTitle(tr("Design Broswer"));
    designBroswer->setAllowedAreas(Qt::LeftDockWidgetArea);
    QTextEdit* edit2 = new QTextEdit;
    designBroswer->setWidget(edit2);
    designBroswer->setVisible(false);

    QDockWidget* floorplanBox = new QDockWidget(main_window_);
    floorplanBox->setObjectName("FloorplanBox");
    floorplanBox->setWindowTitle(tr("Floorplan Box"));
    floorplanBox->setAllowedAreas(Qt::LeftDockWidgetArea);
    QTextEdit* edit3 = new QTextEdit;
    floorplanBox->setWidget(edit3);
    floorplanBox->setVisible(false);

    QDockWidget* worldView = new QDockWidget(main_window_);
    worldView->setObjectName("WorldView");
    worldView->setWindowTitle(tr("World View"));
    worldView->setAllowedAreas(Qt::RightDockWidgetArea);
    world_view = new WorldView();
    worldView->setWidget(world_view);
    worldView->setVisible(true);

    QDockWidget* commonAction = new QDockWidget(main_window_);
    commonAction->setObjectName("CommonAction");
    commonAction->setWindowTitle(tr("Common Action"));
    commonAction->setAllowedAreas(Qt::LeftDockWidgetArea);
    QTextEdit* edit5 = new QTextEdit;
    commonAction->setWidget(edit5);
    commonAction->setVisible(false);

    QDockWidget* navigation_panel = new QDockWidget(main_window_);
    navigation_panel->setObjectName("NavigationPanel");
    navigation_panel->setWindowTitle(tr("Navigation"));
    navigation_panel->setAllowedAreas(Qt::BottomDockWidgetArea);
    navigation_panel->setFeatures(navigation_panel->features() & ~QDockWidget::DockWidgetMovable);
    QWidget* title_widget = navigation_panel->titleBarWidget();
    QWidget* empty        = new QWidget();
    navigation_panel->setTitleBarWidget(empty);
    delete title_widget;
    navigation_panel->setVisible(false);

    main_window_->addDockWidget(Qt::LeftDockWidgetArea, designBroswer);
    main_window_->addDockWidget(Qt::LeftDockWidgetArea, floorplanBox);
    main_window_->addDockWidget(Qt::LeftDockWidgetArea, commonAction);

    main_window_->addDockWidget(Qt::RightDockWidgetArea, componentDock);
    main_window_->addDockWidget(Qt::RightDockWidgetArea, layersDock);
    main_window_->addDockWidget(Qt::RightDockWidgetArea, worldView);

    main_window_->addDockWidget(Qt::BottomDockWidgetArea, navigation_panel);
    main_window_->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    main_window_->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    main_window_->tabifyDockWidget(layersDock, componentDock);
}

} // namespace gui
} // namespace open_edi
