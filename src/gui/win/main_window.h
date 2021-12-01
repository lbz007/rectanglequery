#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMap>
#include <QMdiArea>
#include <QApplication>
#include <QDockWidget>
#include <QScreen>

#include "./layout/graphics_view.h"
#include "tcl.h"

namespace open_edi {
namespace gui {

#define MAIN_WINDOW MainWindow::getInstance()

class ActionHandler;
class RibbonMenuBar;
class ActionGroupManager;
class DocksManager;
class StatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    friend class ActionProducer;
    friend class DocksManager;

    static MainWindow* getInstance() {
        if (!instance_) {
            instance_ = new MainWindow;
        }
        return instance_;
    }
    void setTclInterp(Tcl_Interp* intrep);

  protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void closeEvent(QCloseEvent* e) override;

  private:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void init();
    void addActions();
    void createCentralWindow();
    void loadTheme(const QString& file);

  signals:
  private slots:
    void slotInitial();
    void slotNavItemChanged(bool open);

  private:
    static MainWindow*      instance_;
    RibbonMenuBar*          ribbon_{nullptr};
    GraphicsView*           graphics_view_{nullptr};
    StatusBar*              status_bar_{nullptr};

    Tcl_Interp*             interp_{nullptr};
    ActionHandler*          action_handler_{nullptr};
    ActionGroupManager*     action_manager_{nullptr};
    DocksManager*           docks{nullptr};

    QMap<QString, QAction*> action_map_;
    QDockWidget*            navigation_panel_{nullptr};

    QScreen*                screen_{nullptr};
};

} // namespace gui
} // namespace open_edi
#endif // MAINWINDOW_H
