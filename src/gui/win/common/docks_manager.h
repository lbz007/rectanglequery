#ifndef DOCKS_PRODUCER_H
#define DOCKS_PRODUCER_H

#include <QDockWidget>
#include <QObject>
#include <QTextEdit>

#include "../widget/panel/components_widget.h"
#include "../widget/panel/layer_widget.h"
#include "../widget/panel/world_view.h"

#include "../layout/graphics_view.h"

namespace open_edi {
namespace gui {

class MainWindow;
class ActionHandler;
class DocksManager : public QObject {
    Q_OBJECT
  public:
    explicit DocksManager(QObject* parent = nullptr);

    void createDockWidgets();

    LayerWidget*      layer_widget{nullptr};
    ComponentsWidget* component_widget{nullptr};
    WorldView*        world_view{nullptr};

  signals:

  public slots:

  private:
    MainWindow* main_window_;
};

} // namespace gui
} // namespace open_edi

#endif // DOCKS_PRODUCER_H
