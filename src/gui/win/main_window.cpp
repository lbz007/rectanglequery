#include "main_window.h"

#include "common/action_group_manager.h"
#include "common/action_handler.h"
#include "common/action_producer.h"
#include "common/dialog_manager.h"
#include "common/docks_manager.h"
#include "dialog/import_dlg.h"
#include "widget/ribbon/ribbon_file_menu.h"
#include "widget/ribbon/ribbon_menu_bar.h"
#include "widget/status/status_bar.h"

#include <QApplication>
#include <QDockWidget>
#include <QStatusBar>

#include "util/util.h"

namespace open_edi {
namespace gui {

MainWindow* MainWindow::instance_ = nullptr;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent) {
    setObjectName("MainWindow");
    setAcceptDrops(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    char* framless = getenv("FULL_FRAME"); // remove protect until regression update.
    if (!framless) setWindowFlags(Qt::FramelessWindowHint);
    screen_             = QGuiApplication::primaryScreen();
    QRect rect          = screen_->availableGeometry();
    int   screen_width  = rect.width();
    int   screen_height = rect.height();
    setMinimumSize((screen_width / 2) * 1, (screen_height / 5) * 4);

    graphics_view_ = GraphicsView::getInstance();

    init();
    addActions();
    createCentralWindow();
    loadTheme(QString::fromStdString(open_edi::util::getResourcePath()) + "qss/default.qss");
}

MainWindow::~MainWindow() {
}

void MainWindow::setTclInterp(Tcl_Interp* interp) {
    interp_ = interp;
    DIALOG_MANAGER->setTclInterp(interp_);
    connect(DIALOG_MANAGER, SIGNAL(finishReadData()), this, SLOT(slotInitial()));
    // connect(DIALOG_MANAGER, SIGNAL(finishReadData()), docks->layer_widget, SLOT(refreshTree()));
}

void MainWindow::closeEvent(QCloseEvent* e) {
    hide();
    e->ignore();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* e) {
    if (obj == ribbon_) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::Leave:
        case QEvent::HoverMove:
        case QEvent::MouseButtonDblClick:
            QApplication::sendEvent(this, e);
            break;

        default:
            break;
        }
    }

    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::init() {
    ribbon_ = new RibbonMenuBar(this);
    setMenuWidget(ribbon_);
    ribbon_->installEventFilter(this);
    ribbon_->setRibbonHeight(screen_->availableGeometry().height() / 9);
    ribbon_->show();

    action_handler_ = new ActionHandler(this);
    action_handler_->setView(graphics_view_);
    action_manager_          = new ActionGroupManager(this);
    ActionProducer* producer = new ActionProducer(this, action_handler_);
    producer->fillActionContainer(action_map_, action_manager_);
    producer->addOtherAction(action_map_, action_manager_);

    docks = new DocksManager(this);
    docks->createDockWidgets();

    QMenu* menu = new QMenu;
    foreach (auto dock, findChildren<QDockWidget*>()) {
        menu->addAction(dock->toggleViewAction());
        if (dock->allowedAreas() == Qt::BottomDockWidgetArea) {
            navigation_panel_ = dock;
        }
    }
    action_map_["Window"]->setMenu(menu);

    status_bar_ = new StatusBar(this);
    navigation_panel_->setWidget(status_bar_->panel_stack_widget_);
    status_bar_->createNavigations();
    setStatusBar(status_bar_);
    statusBar()->setMaximumHeight(30);
    status_bar_->installEventFilter(this);

    connect(status_bar_, &StatusBar::signalTilteChanged, this, &MainWindow::slotNavItemChanged);

    connect(status_bar_, &StatusBar::signalTilteChanged, this, &MainWindow::slotNavItemChanged);

    connect(graphics_view_, &GraphicsView::sendPos, status_bar_, &StatusBar::slotGetMouseLocation);

    connect(graphics_view_, &GraphicsView::sendSelectedBox, docks->world_view, &WorldView::slotSyncPosition);

    connect(docks->world_view, &WorldView::sendSelectedBox, graphics_view_, &GraphicsView::slotUpdatePosition);

    connect(status_bar_->slide, &QSlider::valueChanged, graphics_view_, &GraphicsView::slotSetHighlightMaskValue);

    connect(graphics_view_, &GraphicsView::initByCmd, this, &MainWindow::slotInitial);
}

void MainWindow::addActions() {
    RibbonFileMenu* menu = qobject_cast<RibbonFileMenu*>(ribbon_->getFileButton()->menu());
    menu->addFileAction(action_map_["ImportDesign"], Qt::ToolButtonTextBesideIcon);

    ribbon_->fillActions(action_map_);
}

void MainWindow::createCentralWindow() {

    setCentralWidget(graphics_view_);
}

void MainWindow::loadTheme(const QString& file) {

    QFile theme_file(file);

    if (!theme_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    qApp->setStyleSheet(theme_file.readAll());
}

void MainWindow::slotInitial() {
    if (graphics_view_->isInInitial()) {
        graphics_view_->initView();
        QList<LI_Layer*> layer_list;
        docks->layer_widget->refreshTree(graphics_view_->getLayout()->getLayerList());

        docks->world_view->initDieArea(graphics_view_->getLayout()->getDieAreaW(),
                                       graphics_view_->getLayout()->getDieAreaH());

        docks->component_widget->refreshComponentAttributes();
    }
    graphics_view_->setStatusToNormal();
    graphics_view_->refreshFitDraw();
}

void MainWindow::slotNavItemChanged(bool open) {
    if (!open) {
        navigation_panel_->setVisible(false);
    } else {
        navigation_panel_->setVisible(true);
    }
    //    navigation_panel_->setWindowTitle(text);
}

} // namespace gui
} // namespace open_edi
