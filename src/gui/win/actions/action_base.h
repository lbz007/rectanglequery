#ifndef ACTION_ABSTRACT_H
#define ACTION_ABSTRACT_H

#include <QDebug>
#include <QObject>

namespace open_edi {
namespace gui {

class GraphicsView;

class ActionBase : public QObject {
    Q_OBJECT
  public:
    explicit ActionBase(QString name, GraphicsView& view, QObject* parent = nullptr);

  signals:

  public slots:

  private:
    GraphicsView* view_;
    QString        action_name_;
};

} // namespace gui
} // namespace open_edi

#endif // ACTION_ABSTRACT_H
