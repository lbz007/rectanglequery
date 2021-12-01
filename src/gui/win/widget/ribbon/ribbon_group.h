#ifndef RIBBON_GROUP_H
#define RIBBON_GROUP_H

#include <QActionGroup>
#include <QBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QWidget>

namespace open_edi {
namespace gui {

class RibbonButton;

class RibbonGroup : public QWidget {
    Q_OBJECT
  public:
    explicit RibbonGroup(QString title, QWidget* parent = nullptr);
    ~RibbonGroup();

    void init();

    void setTitle(const QString title);

    RibbonButton* addLargeAction(QAction* action);
    RibbonButton* addSmallAction(QAction* action);
    RibbonButton* addMiniAction(QAction* action);

    void addWidget(QWidget* widget, int row, int col, int rowSpan = 1, int columnSpan = 1,
                   Qt::Alignment alignment = Qt::Alignment());
    void removeWidget(QWidget* widget);
    void clear();

    QMenu* addMenu(const QIcon& icon, const QString& text, Qt::ToolButtonStyle style = Qt::ToolButtonFollowStyle);

    QActionGroup* getActionGroup() const;

    void setExpandButtonVisible(bool visible = true);
    bool isExpandButtonVisible() const;

  protected:
  signals:
    void popButtonClicked();

  public slots:

  private:
    QToolBar*     large_bar_;
    QToolBar*     small_bar_;
    QHBoxLayout*  group_layout_;
    QGridLayout*  grid_layout_;
    QLabel*       group_title_;
    RibbonButton* pop_button_;
    int           current_row_{0};
};

} // namespace gui
} // namespace open_edi
#endif // RIBBON_GROUP_H
