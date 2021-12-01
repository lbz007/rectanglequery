#include "title_bar.h"

#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScreen>

#include "util/util.h"

namespace open_edi {
namespace gui {

TitleBar::TitleBar(QObject* parent)
  : QObject(parent)
  , title_(new QLabel)
  , cursor_shape_changed_(false)
  , edge_pressed_(false)
  , maximized_(false)
  , minimized_(false)
  , move_point_(QPoint(0, 0)) {

    setObjectName("TitleBar");
    main_window_ = qobject_cast<QWidget*>(parent);
    main_window_->setMouseTracking(true);
    main_window_->setAttribute(Qt::WA_Hover, true);

    QRect rect   = QGuiApplication::primaryScreen()->availableGeometry();
    normal_rect_ = QRect(rect.x() + 100, rect.y() + 100, 2 * rect.width() / 3, 2 * rect.height() / 3);

    main_window_->installEventFilter(this);

    init();
}

TitleBar::~TitleBar() {
}

void TitleBar::init() {

    icon_ = new QLabel(main_window_);
    icon_->setFixedHeight(22);
    icon_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    icon_->setScaledContents(true);

    title_ = new QLabel(main_window_);
    title_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    minimize_bt_ = new QToolButton(main_window_);
    minimize_bt_->setFixedSize(27, 22);
    minimize_bt_->setObjectName("minimizeButton");
    minimize_bt_->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/min.png"));
    minimize_bt_->setToolTip(tr("Minimize"));

    maximize_bt_ = new QToolButton(main_window_);
    maximize_bt_->setFixedSize(27, 22);
    maximize_bt_->setObjectName("maximizeButton");
    maximize_bt_->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/max.png"));
    maximize_bt_->setToolTip(tr("Maximize"));

    close_bt_ = new QToolButton(main_window_);
    close_bt_->setFixedSize(27, 22);
    close_bt_->setObjectName("closeButton");
    close_bt_->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/close.png"));
    close_bt_->setToolTip(tr("Close"));

    system_group_ = new QWidget(main_window_);
    system_group_->setObjectName(tr("SystemGroupWidget"));
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(minimize_bt_);
    layout->addWidget(maximize_bt_);
    layout->addWidget(close_bt_);
    system_group_->setLayout(layout);
    system_group_->setVisible(false);

    setProperty("RibbonTitleBar", true);
    setObjectName("RibbonTitleBar");

    connect(minimize_bt_, &QToolButton::clicked, this, &TitleBar::slotSystemClicked);
    connect(maximize_bt_, &QToolButton::clicked, this, &TitleBar::slotSystemClicked);
    connect(close_bt_, &QToolButton::clicked, this, &TitleBar::slotSystemClicked);
}

void TitleBar::setWindowTitle(const QString& title) {
    title_->setAlignment(Qt::AlignCenter);
    title_->setText(title);
}

void TitleBar::setTitleBarIcon(const QString& icon) {
    icon_->setPixmap(QPixmap(icon));
}

void TitleBar::addQuickAction(QAction* action) {
    Q_UNUSED(action)
}

QAction* TitleBar::addQuickAction(const QIcon& icon, const QString& text) {
    Q_UNUSED(icon)
    Q_UNUSED(text)
    return nullptr;
}

QToolButton* TitleBar::getCloseButton(){
    return close_bt_;
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* e) {
    Q_UNUSED(e)

    maximize_bt_->click();
}

void TitleBar::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        left_button_pressed = true;

        if (main_window_->isTopLevel()) {
            if (maximized_) {
                move_point_ = e->globalPos() - calcStartPoint(main_window_, e);
            } else {
                move_point_ = e->globalPos() - main_window_->pos();
            }
        }
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent* e) {

    if (!left_button_pressed) {
        return;
    }

    if (main_window_->isTopLevel()) {
        if (maximized_) {
            if (e->globalY() > 2 * move_cursor_.border_width_) {
                move_point_ = e->globalPos() - calcStartPoint(main_window_, e);
                maximized_  = false;
                main_window_->setGeometry(normal_rect_);
            }
        } else {
            if (press_cursor_.on_edges_) {
                e->ignore();
            } else {
                main_window_->move(e->globalPos() - move_point_);
            }
        }
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent* e) {
    left_button_pressed = false;

    if (!maximized_) {
        if (e->globalY() <= 3) {
            main_window_->move(main_window_->frameGeometry().x(), 10);
            maximize_bt_->click();
        } else {
            int y = main_window_->frameGeometry().y();

            if (y < 0) {
                main_window_->move(main_window_->frameGeometry().x(), 10);
            }
        }
    }
}

bool TitleBar::eventFilter(QObject* obj, QEvent* e) {
    switch (e->type()) {
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        handleMouseEvent(obj, e);
        return QObject::eventFilter(obj, e);
    default:
        break;
    }

    return QObject::eventFilter(obj, e);
}

void TitleBar::handleMouseEvent(QObject* obj, QEvent* e) {
    Q_UNUSED(obj);

    switch (e->type()) {
    case QEvent::MouseButtonPress:
        handleMousePressEvent(static_cast<QMouseEvent*>(e));
        break;
    case QEvent::MouseButtonRelease:
        handleMouseRelaseEvent(static_cast<QMouseEvent*>(e));
        break;
    case QEvent::MouseMove:
        handleMouseMoveEvent(static_cast<QMouseEvent*>(e));
        break;
    case QEvent::Leave:
        handleLeaveEvent(static_cast<QMouseEvent*>(e));
        break;
    case QEvent::HoverMove:
        handleHoverMoveEvent(static_cast<QHoverEvent*>(e));
        break;
    default:
        break;
    }
}

void TitleBar::handleMousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && !maximized_) {
        edge_pressed_ = true;
        press_cursor_.recalculate(e->globalPos(), main_window_->frameGeometry());
    }
}

