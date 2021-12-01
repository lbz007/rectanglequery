#include "nav_button.h"
#include <QPainter>
#include <QMouseEvent>

namespace open_edi {
namespace gui {

NavButton::NavButton(QWidget* parent)
    : QToolButton (parent)
{
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setAutoRaise(false);
    setArrowType(Qt::NoArrow);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void NavButton::select(bool selected){
    mouse_press_ = selected;
    if(!mouse_press_)
        mouse_over_ = false;

    update();
}

void NavButton::paintEvent(QPaintEvent* e){

    QPainter painter(this);
    QPen pen(Qt::NoBrush, 1);
    painter.setPen(pen);
    if(mouse_press_){
        painter.setBrush(QColor(0,0,0,100));
    }else if (mouse_over_) {
        painter.setBrush(QColor(255,255,255,50));
    }
    painter.drawRect(rect());

    QToolButton::paintEvent(e);
}

void NavButton::enterEvent(QEvent* e){
    Q_UNUSED(e)

    if(!isEnabled())
        return;

    mouse_over_ = true;
}

void NavButton::leaveEvent(QEvent* e){
    Q_UNUSED(e)

    mouse_over_ = false;
}

void NavButton::mousePressEvent(QMouseEvent* e){
    if(e->button() == Qt::LeftButton){
        if(isCheckable()){
            if(isChecked()){
                mouse_press_ = false;
            }else {
                mouse_press_ = true;
            }
            setChecked(mouse_press_);
            update();
        }else{
            mouse_press_ = true;
            emit click();
        }
    }
}

}
}
