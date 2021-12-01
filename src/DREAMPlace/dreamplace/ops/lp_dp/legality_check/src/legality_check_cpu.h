/**
 * @file   legality_check_cpu.h
 * @author Fly
 * @date   Nov 2020
 */

#ifndef DREAMPLACE_OEM_LEGALITY_CHECK_H
#define DREAMPLACE_OEM_LEGALITY_CHECK_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include "utility/src/Msg.h"
#include "utility/src/common.h"

DREAMPLACE_BEGIN_NAMESPACE

/// compare nodes with x center 
/// resolve ambiguity by index 
/*template <typename T>
struct CompareByNodeXCenterNew
{
    const T* x; 
    const T* node_size_x; 

    CompareByNodeXCenterNew(const T* xx, const T* size_x)
        : x(xx)
        , node_size_x(size_x)
    {
    }

    bool operator()(int i, int j) const 
    {
        T xc1 = x[i]+node_size_x[i]/2;
        T xc2 = x[j]+node_size_x[j]/2;
        return (xc1 < xc2) || (xc1 == xc2 && i < j); 
    }
};*/

template <typename T>
bool boundaryCheckNew(
        const T* x, const T* y, 
        const T* node_size_x, const T* node_size_y, 
        T xl, T yl, T xh, T yh,
        const int num_movable_nodes
        )
{
    bool legal_flag = true; 
    // check node within boundary
    for (int i = 0; i < num_movable_nodes; ++i)
    {
        T node_xl = x[i]; 
        T node_yl = y[i];
        T node_xh = node_xl+node_size_x[i];
        T node_yh = node_yl+node_size_y[i];
        if (node_xl < xl || node_xh > xh || node_yl < yl || node_yh > yh)
        {
            dreamplacePrint(kDEBUG, "node %d (%g, %g, %g, %g) out of boundary\n", i, node_xl, node_yl, node_xh, node_yh);
            legal_flag = false; 
        }
    }
    return legal_flag; 
}

template <typename T>
bool siteAlignmentCheckNew(
        const T* x, const T* y, 
        const T site_width, const T row_height, 
        const T xl, const T yl, 
        const int num_movable_nodes 
        )
{
    bool legal_flag = true; 
    // check row and site alignment 
    for (int i = 0; i < num_movable_nodes; ++i)
    {
        T node_xl = x[i]; 
        T node_yl = y[i];

        T row_id = (node_yl - yl) / row_height; 
        T row_yl = yl + row_height * (int)row_id; 
        T row_yh = row_yl + row_height; 

        if (row_id != int(row_id))
        {
            dreamplacePrint(kERROR, "node %d (%g, %g) failed to align to row %d (%g, %g)\n", i, node_xl, node_yl, (int)row_id, row_yl, row_yh);
            legal_flag = false; 
        }

        T site_id = (node_xl - xl) / site_width; 
        if (site_id != int(site_id))
        {
            dreamplacePrint(kERROR, "node %d (%g, %g) failed to align to row %d (%g, %g) and site\n", i, node_xl, node_yl, (int)row_id, row_yl, row_yh);
            legal_flag = false; 
        }
    }

    return legal_flag; 
}

template <typename T>
bool fenceRegionCheckNew(
        const T* node_size_x, const T* node_size_y, 
        const T* flat_region_boxes, const int* flat_region_boxes_start, const int* node2fence_region_map, 
        const T* x, const T* y, 
        const int num_movable_nodes, 
        const int num_regions
        )
{
    bool legal_flag = true; 
    // check fence regions 
    for (int i = 0; i < num_movable_nodes; ++i)
    {
        T node_xl = x[i]; 
        T node_yl = y[i];
        T node_xh = node_xl + node_size_x[i];
        T node_yh = node_yl + node_size_y[i];

        int region_id = node2fence_region_map[i]; 
        if (region_id < num_regions)
        {
            int box_bgn = flat_region_boxes_start[region_id];
            int box_end = flat_region_boxes_start[region_id + 1];
            T node_area = (node_xh - node_xl) * (node_yh - node_yl);
            // I assume there is no overlap between boxes of a region 
            // otherwise, preprocessing is required 
            for (int box_id = box_bgn; box_id < box_end; ++box_id)
            {
                int box_offset = box_id*4; 
                T box_xl = flat_region_boxes[box_offset];
                T box_yl = flat_region_boxes[box_offset + 1];
                T box_xh = flat_region_boxes[box_offset + 2];
                T box_yh = flat_region_boxes[box_offset + 3];

                T dx = std::max(std::min(node_xh, box_xh) - std::max(node_xl, box_xl), (T)0); 
                T dy = std::max(std::min(node_yh, box_yh) - std::max(node_yl, box_yl), (T)0); 
                T overlap = dx*dy; 
                if (overlap > 0)
                {
                    node_area -= overlap; 
                }
            }
            if (node_area > 0) // not consumed by boxes within a region 
            {
                dreamplacePrint(kERROR, "node %d (%g, %g, %g, %g), out of fence region %d", 
                        i, node_xl, node_yl, node_xh, node_yh, region_id);
                for (int box_id = box_bgn; box_id < box_end; ++box_id)
                {
                    int box_offset = box_id*4; 
                    T box_xl = flat_region_boxes[box_offset];
                    T box_yl = flat_region_boxes[box_offset + 1];
                    T box_xh = flat_region_boxes[box_offset + 2];
                    T box_yh = flat_region_boxes[box_offset + 3];

                    dreamplacePrint(kNONE, " (%g, %g, %g, %g)", box_xl, box_yl, box_xh, box_yh);
                }
                dreamplacePrint(kNONE, "\n");
                legal_flag = false; 
            }
        }
    }
    return legal_flag; 
}

