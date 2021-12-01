#include "ribbon_button.h"
#include "draw_helper.h"
#include <QAction>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyleOptionToolButton>
#include <QStyleOptionFocusRect>
#include <QStylePainter>

#include <QGuiApplication>
#include <QScreen>

namespace open_edi {
namespace gui {

#define LARGE_SIZE 75
#define SMALL_SIZE 25
#define MINI_SIZE 25

RibbonButton::RibbonButton(QWidget* parent) : QToolButton(parent) {
    setObjectName("RibbonToolButton");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMouseTracking(true);

    size_ = kMini;
}

RibbonButton::~RibbonButton() {
}

void RibbonButton::setButtonSize(const ButtonSize &size){
    size_ = size;

    QScreen* screen = QGuiApplication::primaryScreen();
    int height = screen->availableGeometry().height()/27;

    if(kLarge == size_){
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        setObjectName("LargeButton");
        setMinimumHeight(height);
    }else if(kSmall == size_) {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setObjectName("SmallButton");
        setMinimumHeight(height/3 -1);
    }else {
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        setObjectName("MiniButton");
        setMinimumHeight(height/3 -1);
    }
}

void RibbonButton::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && popupMode() == MenuButtonPopup) {
        if(size_ == kLarge){
//            QRect this_rect = rect();
//            QRect rect = this_rect.adjusted(0,height()/2, 0, 0);
            if(rect().isValid() && rect().contains(e->pos())){
                showMenu();
                return;
            }
        }else{
                showMenu();
                    return;
        }
    }
    QToolButton::mousePressEvent(e);
}

} // namespace gui
} // namespace open_edi