void TitleBar::resizeWidget(const QPoint& mouse_pos) {

    QRect origin_rect = main_window_->frameGeometry();
    int   left        = origin_rect.left();
    int   top         = origin_rect.top();
    int   right       = origin_rect.right();
    int   bottom      = origin_rect.bottom();
    origin_rect.getCoords(&left, &top, &right, &bottom);

    int min_width  = main_window_->minimumWidth();
    int min_height = main_window_->minimumHeight();

    if (press_cursor_.on_top_left_edge_) {
        left = mouse_pos.x();
        top  = mouse_pos.y();
    } else if (press_cursor_.on_bottom_left_edge_) {
        left   = mouse_pos.x();
        bottom = mouse_pos.y();
    } else if (press_cursor_.on_top_right_edge_) {
        right = mouse_pos.x();
        top   = mouse_pos.y();
    } else if (press_cursor_.on_bottom_right_edge_) {
        right  = mouse_pos.x();
        bottom = mouse_pos.y();
    } else if (press_cursor_.on_left_edge_) {
        left = mouse_pos.x();
    } else if (press_cursor_.on_right_edge_) {
        right = mouse_pos.x();
    } else if (press_cursor_.on_top_edge_) {
        top = mouse_pos.y();
    } else if (press_cursor_.on_bottom_edge_) {
        bottom = mouse_pos.y();
    }

    QRect new_rect(QPoint(left, top), QPoint(right, bottom));
    if (new_rect.isValid()) {
        if (min_width > new_rect.width()) {
            if (left != origin_rect.left()) {
                new_rect.setLeft(origin_rect.left());
            } else {
                new_rect.setRight(origin_rect.right());
            }
        }
        if (min_height > new_rect.height()) {
            if (top != origin_rect.top()) {
                new_rect.setTop(origin_rect.top());
            } else {
                new_rect.setBottom(origin_rect.bottom());
            }
        }
        main_window_->setGeometry(new_rect);
    }
}

QPoint TitleBar::calcStartPoint(QWidget* pWindow, QMouseEvent* event) const {
    int   mouseX   = event->globalX();
    QRect rect     = main_window_->frameGeometry();
    int   maxWidth = rect.x() + rect.width();
    int   screenX  = rect.x();
    int   oriWidth = normal_rect_.width();

    if (oriWidth == 0) {
        oriWidth = pWindow->minimumWidth();
    }

    QPoint point;
    point.setY(0);

    if (mouseX - screenX < oriWidth / 2) {
        point.setX(screenX); // Align screen left
    } else if (maxWidth - mouseX < oriWidth / 2) {
        point.setX(maxWidth - oriWidth); // Align screen right
    } else {
        point.setX(mouseX - oriWidth / 2);
    }

    return point;
}

void TitleBar::updateCursorShape(const QPoint& mouse_pos) {
    if (main_window_->isFullScreen() || main_window_->isMaximized()) {
        if (cursor_shape_changed_) {
            main_window_->unsetCursor();
            cursor_shape_changed_ = false;
        }
        return;
    }

    move_cursor_.recalculate(mouse_pos, main_window_->frameGeometry());

    if (move_cursor_.on_top_left_edge_ || move_cursor_.on_bottom_right_edge_) {
        main_window_->setCursor(Qt::SizeFDiagCursor);
        cursor_shape_changed_ = true;
    } else if (move_cursor_.on_top_right_edge_ || move_cursor_.on_bottom_left_edge_) {
        main_window_->setCursor(Qt::SizeBDiagCursor);
        cursor_shape_changed_ = true;
    } else if (move_cursor_.on_left_edge_ || move_cursor_.on_right_edge_) {
        main_window_->setCursor(Qt::SizeHorCursor);
        cursor_shape_changed_ = true;
    } else if (move_cursor_.on_top_edge_ || move_cursor_.on_bottom_edge_) {
        main_window_->setCursor(Qt::SizeVerCursor);
        cursor_shape_changed_ = true;
    } else {
        if (cursor_shape_changed_) {
            main_window_->unsetCursor();
            cursor_shape_changed_ = false;
        }
    }
}

void TitleBar::handleMouseMoveEvent(QMouseEvent* e) {
    if (edge_pressed_) {
        if (press_cursor_.on_edges_) {
            resizeWidget(e->globalPos());
        }
    } else {
        updateCursorShape(e->globalPos());
    }
}

void TitleBar::handleMouseRelaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        edge_pressed_ = false;
        press_cursor_.reset();
        updateCursorShape(e->globalPos());
    }
}

void TitleBar::handleLeaveEvent(QMouseEvent* e) {
    Q_UNUSED(e)

    if (!edge_pressed_)
        main_window_->unsetCursor();
}

void TitleBar::handleHoverMoveEvent(QHoverEvent* e) {
    if (!edge_pressed_) {
        updateCursorShape(main_window_->mapToGlobal(e->pos()));
    }
}

void TitleBar::slotSystemClicked() {
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (main_window_) {
        if (button == minimize_bt_) {
            main_window_->showMinimized();
            minimized_ = true;
        } else if (button == maximize_bt_) {
            if (maximized_) {
                main_window_->showNormal();
                maximized_ = false;
                maximize_bt_->setToolTip(tr("Maximize"));
                maximize_bt_->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/max.png"));
            } else {
                normal_rect_ = main_window_->frameGeometry();
                main_window_->showMaximized();
                maximized_ = true;
                maximize_bt_->setToolTip(tr("Restore"));
                maximize_bt_->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/restore.png"));
            }
            minimized_ = false;

        } else if (button == close_bt_) {
            main_window_->close();
        }
    }
}

} // namespace gui
} // namespace open_edi