template <typename T>
bool overlapCheckNew(
        const T* node_size_x, const T* node_size_y, 
        const T* x, const T* y, 
        T row_height, 
        T xl, T yl, T xh, T yh,
        const int num_nodes, 
        const int num_movable_nodes
        )
{
    bool legal_flag = true; 
    int num_rows = ceil((yh-yl)/row_height);
    dreamplaceAssert(num_rows > 0); 
    std::vector<std::vector<int> > row_nodes (num_rows);

    // general to node and fixed boxes 
    auto getXL = [&](int id) {
        return x[id];
    };
    auto getYL = [&](int id) {
        return y[id];
    };
    auto getXH = [&](int id) {
        return x[id] + node_size_x[id];
    };
    auto getYH = [&](int id) {
        return y[id] + node_size_y[id];
    };
    // add a box to row 
    auto addBox2Row = [&](int id, T bxl, T byl, T bxh, T byh){
        int row_idxl = (byl-yl)/row_height; 
        int row_idxh = ceil((byh-yl)/row_height)+1;
        row_idxl = std::max(row_idxl, 0); 
        row_idxh = std::min(row_idxh, num_rows); 

        for (int row_id = row_idxl; row_id < row_idxh; ++row_id)
        {
            T row_yl = yl+row_id*row_height; 
            T row_yh = row_yl+row_height; 

            if (byl < row_yh && byh > row_yl) // overlap with row 
            {
                row_nodes[row_id].push_back(id); 
            }
        }
    };
    // distribute movable cells to rows 
    for (int i = 0; i < num_nodes; ++i)
    {
        T node_xl = x[i]; 
        T node_yl = y[i];
        T node_xh = node_xl+node_size_x[i];
        T node_yh = node_yl+node_size_y[i];

        addBox2Row(i, node_xl, node_yl, node_xh, node_yh); 
    }

    // sort cells within rows 
    for (int i = 0; i < num_rows; ++i)
    {
        auto& nodes_in_row = row_nodes.at(i);
        // using left edge 
        std::sort(nodes_in_row.begin(), nodes_in_row.end(), 
                [&](int node_id1, int node_id2){
                    T x1 = getXL(node_id1);
                    T x2 = getXL(node_id2);
                    return x1 < x2 || (x1 == x2 && (node_id1 < node_id2));
                });
        // After sorting by left edge, 
        // there is a special case for fixed cells where 
        // one fixed cell is completely within another in a row. 
        // This will cause failure to detect some overlaps. 
        // We need to remove the "small" fixed cell that is inside another. 
        if (!nodes_in_row.empty())
        {
            removeContainedFixedCellsFromRow(nodes_in_row, num_movable_nodes, x, node_size_x);
        }
    }

    // check overlap 
    int num = 0;
    const int maxNum = 20;
    for (int i = 0; i < num_rows; ++i)
    {
        for (unsigned int j = 0; j < row_nodes.at(i).size(); ++j)
        {
            if (j > 0 && (num < maxNum))
            {
                int node_id = row_nodes[i][j]; 
                int prev_node_id = row_nodes[i][j-1]; 

                if (node_id < num_movable_nodes || prev_node_id < num_movable_nodes) // ignore two fixed nodes
                {
                    T prev_xh = getXH(prev_node_id); 
                    T cur_xl = getXL(node_id); 
                    if (prev_xh > cur_xl) // detect overlap 
                    {
                        T prev_xl = getXL(prev_node_id); 
                        T prev_yl = getYL(prev_node_id); 
                        T prev_yh = getYH(prev_node_id); 
                        T cur_yl = getYL(node_id); 
                        T cur_xh = getXH(node_id); 
                        T cur_yh = getYH(node_id); 
                        dreamplacePrint(kERROR, "row %d, overlap node %d (%d, %d, %d, %d) with node %d (%d, %d, %d, %d)\n", 
                                i, 
                                prev_node_id, prev_xl, prev_yl, prev_xh, prev_yh, 
                                node_id, cur_xl, cur_yl, cur_xh, cur_yh 
                              );
                        legal_flag = false; 
                        num++;
                    }
                }
            }
        }
    }

    return legal_flag; 
}

