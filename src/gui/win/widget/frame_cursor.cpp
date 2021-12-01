#include "frame_cursor.h"

namespace open_edi {
namespace gui {

FrameCursor::FrameCursor() {
    reset();
}

void FrameCursor::reset() {
    on_edges_             = false;
    on_left_edge_         = false;
    on_right_edge_        = false;
    on_top_edge_          = false;
    on_bottom_edge_       = false;
    on_top_left_edge_     = false;
    on_bottom_left_edge_  = false;
    on_top_right_edge_    = false;
    on_bottom_right_edge_ = false;
}

void FrameCursor::recalculate(const QPoint& mouse_pos, const QRect& frame_rect) {

    int mouse_x      = mouse_pos.x();
    int mouse_y      = mouse_pos.y();
    int frame_x      = frame_rect.x();
    int frame_y      = frame_rect.y();
    int frame_width  = frame_rect.width();
    int frame_height = frame_rect.height();

    on_left_edge_         = (mouse_x >= frame_x) && (mouse_x <= frame_x + border_width_);
    on_right_edge_        = (mouse_x >= frame_x + frame_width - border_width_) && (mouse_x <= frame_x + frame_width);
    on_top_edge_          = (mouse_y >= frame_y) && (mouse_y <= frame_y + border_width_);
    on_bottom_edge_       = (mouse_y >= frame_y + frame_height - border_width_) && (mouse_y <= frame_y + frame_height);
    on_top_left_edge_     = on_left_edge_ && on_top_edge_;
    on_bottom_left_edge_  = on_bottom_edge_ && on_left_edge_;
    on_top_right_edge_    = on_top_edge_ && on_right_edge_;
    on_bottom_right_edge_ = on_bottom_edge_ && on_right_edge_;
    on_edges_             = on_left_edge_ || on_right_edge_ || on_top_edge_ || on_bottom_edge_;
}

} // namespace gui
} // namespace open_edi
