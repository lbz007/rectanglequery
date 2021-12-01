#ifndef EDI_GUI_PALETTE_H_
#define EDI_GUI_PALETTE_H_

#include <QBrush>
#include <QButtonGroup>
#include <QColorDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QList>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSpacerItem>
#include <QTreeWidgetItem>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace open_edi {
namespace gui {

class PushButton : public QPushButton {
    Q_OBJECT
  public:
    PushButton(QWidget* parent = nullptr);
    virtual void paintEvent(QPaintEvent* e) override;
    void         setBrush(QBrush&& brush);
    QBrush       getBrush() { return brush_; };

  signals:
    void revClicked(PushButton*);

  private:
    QBrush brush_;

  private slots:
    void slotClicked();
};

class Palette : public QDialog {
    Q_OBJECT
  public:
    Palette(QList<QBrush>&& brush_list, bool color_borad = false, QWidget* parent = nullptr);
    ~Palette();
    QBrush getBrush();
    void   setItemBackground(QBrush brush);
    void   getItemName(const char* name);
    void   setName(QString name);

    PushButton* color_btn{nullptr};
  signals:

    void signalBtnOKClicked(const char*, QBrush brush);

  private:
    QLabel*        name_label_;
    QBrush         brush_;
    QBrush         old_brush_;
    QBrush         item_brush_;
    QColor         color_;
    Qt::BrushStyle style_;
    int            column_;
    QList<QBrush>  brush_list_;
    char           item_name_[0xff]{0};
    bool           color_borad_;
  private slots:
    void slotColorTriggered(PushButton* bt);
    void slotOKClicked();
    void slotCancelClicked();
    void slotButtonToggled(QAbstractButton* button, bool checked);
    void slotColorDialog(PushButton* bt);
};

} // namespace gui
} // namespace open_edi

#endif