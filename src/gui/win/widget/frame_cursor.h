#ifndef EDI_GUI_FRAME_CURSOR_H_
#define EDI_GUI_FRAME_CURSOR_H_

#include <QPoint>
#include <QRect>

namespace open_edi {
namespace gui {

class FrameCursor {
  public:
    FrameCursor();
    void reset();
    void recalculate(const QPoint& mouse_pos, const QRect& frame_rect);

  public:
    bool on_edges_{true};
    bool on_left_edge_{true};
    bool on_right_edge_{true};
    bool on_top_edge_{true};
    bool on_bottom_edge_{true};
    bool on_top_left_edge_{true};
    bool on_bottom_left_edge_{true};
    bool on_top_right_edge_{true};
    bool on_bottom_right_edge_{true};

    int border_width_{5};
};

} // namespace gui
} // namespace open_edi

#endif // FRAME_CURSOR_H
