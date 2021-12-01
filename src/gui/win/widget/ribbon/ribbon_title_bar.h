#ifndef EDI_GUI_RIBBON_TITLE_BAR_H_
#define EDI_GUI_RIBBON_TITLE_BAR_H_

#include <QToolButton>
#include <QWidget>
#include <QHBoxLayout>

namespace open_edi {
namespace gui {

class QuickAccessBar;
class TitleBar;

class RibbonTitleBar : public QWidget {
    Q_OBJECT

  public:
    RibbonTitleBar(QWidget* parent = nullptr);
    ~RibbonTitleBar();

    void init();
    void setWindowTitle(const QString& title) const;
    void addOtherControl(QAction* action) const;

  protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);

  private:
    QuickAccessBar* quick_access_;
    TitleBar*       title_bar_;
    QHBoxLayout*    other_layout_;
};

} // namespace gui
} // namespace open_edi

#endif // RIBBON_TITLE_BAR_H
