#ifndef EDI_GUI_STACKED_WIDGET_H
#define EDI_GUI_STACKED_WIDGET_H

#include <QEventLoop>
#include <QStackedWidget>

namespace open_edi {
namespace gui {

class StackedWidget : public QStackedWidget {
    Q_OBJECT
  public:
    StackedWidget(QWidget* parent = nullptr);
    ~StackedWidget();

    void setPopupMode();
    bool isPopupMode() const;
    void setNormalMode();
    bool isNormalMode() const;
    void exec();

  protected:
    void hideEvent(QHideEvent* event);

  signals:
    void hideWindow();

  private:
    QEventLoop* loop_{nullptr};
};

} // namespace gui
} // namespace open_edi

#endif // STACKED_WIDGET_H
