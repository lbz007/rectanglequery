#ifndef EDI_GUI_DRAW_HELPER_H
#define EDI_GUI_DRAW_HELPER_H

#include <QIcon>
#include <QStylePainter>
#include <QStyleOption>
#include <QPixmap>

namespace open_edi {
namespace gui {

class DrawHelper
{
public:
    DrawHelper();
    static QPixmap iconToPixmap(const QIcon &icon, QWidget* widget, const QStyleOption *opt, const QSize &icoSize);
    static void drawIcon(const QIcon &icon, QPainter *painter, const QStyleOption *opt
                         , int x, int y, int width, int height);
    static void drawIcon(const QIcon &icon, QPainter *painter, const QStyleOption *opt
                         , const QRect& rect);
    static QSize iconActualSize(const QIcon &icon, const QStyleOption* opt, const QSize& iconSize);

    static void drawText(const QString& text, QStylePainter *painter, const QStyleOption *opt
                         , Qt::Alignment al, int x, int y, int width, int height);
    static void drawText(const QString& text, QStylePainter *painter, const QStyleOption *opt
                         , Qt::Alignment al, const QRect& rect);
};

}
}

#endif // DRAW_HELPER_H
