#ifndef EDI_GUI_NAVIGATION_BAR_H_
#define EDI_GUI_NAVIGATION_BAR_H_

#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QSlider>

namespace open_edi {
namespace gui {

class NavButton;

class StatusBar : public QStatusBar {
    Q_OBJECT
  public:
    explicit StatusBar(QWidget* parent = nullptr);
    ~StatusBar();

    void init();
    void createNavigations();
    void addNavigation(const QString& title, QWidget* widget);

  signals:
    void signalTilteChanged(bool);

  public slots:
    void slotMenuButtonClicked();
    void slotTabItemClicked(bool clicked);
    void slotToggleNavigation(bool toggled);
    void slotGetMouseLocation(double x, double y);

  public:
    QStackedWidget* panel_stack_widget_{nullptr};
    QSlider*        slide{nullptr};

  private:
    QMap<NavButton*, QWidget*> navi_map_;
    QMap<QAction*, NavButton*> toggle_map_;
    QToolBar*                  item_tar_{nullptr};
    QAction*                   menu_action{nullptr};
    QLabel*                    mouse_location_{nullptr};
    QMenu*                     toggle_menu_{nullptr};
    NavButton*                 current_selected_{nullptr};
};

} // namespace gui
} // namespace open_edi

#endif // NAVIGATION_BAR_H
