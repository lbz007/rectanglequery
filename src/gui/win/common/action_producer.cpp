#include "action_producer.h"
#include "action_group_manager.h"
#include "action_handler.h"
#include "util/util.h"
#include "dialog_manager.h"
#include <QAction>
#include <QActionGroup>

namespace open_edi {
namespace gui {

ActionProducer::ActionProducer(QObject* parent, QObject* handler)
  : QObject(parent)
  , action_hander_(handler)
  , main_window_(parent) {
}

void ActionProducer::addOtherAction(QMap<QString, QAction*>& map, ActionGroupManager* manager){
    QAction* action = nullptr;

    action = new QAction(tr("Window"), manager->other_);
    map["Window"] = action;
}

void ActionProducer::fillActionContainer(QMap<QString, QAction*>& map, ActionGroupManager* manager) {

    QString res_path = QString::fromStdString(open_edi::util::getResourcePath()) + "tool/";
    QString res_name = res_path + "undo.svg";

    QAction* action = nullptr;

    /*************file****************************/

    action = new QAction(QIcon(res_path + "import.svg"), tr("Import Design"), manager->file_);
    action->setObjectName("ImportDesign");
    connect(action, SIGNAL(triggered()), DIALOG_MANAGER, SLOT(openImportDesignDialog()));
    map["ImportDesign"] = action;

    action = new QAction(QIcon(res_name), tr("Save Design"), manager->file_);
    action->setObjectName("SaveDesign");
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotSaveDesign()));
    map["SaveDesign"] = action;

    action = new QAction(QIcon(res_name), tr("Set Preference"), manager->file_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotSetPreference()));
    action->setObjectName("SetPreference");
    map["SetPreference"] = action;

    action = new QAction(QIcon(res_name), tr("Find/Select Object"), manager->file_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotFindSelectObject()));
    action->setObjectName("FindSelectObject");
    map["FindSelectObject"] = action;

    /*************edit****************************/

    action = new QAction(QIcon(res_path + "select.svg"), tr("Select"), manager->edit_);
    action->setObjectName("Select");
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotEditUndo()));
    map["Select"] = action;

    action = new QAction(QIcon(res_path + "undo.svg"), tr("Undo"), manager->edit_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotEditUndo()));
    action->setObjectName("Undo");
    map["Undo"] = action;

    action = new QAction(QIcon(res_path + "redo.png"), tr("Redo"), manager->edit_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotEditRedo()));
    action->setObjectName("Redo");
    map["Redo"] = action;

    action = new QAction(QIcon(res_name), tr("HighLight"), manager->edit_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotEditHighLight()));
    action->setObjectName("HighLight");
    map["HighLight"] = action;

    /*************view****************************/

    action = new QAction(QIcon(res_path + "zoomin.png"), tr("Zoom In"), manager->view_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotViewZoomIn()));
    action->setObjectName("ZoomIn");
    map["ZoomIn"] = action;

    action = new QAction(QIcon(res_path + "zoomout.png"), tr("Zoom Out"), manager->view_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotViewZoomOut()));
    action->setObjectName("ZoomOut");
    map["ZoomOut"] = action;

    action = new QAction(QIcon(res_path + "zoomfit.png"), tr("Zoom Fit"), manager->view_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotViewZoomFit()));
    action->setObjectName("ZoomFit");
    map["ZoomFit"] = action;

    action = new QAction(QIcon(res_path + "refresh.svg"), tr("Refresh"), manager->view_);
    connect(action, SIGNAL(triggered()), action_hander_, SLOT(slotViewRefresh()));
    action->setObjectName("Refresh");
    map["Refresh"] = action;
}

} // namespace gui
} // namespace open_edi
