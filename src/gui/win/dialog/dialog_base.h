#ifndef EDI_GUI_DIALOG_BASE_H_
#define EDI_GUI_DIALOG_BASE_H_

#include <QDialog>

namespace open_edi {
namespace gui {

class TitleBar;

class DialogTitle : public QWidget {
    Q_OBJECT

  public:
    DialogTitle(QWidget* parent = nullptr);
    ~DialogTitle();

    void setWindowTitle(const QString& title) const;

  protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);

  private:
    TitleBar* title_bar_;
};

class DialogBase : public QDialog {
    Q_OBJECT
  public:
    explicit DialogBase(QWidget* parent = nullptr);
    ~DialogBase();

    void setCentralWidget(QWidget* window);
    void setDialogTitle(const QString& title) const;

  private:
    DialogTitle* title_bar_;
    QWidget*     central_widget_;
};

} // namespace gui
} // namespace open_edi

#endif // DIALOG_BASE_H
