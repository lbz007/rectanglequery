#include "dialog_base.h"

#include <QHBoxLayout>
#include <QMouseEvent>

#include "../widget/title_bar.h"
#include "util/util.h"

namespace open_edi {
namespace gui {

DialogTitle::DialogTitle(QWidget* parent)
  : QWidget(parent) {
    setObjectName("DialogTitle");
    title_bar_ = new TitleBar(this);

    QToolButton* close_bt = new QToolButton(this);
    close_bt->setToolTip(tr("Close"));
    close_bt->setIcon(QIcon(QString::fromStdString(open_edi::util::getResourcePath()) + "tool/close.png"));
    connect(close_bt, &QToolButton::clicked, parent, &QWidget::close);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(title_bar_->title_);
    layout->addStretch(1);
    layout->addWidget(close_bt);
    setLayout(layout);
}

DialogTitle::~DialogTitle() {
}

void DialogTitle::setWindowTitle(const QString& title) const {
    title_bar_->setWindowTitle(title);
}

void DialogTitle::mousePressEvent(QMouseEvent* e) {
    title_bar_->mousePressEvent(e);
    e->ignore();
}

void DialogTitle::mouseReleaseEvent(QMouseEvent* e) {
    title_bar_->mouseReleaseEvent(e);
    e->ignore();
}

void DialogTitle::mouseMoveEvent(QMouseEvent* e) {
    title_bar_->mouseMoveEvent(e);
}

void DialogTitle::mouseDoubleClickEvent(QMouseEvent* e) {
    title_bar_->mouseDoubleClickEvent(e);
}

DialogBase::DialogBase(QWidget* parent)
  : QDialog(parent) {
    setObjectName("DialogBase");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::Dialog);
    title_bar_ = new DialogTitle(this);
    title_bar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(1, 0, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(title_bar_);
    setLayout(layout);
}

DialogBase::~DialogBase() {
}

void DialogBase::setCentralWidget(QWidget* window) {
    if (!window || central_widget_ == window) {
        return;
    }
    central_widget_ = window;
    layout()->addWidget(central_widget_);
}

void DialogBase::setDialogTitle(const QString& title) const {
    title_bar_->setWindowTitle(title);
}

} // namespace gui
} // namespace open_edi
