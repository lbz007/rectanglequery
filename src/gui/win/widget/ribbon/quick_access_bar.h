#ifndef EDI_GUI_QUICKACCESS_BAR_H_
#define EDI_GUI_QUICKACCESS_BAR_H_

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QToolBar>

namespace open_edi {
namespace gui {

class QuickAccessBar : public QToolBar {
  public:
    QuickAccessBar(QWidget* parent = nullptr);
    ~QuickAccessBar();

    void     init();
    void     addAction(QAction* action);
    QAction* addAction(const QIcon& icon, const QString& text);

    void setArrowVisible(bool visible);

  public slots:
    void quickMenuriggered(QMouseEvent* e);
    void slotArrowButtonClicked();

  private:
    QMenu*   menu_;
    QAction* separ_action_;
    QAction* arrow_action_;
};

} // namespace gui
} // namespace open_edi

#endif // QUICKACCESS_BAR_H
