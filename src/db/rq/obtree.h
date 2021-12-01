#ifndef SRC_DB_BOXTREE_H_
#define SRC_DB_BOXTREE_H_

#include <vector>
#include <cstdio>
#include <algorithm>
#include <thread>
#define INF 1050000000

#define NUM_TH 4
#define NUM_TREE (NUM_TH + 2)
#define MIN_NODE_SIZE 3

namespace boxtree
{
    using namespace std;
    struct rect
    {
        int xl, yl, xr, yr;
    };
    struct treenode
    {
        int xmin, xmax, ymin, ymax;
        int l, r;
    };
    struct rectdb
    {
        int type;
        std::vector<rect> r;
        std::vector<int> id;
        std::vector<treenode> node;
    };

    extern rectdb rdb[NUM_TH + 2];
    extern vector<rect> ans[NUM_TH + 2];
    extern int treeorder[NUM_TREE];

    void allocatetree(std::vector<rect> &rects);
    int recttype(const rect &a);
    void buildBOXTree(rectdb &rdb, int s, int L, int R);
    void initBOXTreeNode(rectdb &rdb, int s, int L, int R);
    void initBuild(rectdb &rdb);

    void queryBOXTree(const rectdb &rdb, int s, rect &boxq, std::vector<rect> &ansrect);

} // namespace open_edi

#endif // SRC_DB_BOXTREE_H_