#ifndef EDI_GUI_RIBBON_BUTTON_H_
#define EDI_GUI_RIBBON_BUTTON_H_

#include <QToolButton>

namespace open_edi {
namespace gui {

class RibbonButton : public QToolButton {
    Q_OBJECT
  public:
    enum ButtonSize{
        kLarge,
        kSmall,
        kMini
    };

    explicit RibbonButton(QWidget* parent = nullptr);
    virtual ~RibbonButton();
    void setButtonSize(const ButtonSize &size);

  signals:
    void menuActionClicked(QMouseEvent* e);

  protected:
    void mousePressEvent(QMouseEvent* e);

private:
    ButtonSize size_{kSmall};
};

} // namespace gui
} // namespace open_edi
#endif // RIBBON_BUTTON_H
