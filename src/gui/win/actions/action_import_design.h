#ifndef ACTIONFILEIMPORTDESIGN_H
#define ACTIONFILEIMPORTDESIGN_H

#include "action_base.h"

namespace open_edi {
namespace gui {

class ActionFileImportDesign : public ActionBase {
    Q_OBJECT
  public:
    ActionFileImportDesign(GraphicsView& view, QObject* parent = nullptr);
    virtual ~ActionFileImportDesign();

  signals:

  public slots:
};

} // namespace gui
} // namespace open_edi

#endif // ACTIONFILEIMPORTDESIGN_H
