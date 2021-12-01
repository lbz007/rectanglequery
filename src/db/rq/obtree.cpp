#include "db/rq/obtree.h"

namespace boxtree
{

    rectdb rdb[NUM_TREE];
    vector<rect> ans[NUM_TREE];
    int treeorder[NUM_TREE];

    bool cmpbox(rectdb &rdb, int cmptype, int a, int b)
    {
        rect &ra=rdb.r[a],&rb=rdb.r[b];
        if (cmptype == 0)
        {
            if (ra.xl != rb.xl) return ra.xl < rb.xl;
            return ra.yl < rb.yl;
        }
        if (cmptype == 1)
        {
            if (ra.xr != rb.xr) return ra.xr < rb.xr;
            return ra.yr < rb.yr;
        }
        if (cmptype == 2)
        {
            if (ra.yl != rb.yl) return ra.yl < rb.yl;
            return ra.xl < rb.xl;
        }
        if (cmptype == 3)
        {
            if (ra.yr != rb.yr) return ra.yr < rb.yr;
            return ra.xr < rb.xr;
        }
        return 0;
    }
    bool cmporder(int a, int b)
    {
        return rdb[a].r.size() > rdb[b].r.size();
    }
    void qselect(rectdb &rdb, int S, int hh, int tt, int k, int cmptype)
    {
        int mid = rdb.id[S + hh + ((tt - hh + 1) >> 1)];
        int i = hh, j = tt;
        while (i <= j)
        {
            while (cmpbox(rdb, cmptype, rdb.id[S + i], mid))
                ++i;
            while (cmpbox(rdb, cmptype, mid, rdb.id[S + j]))
                --j;
            if (i <= j)
            {
                swap(rdb.id[S + i], rdb.id[S + j]);
                ++i;
                --j;
            }
        }
        if (hh <= j && k <= j)
            return qselect(rdb, S, hh, j, k, cmptype);
        if (i <= tt && k >= i)
            return qselect(rdb, S, i, tt, k, cmptype);
        return;
    }
    void allocatetree(std::vector<rect> &rects)
    {
        struct treetype
        {
            int size, intth, sth, lastth;
            double dnum, doubleth;
        } tt[3];
        for (int i = 0; i < 3; i++)
            tt[i].lastth = tt[i].size = 0;
        int rsize = rects.size();
        for (int i = 0; i < rsize; i++)
        {
            tt[recttype(rects[i])].size++;
        }
        vector<bool> ped(rsize, 0);
        double sumdouble = 0;
        for (int i = 0; i < 3; i++)
        {
            tt[i].intth = NUM_TH * tt[i].size / rsize;
            tt[i].doubleth = NUM_TH * 1.0 * tt[i].size / rsize - tt[i].intth;
            sumdouble += tt[i].doubleth;
        }
        tt[0].sth = 0;
        tt[1].sth = tt[0].intth;
        tt[2].sth = tt[1].sth + tt[1].intth;
        for (int i = 0; i < 3; i++)
            tt[i].dnum = tt[i].doubleth / (tt[i].intth + tt[i].doubleth);
        if (sumdouble < 1.5)
        {
            rdb[NUM_TH - 1].type = 0;
            rdb[NUM_TH].type = 1;
            rdb[NUM_TH + 1].type = 2;
            for (int i = 0; i < rsize; i++)
            {
                int t = recttype(rects[i]);
                if (rand() % 10000 < tt[t].dnum * 10000)
                {
                    rdb[NUM_TH - 1 + t].r.push_back(rects[i]);
                    ped[i] = true;
                }
            }
        }
        else
        {
            int maxt = 0, t1 = 1, t2 = 2;
            if (tt[1].doubleth > tt[maxt].doubleth)
            {
                maxt = 1;
                t1 = 0;
                t2 = 2;
            }
            if (tt[2].doubleth > tt[maxt].doubleth)
            {
                maxt = 2;
                t1 = 0;
                t2 = 1;
            }
            rdb[NUM_TH - 2].type = 0;
            rdb[NUM_TH - 1].type = 1;
            rdb[NUM_TH].type = 2;
            rdb[NUM_TH + 1].type = maxt;
            for (int i = 0; i < rsize; i++)
            {
                int t = recttype(rects[i]);
                if (t == maxt)
                {
                    if (rand() % 10000 < tt[t].dnum * 10000)
                    {
                        if (rand() % 10000 < (1 - tt[t1].doubleth) * 10000)
                            rdb[NUM_TH - 2 + t].r.push_back(rects[i]);
                        else
                            rdb[NUM_TH + 1].r.push_back(rects[i]);
                    }
                }
                else
                {
                    if (rand() % 10000 < tt[t].dnum * 10000)
                        rdb[NUM_TH - 2 + t].r.push_back(rects[i]);
                }
                ped[i] = true;
            }
        }
        for (int i = 0; i < rsize; i++)
        {
            if (ped[i])
                continue;
            int t = recttype(rects[i]);
            int treeid;
            treeid = tt[t].sth + tt[t].lastth;
            tt[t].lastth = (tt[t].lastth + 1) % tt[t].intth;
            rdb[treeid].r.push_back(rects[i]);
        }
        for (int i = tt[0].sth; i < tt[1].sth; i++)
            rdb[i].type = 0;
        for (int i = tt[1].sth; i < tt[2].sth; i++)
            rdb[i].type = 1;
        for (int i = tt[2].sth; i < tt[2].sth + tt[2].intth; i++)
            rdb[i].type = 2;
        // for (int i=0;i<NUM_TREE;i++)
        //     printf("tree%d: size=%ld type=%d\n",i,rdb[i].r.size(),rdb[i].type);
        for (int i = 0; i < NUM_TREE; i++)
            treeorder[i] = i;
        sort(treeorder, treeorder + NUM_TREE, cmporder);
        return;
    }
    void initBuild(rectdb &rdb)
    {
        int n = rdb.r.size();
        rdb.id.resize(n);
        for (int i = 0; i < n; i++)
            rdb.id[i] = i;
        buildBOXTree(rdb, 1, 0, n - 1);
        vector<rect> tmprect(n);
        for (int i=0;i<n;i++)
            tmprect[i]=rdb.r[rdb.id[i]];
        for (int i=0;i<n;i++)
            rdb.r[i]=tmprect[i];
        return;
    }
    int recttype(const rect &a)
    {
        if (a.yr - a.yl >= 4 * (a.xr - a.xl))
            return 0; // v net
        if (a.xr - a.xl >= 4 * (a.yr - a.yl))
            return 1; // h net
        return 2;     //cell
    }
    bool inbox(const rect &a, const rect &b)
    {
        if (a.xl >= b.xl && a.xr <= b.xr && a.yl >= b.yl && a.yr <= b.yr)
            return true;
        return false;
    }
    bool outbox(const rect &a, const rect &b)
    {
        if (a.xr < b.xl || a.xl > b.xr || a.yr < b.yl || a.yl > b.yr)
            return true;
        return false;
    }
    void binitBOXTreeNode(rectdb &rdb, int s, int L, int R)
    {
        while (rdb.node.size() < s + 1)
            rdb.node.push_back({});
        rdb.node[s].xmin = rdb.node[s].ymin = INF;
        rdb.node[s].xmax = rdb.node[s].ymax = -INF;
        rdb.node[s].l = L;
        rdb.node[s].r = R;
    }
    void buildBOXTree(rectdb &rdb, int s, int L, int R)
    {
        binitBOXTreeNode(rdb, s, L, R);
        int n = R - L + 1;
        for (int i = L; i <= R; i++)
        {
            rect &tmp=rdb.r[rdb.id[i]];
            rdb.node[s].xmin = std::min(tmp.xl, rdb.node[s].xmin);
            rdb.node[s].xmax = std::max(tmp.xr, rdb.node[s].xmax);
            rdb.node[s].ymin = std::min(tmp.yl, rdb.node[s].ymin);
            rdb.node[s].ymax = std::max(tmp.yr, rdb.node[s].ymax);
        }
        if (n > MIN_NODE_SIZE)
        {
            int h = n >> 1;
            if (rdb.type == 2)
            {
                if (rdb.node[s].xmax - rdb.node[s].xmin > rdb.node[s].ymax - rdb.node[s].ymin)
                    qselect(rdb, L, 0, n - 1, h, s & 1);
                else
                    qselect(rdb, L, 0, n - 1, h, 2 + (s & 1));
            }
            if (rdb.type == 0)
            {
                if ((rdb.node[s].xmax - rdb.node[s].xmin) << 1 > (rdb.node[s].ymax - rdb.node[s].ymin))
                    qselect(rdb, L, 0, n - 1, h, s & 1);
                else
                    qselect(rdb, L, 0, n - 1, h, 2 + (s & 1));
            }
            if (rdb.type == 1)
            {
                if ((rdb.node[s].xmax - rdb.node[s].xmin) > (rdb.node[s].ymax - rdb.node[s].ymin) << 1)
                    qselect(rdb, L, 0, n - 1, h, s & 1);
                else
                    qselect(rdb, L, 0, n - 1, h, 2 + (s & 1));
            }
            buildBOXTree(rdb, s << 1, L, L + h - 1);
            buildBOXTree(rdb, (s << 1) + 1, L + h, R);
        }

        return;
    }
    void queryBOXTree(const rectdb &rdb, int s, rect &boxq, std::vector<rect> &ansrect)
    {
        rect boxs = {rdb.node[s].xmin, rdb.node[s].ymin, rdb.node[s].xmax, rdb.node[s].ymax};
        int L = rdb.node[s].l, R = rdb.node[s].r;
        if (outbox(boxs, boxq))
            return;
        if (inbox(boxs, boxq) || R == L)
        {
            for (int i = L; i <= R; i++)
                ansrect.push_back(rdb.r[i]);
            return;
        }
        if (R - L < MIN_NODE_SIZE)
        {
            for (int i = L; i <= R; i++)
                if (!outbox(rdb.r[i], boxq))
                    ansrect.push_back(rdb.r[i]);
            return;
        }
        queryBOXTree(rdb, s << 1, boxq, ansrect);
        queryBOXTree(rdb, (s << 1) + 1, boxq, ansrect);
        return;
    }

} // namespace db
