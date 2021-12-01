#ifndef OPEN_EDI_LAYOUT_GRAPHICS_SCENE_H_
#define OPEN_EDI_LAYOUT_GRAPHICS_SCENE_H_

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPoint>

namespace open_edi {
namespace gui {

class GraphicsScene : public QGraphicsScene {
    Q_OBJECT
  public:
    explicit GraphicsScene(QObject* parent = nullptr);
    ~GraphicsScene();

    void setSelectionRect(QPointF p1, QPointF p2);

  protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

  private:
    QGraphicsRectItem* selectionRectItem_;

    // signals:
    //   void sendPos(QPointF point);
};

} // namespace gui
} // namespace open_edi

#endif // GRAPHICSSCENCE_H
