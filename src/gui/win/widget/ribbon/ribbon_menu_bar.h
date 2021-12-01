#ifndef RIBBON_H
#define RIBBON_H

#include <QMap>
#include <QMenuBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTabBar>

namespace open_edi {
namespace gui {

class RibbonTitleBar;
class RibbonPage;
class StackedWidget;
class ApplicationButton;

class RibbonMenuBar : public QMenuBar {
    Q_OBJECT
  public:
    explicit RibbonMenuBar(QWidget* parent = nullptr);
    ~RibbonMenuBar();

    void init();
    void fillActions(QMap<QString, QAction*> map);

    RibbonPage*     addPage(const QString& text);
    QPushButton*    getFileButton() const;
    int  pageCount() const;

    void setHideMode(bool hide);
    bool isRibbonHideMode() const;

    void setRibbonHeight(int height);

  signals:
    void currentTabChanged(int);

  public slots:
    void slotThemeChanged();

    void slotStackedWidgetHided();
    void slotPageTitleChanged(const QString& text);
    void slotTabBarChanged(int index);
    void slotTabBarClicked(int index);
    void slotTabBarDoubleClicked(int index);
    void slotPageShowHide(bool show);

  private:
    ApplicationButton*         file_button_;
    QTabBar*                   tab_bar_;
    StackedWidget*             stacked_widget_;
    RibbonTitleBar*            title_bar_;

    int                     height_;
    QMap<QString, QAction*> map_;
    QMenu*                  menu_;
};

} // namespace gui
} // namespace open_edi

#endif // RIBBON_H
