#ifndef ACTIONHANDLER_H
#define ACTIONHANDLER_H

#include <QObject>
#include "../actions/action_base.h"
#include "common_def.h"

namespace open_edi {
namespace gui {

class ActionHandler : public QObject {
    Q_OBJECT

  public:
    explicit ActionHandler(QObject* parent = nullptr);
    virtual ~ActionHandler();

    ActionBase* getCurrentAction();
    ActionBase* setCurrentAction(EDAGui::ActionType type);

    void setView(GraphicsView* view);

  signals:
    void sendReadData();
  public slots:
    void slotImportDesign();
    void slotSaveDesign();
    void slotSetPreference();
    void slotFindSelectObject();
    void slotEditUndo();
    void slotEditRedo();
    void slotEditHighLight();
    void slotViewZoomIn();
    void slotViewZoomOut();
    void slotViewZoomFit();
    void slotViewRefresh();

  private:
    GraphicsView* view_;
};

} // namespace gui
} // namespace open_edi
#endif // ACTIONHANDLER_H