template <typename T>
bool legalityCheckSiteMapKernelCPUNew(
        const T* init_x, const T* init_y, 
        const T* node_size_x, const T* node_size_y, 
        const T* x, const T* y, 
        T xl, T yl, T xh, T yh,
        T site_width, T row_height, 
        const int num_nodes, 
        const int num_movable_nodes
        )
{
    int num_rows = ceil((yh-yl))/row_height; 
    int num_sites = ceil((xh-xl)/site_width);
    std::vector<std::vector<unsigned char> > site_map (num_rows, std::vector<unsigned char>(num_sites, 0)); 

    // fixed macros 
    for (int i = num_movable_nodes; i < num_nodes; ++i)
    {
        T node_xl = x[i]; 
        T node_yl = y[i];
        T node_xh = node_xl+node_size_x[i];
        T node_yh = node_yl+node_size_y[i];

        int idxl = (node_xl-xl)/site_width;
        int idxh = ceil((node_xh-xl)/site_width)+1;
        int idyl = (node_yl-yl)/row_height;
        int idyh = ceil((node_yh-yl)/row_height)+1;
        idxl = std::max(idxl, 0); 
        idxh = std::min(idxh, num_sites); 
        idyl = std::max(idyl, 0); 
        idyh = std::min(idyh, num_rows);

        for (int iy = idyl; iy < idyh; ++iy)
        {
            for (int ix = idxl; ix < idxh; ++ix)
            {
                T site_xl = xl+ix*site_width; 
                T site_xh = site_xl+site_width;
                T site_yl = yl+iy*row_height;
                T site_yh = site_yl+row_height;

                if (node_xl < site_xh && node_xh > site_xl 
                        && node_yl < site_yh && node_yh > site_yl) // overlap 
                {
                    site_map[iy][ix] = 255; 
                }
            }
        }
    }

    bool legal_flag = true; 
    // movable cells 
    for (int i = 0; i < num_movable_nodes; ++i)
    {
        T node_xl = x[i]; 
        T node_yl = y[i];
        T node_xh = node_xl+node_size_x[i];
        T node_yh = node_yl+node_size_y[i];

        int idxl = (node_xl-xl)/site_width;
        int idxh = ceil((node_xh-xl)/site_width)+1;
        int idyl = (node_yl-yl)/row_height;
        int idyh = ceil((node_yh-yl)/row_height)+1;
        idxl = std::max(idxl, 0); 
        idxh = std::min(idxh, num_sites); 
        idyl = std::max(idyl, 0); 
        idyh = std::min(idyh, num_rows);

        for (int iy = idyl; iy < idyh; ++iy)
        {
            for (int ix = idxl; ix < idxh; ++ix)
            {
                T site_xl = xl+ix*site_width; 
                T site_xh = site_xl+site_width;
                T site_yl = yl+iy*row_height;
                T site_yh = site_yl+row_height;

                if (node_xl < site_xh && node_xh > site_xl 
                        && node_yl < site_yh && node_yh > site_yl) // overlap 
                {
                    if (site_map[iy][ix])
                    {
                        dreamplacePrint(kERROR, "detect overlap at site (%g, %g, %g, %g) for node %d (%g, %g, %g, %g)\n", 
                                site_xl, site_yl, site_xh, site_yh, 
                                i, 
                                node_xl, node_yl, node_xh, node_yh
                                );
                        legal_flag = false; 
                    }
                    site_map[iy][ix] += 1; 
                }
            }
        }
    }

    return legal_flag; 
}

DREAMPLACE_END_NAMESPACE

#endif
