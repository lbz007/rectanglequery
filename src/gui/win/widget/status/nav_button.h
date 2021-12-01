#ifndef EDI_GUI_NAV_BUTTON_H_
#define EDI_GUI_NAV_BUTTON_H_

#include <QToolButton>

namespace open_edi {
namespace gui {

class NavButton : public QToolButton
{
    Q_OBJECT
public:
    explicit NavButton(QWidget* parent = nullptr);

    void select(bool selected);

protected:
    void paintEvent(QPaintEvent* e);
    void enterEvent(QEvent* e);
    void leaveEvent(QEvent* e);
    void mousePressEvent(QMouseEvent* e);

private:
    bool mouse_over_{false};
    bool mouse_press_{false};
};

}
}

#endif // NAV_BUTTON_H
