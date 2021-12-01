#ifndef ACTIONPRODUCER_H
#define ACTIONPRODUCER_H

#include <QMap>
#include <QObject>

class QAction;
class QActionGroup;

namespace open_edi {
namespace gui {

class ActionGroupManager;

class ActionProducer : public QObject {
    Q_OBJECT
  public:
    explicit ActionProducer(QObject* parent = nullptr, QObject* hander = nullptr);
    void fillActionContainer(QMap<QString, QAction*>& map, ActionGroupManager* manager);
    void addOtherAction(QMap<QString, QAction*>& map, ActionGroupManager* manager);

  private:
    QObject* action_hander_;
    QObject* main_window_;
};

} // namespace gui
} // namespace open_edi
#endif // ACTIONPRODUCER_H
