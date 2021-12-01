#include "ribbon_title_bar.h"

#include <QMouseEvent>

#include "../title_bar.h"
#include "quick_access_bar.h"
#include "util/util.h"

namespace open_edi {
namespace gui {

RibbonTitleBar::RibbonTitleBar(QWidget* parent)
  : QWidget(parent) {
    title_bar_ = new TitleBar(parent);
    init();
}

RibbonTitleBar::~RibbonTitleBar() {
}

void RibbonTitleBar::init() {
    quick_access_ = new QuickAccessBar(this);
    quick_access_->hide();

    other_layout_ = new QHBoxLayout;
    other_layout_->setContentsMargins(0, 0, 0, 0);
    other_layout_->setSpacing(0);

    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->addWidget(quick_access_);
    main_layout->addStretch(1);
    main_layout->addWidget(title_bar_->title_);
    main_layout->addStretch(1);
    main_layout->addLayout(other_layout_);
    main_layout->addWidget(title_bar_->system_group_);
    title_bar_->system_group_->setVisible(true);
    setLayout(main_layout);
}

void RibbonTitleBar::addOtherControl(QAction* action) const{
    QToolButton* button = new QToolButton();
    button->setFixedSize(27,22);
    button->setObjectName("OtherControl");
    button->setDefaultAction(action);
    button->setPopupMode(QToolButton::InstantPopup);
    other_layout_->addWidget(button);
}

void RibbonTitleBar::setWindowTitle(const QString& title) const {
    title_bar_->setWindowTitle(title);
}

void RibbonTitleBar::mousePressEvent(QMouseEvent* e) {
    title_bar_->mousePressEvent(e);
    e->ignore();
}

void RibbonTitleBar::mouseReleaseEvent(QMouseEvent* e) {
    title_bar_->mouseReleaseEvent(e);
    e->ignore();
}

void RibbonTitleBar::mouseMoveEvent(QMouseEvent* e) {
    title_bar_->mouseMoveEvent(e);
}

void RibbonTitleBar::mouseDoubleClickEvent(QMouseEvent* e) {
    title_bar_->mouseDoubleClickEvent(e);
}

} // namespace gui
} // namespace open_edi
