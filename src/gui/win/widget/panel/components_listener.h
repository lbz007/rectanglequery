#ifndef EDI_GUI_COMPONENTS_LISTENER_H_
#define EDI_GUI_COMPONENTS_LISTENER_H_

#include <QString>
namespace open_edi {
namespace gui {

class ComponentListener {
  public:
    ComponentListener();
    virtual ~ComponentListener() {}

    virtual void setComponentVisible(QString name, bool v)    = 0;
    virtual void setComponentSelectable(QString name, bool v) = 0;
    virtual void setComponentBrushStyle(QString name, Qt::BrushStyle brush_style) = 0;
    virtual void componentUpdate() = 0;
    
};

} // namespace gui
} // namespace open_edi

#endif // LAYER_LISTENER_H
