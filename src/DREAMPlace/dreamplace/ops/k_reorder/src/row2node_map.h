/**
 * @file   row2node_map.h
 * @author Yibo Lin
 * @date   Apr 2019
 */
#ifndef _DREAMPLACE_K_REORDER_ROW2NODE_MAP_H
#define _DREAMPLACE_K_REORDER_ROW2NODE_MAP_H

#include "utility/src/common.h"

DREAMPLACE_BEGIN_NAMESPACE

/// @brief distribute cells to rows 
template <typename DetailedPlaceDBType>
void make_row2node_map(const DetailedPlaceDBType& db, const typename DetailedPlaceDBType::type* vx, const typename DetailedPlaceDBType::type* vy, std::vector<std::vector<int> >& row2node_map, int num_threads) 
{
    // distribute cells to rows 
    for (int i = 0; i < db.num_nodes; ++i)
    {
        //typename DetailedPlaceDBType::type node_xl = vx[i]; 
        typename DetailedPlaceDBType::type node_yl = vy[i];
        //typename DetailedPlaceDBType::type node_xh = node_xl+db.node_size_x[i];
        typename DetailedPlaceDBType::type node_yh = node_yl+db.node_size_y[i];

        int row_idxl = (node_yl-db.yl)/db.row_height; 
        int row_idxh = ceil((node_yh-db.yl)/db.row_height)+1;
        row_idxl = std::max(row_idxl, 0); 
        row_idxh = std::min(row_idxh, db.num_sites_y); 

        for (int row_id = row_idxl; row_id < row_idxh; ++row_id)
        {
            typename DetailedPlaceDBType::type row_yl = db.yl+row_id*db.row_height; 
            typename DetailedPlaceDBType::type row_yh = row_yl+db.row_height; 

            if (node_yl < row_yh && node_yh > row_yl) // overlap with row 
            {
                row2node_map[row_id].push_back(i); 
            }
        }
    }

#pragma omp parallel for num_threads (num_threads) schedule(dynamic, 1)
    for (int i = 0; i < db.num_sites_y; ++i)
    {
        auto& row2nodes = row2node_map[i];
        // sort cells within rows according to left edges 
        std::sort(row2nodes.begin(), row2nodes.end(), 
                [&] (int node_id1, int node_id2) {
                    typename DetailedPlaceDBType::type x1 = vx[node_id1];
                    typename DetailedPlaceDBType::type x2 = vx[node_id2];
                    return x1 < x2 || (x1 == x2 && node_id1 < node_id2);
                });
        // After sorting by left edge, 
        // there is a special case for fixed cells where 
        // one fixed cell is completely within another in a row. 
        // This will cause failure to detect some overlaps. 
        // We need to remove the "small" fixed cell that is inside another. 
        if (!row2nodes.empty())
        {
            removeContainedFixedCellsFromRow(row2nodes, db.num_movable_nodes, vx, db.node_size_x);
            // sort according to center 
            std::sort(row2nodes.begin(), row2nodes.end(), 
                    [&] (int node_id1, int node_id2) {
                    typename DetailedPlaceDBType::type x1 = vx[node_id1] + db.node_size_x[node_id1]/2;
                    typename DetailedPlaceDBType::type x2 = vx[node_id2] + db.node_size_x[node_id2]/2;
                    return x1 < x2 || (x1 == x2 && node_id1 < node_id2);
                    });
        }
    }
}

DREAMPLACE_END_NAMESPACE

#endif
