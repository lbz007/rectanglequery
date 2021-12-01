#include "ribbon_menu_bar.h"

#include <QDebug>
#include <QLabel>
#include <QFile>
#include <QApplication>

#include "ribbon_file_menu.h"
#include "stacked_widget.h"
#include "ribbon_button.h"
#include "application_button.h"
#include "util/util.h"
#include "ribbon_page.h"
#include "ribbon_title_bar.h"
#include "ribbon_group.h"
#include "quick_access_bar.h"

namespace open_edi {
namespace gui {

RibbonMenuBar::RibbonMenuBar(QWidget *parent)
    : QMenuBar (parent)
    , title_bar_(new RibbonTitleBar(parent))
{
    setObjectName("RibbonMenuBar");
    init();
}

RibbonMenuBar::~RibbonMenuBar()
{

}

void RibbonMenuBar::init()
{
    title_bar_->setObjectName("RibbonTitleBar");

    file_button_ = new ApplicationButton(tr("File"), this);
    RibbonFileMenu* menu = new RibbonFileMenu(file_button_);
    file_button_->setMenu(menu);

    tab_bar_ = new QTabBar(this);
    tab_bar_->setObjectName("RibbonMenuTabBar");
//    tab_bar_->setFixedHeight(32);
    connect(tab_bar_, &QTabBar::tabBarDoubleClicked, this, &RibbonMenuBar::slotTabBarDoubleClicked);
    connect(tab_bar_, &QTabBar::currentChanged, this, &RibbonMenuBar::slotTabBarChanged);
    connect(tab_bar_, &QTabBar::tabBarClicked, this, &RibbonMenuBar::slotTabBarClicked);

    stacked_widget_ = new StackedWidget(this);
    stacked_widget_->setObjectName("RibbonMenuStackedWidget");
    stacked_widget_->installEventFilter(this);
    connect(stacked_widget_, &StackedWidget::hideWindow, this, &RibbonMenuBar::slotStackedWidgetHided);

    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->addWidget(file_button_);
    tabLayout->addWidget(tab_bar_);
    tabLayout->addStretch();

    QLabel* logo = new QLabel(this);
    logo->setPixmap(QPixmap(QString::fromStdString(open_edi::util::getResourcePath())+"tool/logo.png"));
    tabLayout->addWidget(logo);

    RibbonButton* show_hide = new RibbonButton(this);
    show_hide->setFixedSize(27,22);
    show_hide->setObjectName("ShowHideButton");
    show_hide->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/show.png"));
    tabLayout->addWidget(show_hide);
    connect(show_hide, &QToolButton::clicked, this, &RibbonMenuBar::slotPageShowHide);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addWidget(title_bar_);
    vLayout->addLayout(tabLayout);
    vLayout->addWidget(stacked_widget_);

    setLayout(vLayout);
}

void RibbonMenuBar::fillActions(QMap<QString, QAction*> map){
    map_ = map;

    RibbonPage*  page  = addPage(tr("Common"));
    RibbonGroup* group = page->addGroup(tr("view"));

    group->addLargeAction(map_["Select"]);
    group->addMiniAction(map_["ZoomIn"]);
    group->addMiniAction(map_["ZoomOut"]);
    group->addMiniAction(map_["ZoomFit"]);
    group->addMiniAction(map_["Refresh"]);

    group = page->addGroup(tr("window"));
    group->addSmallAction(map_["Window"]);
    group->addSmallAction(new QAction());
    group->addSmallAction(new QAction());

    title_bar_->setWindowTitle(tr("OpenEDA"));
    QAction* action = new QAction(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/skin.png"), tr("Skin"));
    QMenu* menu = new QMenu;
    QAction* theme_default = menu->addAction(tr("Default"));
    QAction* theme_light = menu->addAction(tr("Light"));
    action->setMenu(menu);
    title_bar_->addOtherControl(action);
    connect(theme_default, &QAction::triggered, this, &RibbonMenuBar::slotThemeChanged);
    connect(theme_light, &QAction::triggered, this, &RibbonMenuBar::slotThemeChanged);
}

RibbonPage* RibbonMenuBar::addPage(const QString &text)
{
    RibbonPage* page = new RibbonPage(this);
    page->setWindowTitle(text);

    int index = tab_bar_->addTab(text);
    tab_bar_->setTabData(index, QVariant((quint64)page));
    stacked_widget_->addWidget(page);
    connect(page, &QWidget::windowTitleChanged, this, &RibbonMenuBar::slotPageTitleChanged);

    return page;
}

int RibbonMenuBar::pageCount() const{
    return tab_bar_->count();
}

void RibbonMenuBar::setHideMode(bool hide){
    if(hide){
         stacked_widget_->setPopupMode();
         stacked_widget_->setFocusPolicy(Qt::NoFocus);
         stacked_widget_->clearFocus();
         tab_bar_->setFocus();
         stacked_widget_->hide();
         setFixedHeight(tab_bar_->geometry().bottom());
    }else {
         stacked_widget_->setNormalMode();
         stacked_widget_->setFocus();
         stacked_widget_->show();
         setFixedHeight(height_);
    }
    update();
}

bool RibbonMenuBar::isRibbonHideMode() const{
    return stacked_widget_->isPopupMode();
}

void RibbonMenuBar::slotThemeChanged(){
    QAction* action = qobject_cast<QAction*>(sender());
    QString theme = QString::fromStdString(open_edi::util::getResourcePath())
            + "qss/" + action->text().toLower() + ".qss";
    QFile theme_file(theme);
    if (!theme_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    qApp->setStyleSheet(theme_file.readAll());
    update();
}

void RibbonMenuBar::slotStackedWidgetHided(){
    tab_bar_->setCurrentIndex(-1);
}

void RibbonMenuBar::slotPageTitleChanged(const QString &text){
   QWidget *w = qobject_cast<QWidget*>(sender());

   for(int i = 0; i < tab_bar_->count(); i++){
       QVariant var = tab_bar_->tabData(i);
       QWidget *page = nullptr;
       if(var.isValid()){
           page = (QWidget*)(var.value<quint64>());
           if(page == w){
               tab_bar_->setTabText(i, text);
           }
       }
   }
}

void RibbonMenuBar::slotTabBarDoubleClicked(int index){
    Q_UNUSED(index)
    setHideMode(!isRibbonHideMode());
}

void RibbonMenuBar::slotTabBarClicked(int index){
    if(isRibbonHideMode()){
        if(!stacked_widget_->isVisible()){
            if(stacked_widget_->isPopupMode()){
                tab_bar_->setCurrentIndex(index);
            }
        }
    }
}

void RibbonMenuBar::slotTabBarChanged(int index){
    QVariant var = tab_bar_->tabData(index);
    RibbonPage *page = nullptr;

    if(var.isValid()){
        page = (RibbonPage*)(var.value<quint64>());
    }
    if(page){
        if(stacked_widget_->currentWidget() != page){
            stacked_widget_->setCurrentWidget(page);
        }
        if(isRibbonHideMode()){
            if(!stacked_widget_->isVisible()){
                if(stacked_widget_->isPopupMode()){
                    QPoint pos(mapToGlobal(QPoint(tab_bar_->rect().left(), tab_bar_->rect().bottom()+1)));
                    stacked_widget_->exec();
                    stacked_widget_->setGeometry(pos.x(), pos.y()+title_bar_->height(), width(), stacked_widget_->height());
                }
            }
        }
    }
    emit currentTabChanged(index);
}

void RibbonMenuBar::slotPageShowHide(bool show){
    Q_UNUSED(show)

//    setHideMode(show);
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if(isRibbonHideMode()){
        button->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/show.png"));
        button->setToolTip(tr("Show"));
        setHideMode(false);
    }else {
        button->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/hide.png"));
        button->setToolTip(tr("Hide"));
        setHideMode(true);
    }
}

QPushButton* RibbonMenuBar::getFileButton() const{
    return file_button_;
}

void RibbonMenuBar::setRibbonHeight(int height){
    height_ = height;
    setFixedHeight(height_);
}

}
}
