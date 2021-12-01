#include "status_bar.h"
#include "nav_button.h"
#include "util/util.h"

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QTextBrowser>

namespace open_edi {
namespace gui {

StatusBar::StatusBar(QWidget* parent)
  : QStatusBar(parent) {
    setObjectName("StatusBar");
    init();
    //   setMaximumHeight(25);
}

StatusBar::~StatusBar() {
}

void StatusBar::init() {

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(5);
    setContentsMargins(0, 0, 0, 0);
    //   layout()-> setLayout(layout);

    item_tar_ = new QToolBar(this);
    item_tar_->setOrientation(Qt::Horizontal);
    menu_action = new QAction(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/menu.png"), tr(""));
    connect(menu_action, &QAction::triggered, this, &StatusBar::slotMenuButtonClicked);
    item_tar_->addAction(menu_action);

    slide = new QSlider(Qt::Horizontal, this);
    slide->setToolTip(tr("Precision/Speed"));
    slide->setMaximum(0);
    slide->setMaximum(255);
    slide->setFixedWidth(160);
    slide->setFixedHeight(18);
    mouse_location_ = new QLabel(this);
    addPermanentWidget(mouse_location_);
    layout->addWidget(slide);
    layout->addWidget(item_tar_);
    layout->addStretch();
    //    layout->addWidget(mouse_location_);

    //    slide->setVisible(false);
    item_tar_->setVisible(false);

    panel_stack_widget_ = new QStackedWidget;
    toggle_menu_        = new QMenu(this);
}

void StatusBar::createNavigations() {
    QTextBrowser* log_widget    = new QTextBrowser;
    QTextBrowser* search_widget = new QTextBrowser;
    addNavigation(tr("Log"), log_widget);
    addNavigation(tr("Search"), search_widget);
}

void StatusBar::addNavigation(const QString& title, QWidget* widget) {
    panel_stack_widget_->addWidget(widget);

    NavButton* button = new NavButton();
    button->setText(title);
    button->setCheckable(true);
    button->setToolTip(title);
    navi_map_.insert(button, widget);
    connect(button, &NavButton::toggled, this, &StatusBar::slotTabItemClicked);

    QAction* toggle_action = new QAction(title, this);
    toggle_action->setCheckable(true);
    toggle_action->setChecked(true);
    connect(toggle_action, &QAction::toggled, this, &StatusBar::slotToggleNavigation);
    toggle_menu_->addAction(toggle_action);
    toggle_map_.insert(toggle_action, button);
    item_tar_->insertWidget(menu_action, button);
}

void StatusBar::slotMenuButtonClicked() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action == menu_action) {
        toggle_menu_->exec(QCursor::pos());
    }
}

void StatusBar::slotTabItemClicked(bool clicked) {
    NavButton* button = qobject_cast<NavButton*>(sender());
    if (!current_selected_) {
        current_selected_ = button;
    } else if (current_selected_ != button) {
        current_selected_->select(false);
        current_selected_->setChecked(false);
        current_selected_ = button;
    }
    QWidget* widget = navi_map_.value(button);
    if (widget) {
        panel_stack_widget_->setCurrentWidget(widget);
        emit signalTilteChanged(clicked);
    }
}

void StatusBar::slotToggleNavigation(bool toggled) {
    QAction*   action = qobject_cast<QAction*>(sender());
    NavButton* button = toggle_map_.value(action);
    if (button) {
        qDebug() << "button->text: " << button->text();
        button->setVisible(toggled);
    }
}

void StatusBar::slotGetMouseLocation(double x, double y) {
    QString str;
    str = QString::number(x, 'g', 10) + "," + QString::number(y, 'g', 10);
    mouse_location_->setText(str);
}

} // namespace gui
} // namespace open_edi
