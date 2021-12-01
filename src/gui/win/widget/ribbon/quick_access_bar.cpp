
#include "quick_access_bar.h"
#include <QMouseEvent>
#include "ribbon_button.h"
#include "util/util.h"
#include "../status/nav_button.h"

namespace open_edi {
namespace gui {

QuickAccessBar::QuickAccessBar(QWidget* parent)
  : QToolBar(parent), menu_(nullptr), separ_action_(nullptr) {
    setObjectName("QuickAccessBar");
    init();
}

QuickAccessBar::~QuickAccessBar() {
}

void QuickAccessBar::init() {
    arrow_action_ = new QAction(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/menu.png"), "");
    menu_ = new QMenu;
    arrow_action_->setMenu(menu_);
    connect(arrow_action_, &QAction::triggered, this, &QuickAccessBar::slotArrowButtonClicked);

    NavButton* button = new NavButton(this);
    button->setIcon(arrow_action_->icon());
    button->setMenu(menu_);
    button->setDefaultAction(arrow_action_);
    addWidget(button);
    separ_action_ = menu_->addSeparator();
}

void QuickAccessBar::addAction(QAction* action) {

    QAction* checkAction = new QAction(action->text(), this);
    checkAction->setCheckable(true);
    checkAction->setChecked(true);

    RibbonButton *button = new RibbonButton(this);
    button->setDefaultAction(action);

    menu_->insertAction(separ_action_, checkAction);
//    connect(checkAction, SIGNAL(triggered(bool)), button, SLOT(setvisible(bool)));

    insertWidget(arrow_action_, button);
}

QAction* QuickAccessBar::addAction(const QIcon& icon, const QString& text) {
    QAction* action = new QAction(icon, text);
    addAction(action);
    return action;
}

void QuickAccessBar::setArrowVisible(bool visible) {
    arrow_action_->setVisible(visible);
}

void QuickAccessBar::slotArrowButtonClicked(){
    QAction* action = qobject_cast<QAction*>(sender());
    if(action == arrow_action_){
        menu_->exec(QCursor::pos());
    }
}

} // namespace gui
} // namespace open_edi
