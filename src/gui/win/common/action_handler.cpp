#include "action_handler.h"
#include "../actions/action_edit_undo.h"
#include "../actions/action_import_design.h"
#include "../actions/action_view_fit.h"
#include "../actions/action_view_zoomin.h"
#include "../actions/action_view_zoomout.h"
#include "../actions/action_view_refresh.h"
#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

ActionHandler::ActionHandler(QObject* parent)
  : QObject(parent), view_(nullptr) {
}

ActionHandler::~ActionHandler() {
}

void ActionHandler::setView(GraphicsView* view) {
    view_ = view;
}

ActionBase* ActionHandler::setCurrentAction(EDAGui::ActionType type) {
    ActionBase* action = nullptr;

    if (!view_) {
        return nullptr;
    }

    switch (type) {

    case EDAGui::kFileImportDesign:
        action = new ActionFileImportDesign(*view_, this);
        break;

    case EDAGui::kFileSaveDesign:
        break;

    case EDAGui::kFileSetPreference:
        break;

    case EDAGui::kEditUndo:
        action = new ActionEditUndo(*view_, this);
        break;

    case EDAGui::kEditRedo:
        break;

    case EDAGui::kEditHighLight:
        break;

    case EDAGui::kViewZoomIn:
        action = new ActionViewZoomIn(*view_, this);
        break;

    case EDAGui::kViewZoomOut:
        action = new ActionViewZoomOut(*view_, this);
        break;
    case EDAGui::kViewZoomFit:
        action = new ActionViewFit(*view_, this);
        break;
    case EDAGui::kViewRefresh:
        action = new ActionViewRefresh(*view_, this);
        break;
    default: break;
    }

    return action;
}

ActionBase* ActionHandler::getCurrentAction() {
    return nullptr;
}

void ActionHandler::slotImportDesign() {
    setCurrentAction(EDAGui::kFileImportDesign);
}

void ActionHandler::slotSaveDesign() {
    setCurrentAction(EDAGui::kFileSaveDesign);
}

void ActionHandler::slotSetPreference() {
    setCurrentAction(EDAGui::kFileSetPreference);
}

void ActionHandler::slotFindSelectObject() {
    setCurrentAction(EDAGui::kFileFindSelectObject);
}

void ActionHandler::slotEditUndo() {
    setCurrentAction(EDAGui::kEditUndo);
}

void ActionHandler::slotEditRedo() {
    setCurrentAction(EDAGui::kEditRedo);
}

void ActionHandler::slotEditHighLight() {
    setCurrentAction(EDAGui::kEditHighLight);
}

void ActionHandler::slotViewZoomIn() {
    setCurrentAction(EDAGui::kViewZoomIn);
}

void ActionHandler::slotViewZoomOut() {
    setCurrentAction(EDAGui::kViewZoomOut);
}

void ActionHandler::slotViewZoomFit() {
    setCurrentAction(EDAGui::kViewZoomFit);
}

void ActionHandler::slotViewRefresh() {
    setCurrentAction(EDAGui::kViewRefresh);
}

} // namespace gui
} // namespace open_edi
