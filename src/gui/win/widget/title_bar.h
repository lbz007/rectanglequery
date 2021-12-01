#ifndef EDI_GUI_TITLE_BAR_H_
#define EDI_GUI_TITLE_BAR_H_

#include <QLabel>
#include <QToolButton>
#include <QWidget>

#include "frame_cursor.h"

namespace open_edi {
namespace gui {

class TitleBar : public QObject {
    Q_OBJECT

  public:
    explicit TitleBar(QObject* parent = nullptr);
    ~TitleBar() override;
    void init();

    void     setWindowTitle(const QString& title);
    void     setTitleBarIcon(const QString& icon);
    void     addQuickAction(QAction* action);
    QAction* addQuickAction(const QIcon& icon, const QString& text);
    QToolButton* getCloseButton();

    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    bool eventFilter(QObject* obj, QEvent* e) override;

  private:
    void updateMaximize();
    void handleMouseEvent(QObject* obj, QEvent* e);
    void handleMousePressEvent(QMouseEvent* e);
    void handleMouseRelaseEvent(QMouseEvent* e);
    void handleMouseMoveEvent(QMouseEvent* e);
    void handleHoverMoveEvent(QHoverEvent* e);
    void handleLeaveEvent(QMouseEvent* e);

    void   resizeWidget(const QPoint& mouse_pos);
    void   updateCursorShape(const QPoint& mouse_pos);
    QPoint calcStartPoint(QWidget* pWindow, QMouseEvent* e) const;

  signals:
  public slots:
    void slotSystemClicked();

  public:
    QLabel*  title_;
    QWidget* system_group_;

  private:
    QWidget* main_window_;

    QLabel*      icon_;
    QToolButton* minimize_bt_;
    QToolButton* maximize_bt_;
    QToolButton* close_bt_;
    QRect        normal_rect_;

    bool        left_button_pressed;
    bool        cursor_shape_changed_;
    bool        edge_pressed_;
    bool        maximized_;
    bool        minimized_;
    FrameCursor press_cursor_;
    FrameCursor move_cursor_;
    QPoint      move_point_;
};

} // namespace gui
} // namespace open_edi
#endif // RIBBON_TITLE_BAR_H
