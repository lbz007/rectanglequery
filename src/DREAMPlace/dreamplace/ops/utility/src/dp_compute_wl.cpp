/* @file: dp_compute_wl.cpp
 * @date: <date>
 * @brief: for int64_t version compute_hpwl. this can be replaced later by common hpwl report func. after correlation.
 */

#include "utility/src/DetailedPlaceDB.h"

DREAMPLACE_BEGIN_NAMESPACE
template<>
int64_t DetailedPlaceDB<int>::compute_net_hpwl_edi(int net_id) const
{
        UBox<int> box (
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max(),
                -std::numeric_limits<int>::max(),
                -std::numeric_limits<int>::max()
                );
        for (int net2pin_id = flat_net2pin_start_map[net_id]; net2pin_id < flat_net2pin_start_map[net_id+1]; ++net2pin_id)
        {
            int net_pin_id = flat_net2pin_map[net2pin_id];
            int other_node_id = pin2node_map[net_pin_id];
            box.xl = std::min(box.xl, x[other_node_id]+pin_offset_x[net_pin_id]);
            box.xh = std::max(box.xh, x[other_node_id]+pin_offset_x[net_pin_id]);
            box.yl = std::min(box.yl, y[other_node_id]+pin_offset_y[net_pin_id]);
            box.yh = std::max(box.yh, y[other_node_id]+pin_offset_y[net_pin_id]);
        }
        if (box.xl == std::numeric_limits<int>::max() || box.yl == std::numeric_limits<int>::max())
        {
            return 0;
        }
        return static_cast<int64_t>((box.xh-box.xl) + (box.yh-box.yl));
}

template<>
int64_t DetailedPlaceDB<int>::compute_total_hpwl_edi() const
{
        int64_t total_hpwl = 0;
        for (int net_id = 0; net_id < num_nets; ++net_id)
        {
            {
                total_hpwl += compute_net_hpwl_edi(net_id);
            }
        }
        return total_hpwl/site_width;
}

DREAMPLACE_END_NAMESPACE
