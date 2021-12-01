#ifndef DIALOG_MANAGER_H
#define DIALOG_MANAGER_H

#include <QObject>
#include "tcl.h"

namespace open_edi {
namespace gui {

#define DIALOG_MANAGER DialogManager::getInstance()

class DialogManager : public QObject {
    Q_OBJECT
  public:
    static DialogManager* getInstance() {
        if (!instance_) {
            instance_ = new DialogManager;
        }
        return instance_;
    }

    void setTclInterp(Tcl_Interp* interp);

private:
    DialogManager(QObject* parent = nullptr);

  signals:
    void finishReadData();

  public slots:
    void openImportDesignDialog();

  private:
    static DialogManager* instance_;
    QWidget*              parent_;
    Tcl_Interp*           interp_{nullptr};
};

} // namespace gui
} // namespace open_edi

#endif // DIALOG_MANAGER_H
