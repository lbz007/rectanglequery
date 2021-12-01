#include "ribbon_group.h"
#include "ribbon_button.h"

#include <QMenu>
#include <QPaintEvent>
#include <QPainter>

namespace open_edi {
namespace gui {

RibbonGroup::RibbonGroup(QString title, QWidget* parent)
  : QWidget(parent) {
    setObjectName("RibbonGroupWidget");
    init();
    setTitle(title);
}

RibbonGroup::~RibbonGroup() {
}

void RibbonGroup::init() {

    grid_layout_ = new QGridLayout();
    grid_layout_->setSpacing(3);
    grid_layout_->setContentsMargins(0, 1, 1, 5);

    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0,0,0,0);
    QWidget* title_widget = new QWidget(this);
    title_widget->setObjectName("RibbonTitleWidget");
    title_widget->setLayout(titleLayout);
    group_title_ = new QLabel(title_widget);
    QFont font;
    font.setPixelSize(13);
    group_title_->setFont(font);
    group_title_->setFixedHeight(15);
    group_title_->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    titleLayout->addWidget(group_title_);

    pop_button_ = new RibbonButton(title_widget);
    pop_button_->setText(">");
    pop_button_->setFixedSize(QSize(9, 9));
    pop_button_->setVisible(false);
    //   connect(pop_button_, SIGNAL(triggered(QAction*)), this, SIGNAL(popButtonClicked()));
    titleLayout->addWidget(pop_button_);

    QVBoxLayout* bodyLayout = new QVBoxLayout;
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->addLayout(grid_layout_);
    bodyLayout->addWidget(title_widget);

    QWidget* line = new QWidget;
    line->setFixedWidth(1);
    line->setAutoFillBackground(true);
    line->setBackgroundRole(QPalette::Mid);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(5,5,0,0);
    mainLayout->addLayout(bodyLayout);
    mainLayout->addWidget(line);
    setLayout(mainLayout);
}

QMenu* RibbonGroup::addMenu(const QIcon& icon, const QString& text, Qt::ToolButtonStyle style) {
    Q_UNUSED(style)

    QMenu*   menu   = new QMenu(text, this);
    QAction* action = menu->menuAction();
    action->setIcon(icon);
    return menu;
}

void RibbonGroup::setTitle(const QString title) {
    group_title_->setText(title);
    group_title_->setAlignment(Qt::AlignCenter);
}

RibbonButton* RibbonGroup::addLargeAction(QAction* action){

    RibbonButton* button = new RibbonButton(this);
    button->setButtonSize(RibbonButton::kLarge);
    button->setAutoRaise(true);
    button->setDefaultAction(action);
    if(action->menu())
        button->setPopupMode(QToolButton::MenuButtonPopup);

    grid_layout_->addWidget(button, 0, grid_layout_->columnCount(), 3, 1);
    current_row_ = 0;
    addAction(action);

    return button;
}

RibbonButton* RibbonGroup::addSmallAction(QAction* action){

    RibbonButton* button = new RibbonButton(this);
    button->setButtonSize(RibbonButton::kSmall);
    button->setAutoRaise(true);
    button->setDefaultAction(action);
    if(action->menu())
        button->setPopupMode(QToolButton::MenuButtonPopup);
    if(0 == current_row_){
        grid_layout_->addWidget(button, current_row_, grid_layout_->columnCount(), 1, 1);
    }else {
        grid_layout_->addWidget(button, current_row_, grid_layout_->columnCount()-1, 1, 1);
    }

    current_row_ += 1;
    if(current_row_ > 2)
       current_row_ = 0;
    addAction(action);

    return button;
}

RibbonButton* RibbonGroup::addMiniAction(QAction* action){

    RibbonButton* button = new RibbonButton(this);
    button->setButtonSize(RibbonButton::kMini);
    button->setAutoRaise(true);
    button->setDefaultAction(action);
    if(action->menu())
        button->setPopupMode(QToolButton::InstantPopup);
    if(0 == current_row_){
        grid_layout_->addWidget(button, current_row_, grid_layout_->columnCount(), 1, 1);
    }else {
        grid_layout_->addWidget(button, current_row_, grid_layout_->columnCount()-1, 1, 1);
    }

    current_row_ += 1;
    if(current_row_ > 2)
        current_row_ = 0;

    return button;
}

void RibbonGroup::addWidget(QWidget* widget, int row, int col, int rowSpan, int columnSpan,
                            Qt::Alignment alignment) {
    widget->setParent(this);
    grid_layout_->addWidget(widget, row, col, rowSpan, columnSpan, alignment);
    current_row_ = row + rowSpan;
    if(row > 2)
        current_row_ = 0;
}

void RibbonGroup::removeWidget(QWidget* widget) {
    grid_layout_->removeWidget(widget);
}

void RibbonGroup::setExpandButtonVisible(bool visible) {
    pop_button_->setVisible(visible);
}

bool RibbonGroup::isExpandButtonVisible() const {
    return pop_button_->isVisible();
}

void RibbonGroup::clear() {
}

} // namespace gui
} // namespace open_edi
