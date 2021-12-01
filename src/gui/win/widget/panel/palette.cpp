#include "palette.h"

namespace open_edi {
namespace gui {

constexpr int win_size = 26;

PushButton::PushButton(QWidget* parent) : QPushButton(parent) {
    connect(this, &PushButton::clicked, this, &PushButton::slotClicked);
}

void PushButton::paintEvent(QPaintEvent* e) {

    QPainter painter(this);
    auto     rect = e->rect();
    painter.setBrush(brush_);
    painter.fillRect(rect, brush_);
    QPen pen;
    if (!isChecked()) {
        pen.setColor(QColor("#8f8f91"));
    } else {
        pen.setColor(QColor("#000000"));
    }
    pen.setWidth(5);
    painter.setPen(pen);
    rect.setX(rect.x() - 1);
    rect.setY(rect.y() - 1);
    painter.drawRect(rect);

    // QPushButton::paintEvent(e);
}

void PushButton::setBrush(QBrush&& brush) {
    brush_ = brush;
}

void PushButton::slotClicked() {
    emit revClicked(this);
}

Palette::Palette(QList<QBrush>&& brush_list, bool color_borad, QWidget* parent) : QDialog(parent) {

    setWindowModality(Qt::ApplicationModal);

    name_label_  = new QLabel(this);
    color_borad_ = color_borad;

    PushButton* push_button;
    auto        horizontal_layout1 = new QHBoxLayout();

    horizontal_layout1->addWidget(name_label_);

    auto btn_group = new QButtonGroup(this);

    for (auto brush : brush_list) {
        push_button = new PushButton(this);
        push_button->setText("");
        push_button->setFixedSize(win_size, win_size);
        push_button->setBrush(std::move(brush));
        push_button->setCheckable(true);
        horizontal_layout1->addWidget(push_button);
        btn_group->addButton(push_button);
        connect(push_button, &PushButton::revClicked, this, &Palette::slotColorTriggered);
    }

    QHBoxLayout* horizontal_layout2 = nullptr;
    if (color_borad) {
        horizontal_layout2 = new QHBoxLayout();
        color_btn          = new PushButton(this);
        color_btn->setText("");
        color_btn->setFixedSize(win_size + 3, win_size + 3);
        color_btn->setBrush(QBrush("Gray"));
        color_btn->setCheckable(true);
        btn_group->addButton(color_btn);
        connect(color_btn, &PushButton::revClicked, this, &Palette::slotColorDialog);
        horizontal_layout2->addLayout(horizontal_layout1);
        horizontal_layout2->addWidget(color_btn);
    } else {
        horizontal_layout2 = horizontal_layout1;
    }

    connect(btn_group, static_cast<void (QButtonGroup::*)(QAbstractButton*, bool)>(&QButtonGroup::buttonToggled), this, &Palette::slotButtonToggled);

    QPushButton* btn_ok     = new QPushButton("OK", this);
    QPushButton* btn_cancel = new QPushButton("Cancel", this);

    connect(btn_ok, &QPushButton::clicked, this, &Palette::slotOKClicked);
    connect(btn_cancel, &QPushButton::clicked, this, &Palette::slotCancelClicked);

    auto horizontal_layout3 = new QHBoxLayout();
    horizontal_layout3->addItem(new QSpacerItem(btn_ok->width(),
                                                btn_ok->height(),
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum));
    horizontal_layout3->addWidget(btn_ok);
    horizontal_layout3->addItem(new QSpacerItem(btn_ok->width(),
                                                btn_ok->height(),
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum));
    horizontal_layout3->addWidget(btn_cancel);
    horizontal_layout3->addItem(new QSpacerItem(btn_ok->width(),
                                                btn_ok->height(),
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum));

    auto vertical_layout = new QVBoxLayout(this);

    vertical_layout->addLayout(horizontal_layout2);
    vertical_layout->addLayout(horizontal_layout3);

    setLayout(vertical_layout);
} // namespace gui

Palette::~Palette() {
}

QBrush Palette::getBrush() {
    return brush_;
}

void Palette::setItemBackground(QBrush brush) {
    item_brush_ = brush;
}

void Palette::getItemName(const char* name) {
    memset(item_name_, 0, sizeof(item_name_));
    memcpy(item_name_, name, strlen(name));
    if (color_borad_) {
        color_btn->setBrush(QBrush("Gray"));
    }
}

void Palette::setName(QString name) {
    name_label_->setText(name);
}

void Palette::slotColorTriggered(PushButton* bt) {
    brush_ = bt->getBrush();
}

void Palette::slotOKClicked() {
    emit signalBtnOKClicked(item_name_, getBrush());
    hide();
}

void Palette::slotCancelClicked() {
    hide();
}

void Palette::slotButtonToggled(QAbstractButton* button, bool checked) {
}

void Palette::slotColorDialog(PushButton* bt) {

    QColorDialog color_dlg(this);
    color_dlg.setOption(QColorDialog::DontUseNativeDialog);
    if (color_dlg.exec() == QDialog::Accepted) {
        auto color = color_dlg.currentColor();
        bt->setBrush(color);
        bt->update();
        slotColorTriggered(bt);
    }
}

} // namespace gui
} // namespace open_edi