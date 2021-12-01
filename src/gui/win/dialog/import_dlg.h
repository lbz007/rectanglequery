#ifndef EDI_GUI_IMPORT_DLG_H_
#define EDI_GUI_IMPORT_DLG_H_

#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "dialog_base.h"

namespace open_edi {
namespace gui {

class ImportDlg : public DialogBase {
    Q_OBJECT

  public:
    ImportDlg(const QString& title, QWidget* parent = nullptr);
    ~ImportDlg();

  public:
    QString getLefPath();
    QString getDefPath();

  public slots:
    void slotBroswer();

  private:
    void init();

  private:
    QLineEdit* net_edit_{nullptr};
    QLineEdit* lib_edit_{nullptr};
};

} // namespace gui
} // namespace open_edi

#endif // IMPORT_DLG_H
