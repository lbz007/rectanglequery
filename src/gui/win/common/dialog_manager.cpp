#include "dialog_manager.h"
#include "../dialog/import_dlg.h"

namespace open_edi {
namespace gui {

DialogManager* DialogManager::instance_ = nullptr;

DialogManager::DialogManager(QObject* parent)
  : QObject(parent) {
    parent_ = qobject_cast<QWidget*>(parent);
}

void DialogManager::openImportDesignDialog() {
    ImportDlg dlg(tr("Import Design"), parent_);
    if (dlg.exec()) {
        QString lef_cmd = "read_lef " + dlg.getLefPath();
        QString def_cmd = "read_def " + dlg.getDefPath();

        if(interp_){
            Tcl_Eval(interp_, lef_cmd.toLocal8Bit());
            Tcl_Eval(interp_, def_cmd.toLocal8Bit());

            emit finishReadData();
        }
    }
}

void DialogManager::setTclInterp(Tcl_Interp* interp) {
    interp_ = interp;
}

} // namespace gui
} // namespace open_edi
