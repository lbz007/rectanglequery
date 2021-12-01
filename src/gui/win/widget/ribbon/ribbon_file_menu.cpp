#include "ribbon_file_menu.h"
#include <QLayout>
#include "ribbon_button.h"
//#include <QLayoutItem>

namespace open_edi {
namespace gui {
RibbonFileMenu::RibbonFileMenu(QWidget* parent)
  : QMenu(parent) {
}

RibbonFileMenu::~RibbonFileMenu() {
}

void RibbonFileMenu::addFileAction(QAction* action, Qt::ToolButtonStyle style) {
    if (style == Qt::ToolButtonTextUnderIcon) {
        return;
    }
    /*
    RibbonButton* button = new RibbonButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(style);
    button->setText(action->text());
    button->setDefaultAction(action);
*/
    addAction(action);
    //    QLayoutItem* item = layout()->itemAt(layout()->count() -1);
    //    if(QToolButton* button = qobject_cast<QToolButton*>(item? item->widget() : nullptr))
    //    {
    //        button->setToolButtonStyle(style);
    //    }
    //    file_buttons_.append(button);
}

QAction* RibbonFileMenu::addFileAction(const QString& text) {
    /*
    RibbonButton* button = new RibbonButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    button->setText(text);
    file_buttons_.append(button);
*/
    QAction* action = new QAction(text);
    addAction(action);

    return action;
}

} // namespace gui
} // namespace open_edi
