#ifndef ACTION_GROUP_MANAGER_H
#define ACTION_GROUP_MANAGER_H

#include <QList>
#include <QMap>
#include <QObject>

class QActionGroup;
class QAction;

namespace open_edi {
namespace gui {

class ActionGroupManager : public QObject {
    Q_OBJECT

  public:
    explicit ActionGroupManager(QObject* object = nullptr);

    QActionGroup* file_;
    QActionGroup* edit_;
    QActionGroup* view_;
    QActionGroup* small_tool_;
    QActionGroup* report_;
    QActionGroup* tool_;
    QActionGroup* floorplan_;
    QActionGroup* partition_;
    QActionGroup* wire_edit_;
    QActionGroup* wire_operate_;
    QActionGroup* design_view_;
    QActionGroup* ruler_;
    QActionGroup* ruler_shape_;
    QActionGroup* ruler_snap_;
    QActionGroup* other_;

    QList<QActionGroup*>         toolGroups();
    QList<QActionGroup*>         commonGroups();
    QMap<QString, QActionGroup*> getAllGroups();
};

} // namespace gui
} // namespace open_edi
#endif // ACTION_GROUP_MANAGER_H
