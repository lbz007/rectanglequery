#include "stacked_widget.h"

namespace open_edi {
namespace gui {

StackedWidget::StackedWidget(QWidget* parent) : QStackedWidget(parent) {
    setObjectName("StackedWidget");
    setNormalMode();
}

StackedWidget::~StackedWidget() {
    if (loop_) {
        loop_->exit();
    }
    delete loop_;
}

void StackedWidget::setPopupMode() {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setFrameShape(QFrame::Panel);
}

bool StackedWidget::isPopupMode() const {
    return windowFlags() & Qt::Popup;
}

void StackedWidget::setNormalMode() {
    if (loop_) {
        loop_->exit();
        loop_ = nullptr;
    }
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    setFrameShape(QFrame::NoFrame);
}

bool StackedWidget::isNormalMode() const {
    return !isPopupMode();
}

void StackedWidget::exec() {
    setFocus();
    if (isNormalMode()) {
        loop_ = nullptr;
        return;
    }

    show();
    QEventLoop loop;
    loop_ = &loop;
    loop.exec();
    loop_ = nullptr;
}

void StackedWidget::hideEvent(QHideEvent* event) {
    if (isPopupMode()) {
        if (loop_) {
            loop_->exit();
        }
    }
    setFocus();
    emit hideWindow();
    QStackedWidget::hideEvent(event);
}

} // namespace gui
} // namespace open_edi
