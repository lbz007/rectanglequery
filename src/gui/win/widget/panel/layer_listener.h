#ifndef EDI_GUI_LAYER_LISTENER_H_
#define EDI_GUI_LAYER_LISTENER_H_

#include <QList>
#include <QString>
#include <QColor>

namespace open_edi {
namespace gui {

class LayerListener {
  public:
    LayerListener();
    virtual ~LayerListener(){};

    virtual void            setLayerVisible(QString name, bool v)     = 0;
    virtual void            setLayerSelectable(QString name, bool v)  = 0;
    virtual void            setLayerColor(QString name, QColor color) = 0;
    virtual void            layerUpdate()                             = 0;
    virtual QList<QString>* refreshLayerTree();
};

} // namespace gui
} // namespace open_edi

#endif // LAYER_LISTENER_H
