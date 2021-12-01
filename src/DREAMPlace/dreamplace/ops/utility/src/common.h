/**
 * @file common.h
 * @date
 */
#ifndef _DREAMPLACE_UTILITY_COMMON_H
#define _DREAMPLACE_UTILITY_COMMON_H

#include "utility/src/Msg.h"

DREAMPLACE_BEGIN_NAMESPACE

/**
 * @brief remove 'small' fixed cells that are inside another
 * @param nodes_in_row, sorted by left edge, with small fixed cells removed when return.
 * @pre nodes_in_row should have been sorted by left edge.
 * @post afterwards, sort by center or by left edge will have same order.
 */
template <typename T>
void removeContainedFixedCellsFromRow(
     std::vector<int>& nodes_in_row,
     const int num_movable_nodes,
     const T* x,
     const T* node_size_x
)
{
    if (nodes_in_row.empty())
    {
        return;
    }
    std::vector<int> tmp_nodes; 
    tmp_nodes.reserve(nodes_in_row.size());
    tmp_nodes.push_back(nodes_in_row.front()); 
    int node_id1 = nodes_in_row.at(0);
    for (int j = 1, je = nodes_in_row.size(); j < je; ++j)
    {
        int node_id2 = nodes_in_row.at(j);
        // two fixed cells 
        if (node_id1 >= num_movable_nodes && node_id2 >= num_movable_nodes)
        {
            T xl1 = x[node_id1]; 
            T xl2 = x[node_id2];
            T width1 = node_size_x[node_id1]; 
            T width2 = node_size_x[node_id2]; 
            T xh1 = xl1 + width1; 
            T xh2 = xl2 + width2; 
            // only collect node_id2 if its right edge is righter than node_id1 
            if (xh1 < xh2)
            {
                tmp_nodes.push_back(node_id2);
                node_id1 = node_id2;
            }
        }
        else 
        {
            tmp_nodes.push_back(node_id2);
            if (node_id2 >= num_movable_nodes)
            {
                node_id1 = node_id2;
            }
        }
    }
    nodes_in_row.swap(tmp_nodes);

    return;
}

DREAMPLACE_END_NAMESPACE

#endif
