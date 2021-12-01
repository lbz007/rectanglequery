#ifndef EDI_GUI_RIBBON_FILE_MENU_H_
#define EDI_GUI_RIBBON_FILE_MENU_H_

#include <QMenu>

namespace open_edi {
namespace gui {

class RibbonButton;

class RibbonFileMenu : public QMenu {
    Q_OBJECT

  public:
    explicit RibbonFileMenu(QWidget* parent = nullptr);
    ~RibbonFileMenu();

  public:
    void     addFileAction(QAction* action, Qt::ToolButtonStyle style = Qt::ToolButtonTextOnly);
    QAction* addFileAction(const QString& text);

    void addRecentFile(const QString& recent_file);

  private:
    QList<RibbonButton*> file_buttons_;
};

} // namespace gui
} // namespace open_edi

#endif // RIBBON_FILE_MENU_H
