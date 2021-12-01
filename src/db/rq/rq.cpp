/* @file  rq.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/rq/rq.h"
#include "db/rq/obtree.h"

namespace open_edi {
namespace db {

DataModel dm;

std::thread threads[NUM_TH];
int thCnt;
std::mutex mtx;
void multThBuild(int thid)
{
    int treeid;
    while (1)
    {
        mtx.lock();
        treeid=thCnt++;
        mtx.unlock();
        if (treeid>=NUM_TREE)
            return;
        // printf("init tree %d in thread %d\n",boxtree::treeorder[treeid],thid);
        boxtree::initBuild(boxtree::rdb[boxtree::treeorder[treeid]]);      
    }
    return;   
}
void multThQuery(int thid, boxtree::rect search_box)
{
    int treeid;
    while (1)
    {
        mtx.lock();
        treeid=thCnt++;
        mtx.unlock();
        if (treeid>=NUM_TREE)
            return;
        boxtree::queryBOXTree(boxtree::rdb[boxtree::treeorder[treeid]],1,search_box,boxtree::ans[boxtree::treeorder[treeid]]);       
    }
    return;   
}
bool cmpans(boxtree::rect a, boxtree::rect b)
{
    if (a.xl!=b.xl) return a.xl<b.xl;
    if (a.yl!=b.yl) return a.yl<b.yl;
    if (a.xr!=b.xr) return a.xr<b.xr;
    return a.yr<b.yr;
}
int initQuery() {    
    // add your code here to do initialization for query 
    Monitor monitor; 
    dm.importAllGeometries();
    monitor.print("import geometries");

    monitor.reset();
    std::vector<LRect> rects = dm.getGeometries();
    std::vector<boxtree::rect> tmprect;
    int tmpsize=rects.size();
    for (int i=0;i<tmpsize;i++)
        tmprect.push_back({rects[i].rect_.getLLX(),rects[i].rect_.getLLY(),rects[i].rect_.getURX(),rects[i].rect_.getURY()});
    boxtree::allocatetree(tmprect);

    thCnt=0;
    for (int i=0;i<NUM_TH;i++)
        threads[i]=std::thread(multThBuild,i);
    for (int i=0;i<NUM_TH;i++) 
        threads[i].join();
        
    monitor.printInternal("build");
    return 0;
}

int query(const Box &search_area) {
    Monitor monitor;
    // add your code here to query data
    boxtree::rect search_box={search_area.getLLX(),search_area.getLLY(),search_area.getURX(),search_area.getURY()};
    thCnt=0;
    for (int i=0;i<NUM_TH;i++)
        threads[i]=std::thread(multThQuery,i,search_box);
    for (int i=0;i<NUM_TH;i++) 
        threads[i].join();
    for (int i=0;i<NUM_TREE;i++)
        printf("result: %ld\n",boxtree::ans[i].size());
    monitor.printInternal("query");

    // message->info("search result is :\n");
    // std::vector<boxtree::rect> tmpans;
    // for (int i = 0; i < NUM_TREE; i++)
    //     for (unsigned int j = 0; j < boxtree::ans[i].size(); j++) 
    //         tmpans.push_back(boxtree::ans[i][j]);
    // sort(tmpans.begin(),tmpans.end(),cmpans);
    // for (unsigned int i = 0; i < tmpans.size(); i++) 
    // {      
    //     message->info("%d %d %d %d\n", tmpans[i].xl, tmpans[i].yl, tmpans[i].xr, tmpans[i].yr);
    // }
    return 0;
}

int cleanupQuery() {
    // add your code here to do cleanup for query
    return 0;
}

int cmdInitQuery(Command* cmd) {
    initQuery();
    return TCL_OK;
}

int cmdQuery(Command* cmd) {
    Box search_area;
    if (cmd->isOptionSet("area")) {
        cmd->getOptionValue("area", search_area);
    } else {
        search_area = getTopCell()->getFloorplan()->getCoreBox();
        // if core box is not set, use die area as searching area
    }
    query(search_area);
    return TCL_OK;
}

int cmdCleanupQuery(Command* cmd) {
    cleanupQuery();
    return TCL_OK;
}

}  // namespace db
}  // namespace open_edi
