/*************************************************************************
    > File Name: 
    > Author: Fei
    > Mail:
    > Created Time: Fri 25 Sep 2020 03:20:14 PM CDT
 ************************************************************************/
#include "lp_dp/abacus_legalize/src/abacus_legalize.h"
#include "lp_dp/abacus_legalize/src/abacus_legalize_cpu.h"
#include "utility/src/Msg.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

DREAMPLACE_BEGIN_NAMESPACE

int 
abacusLegalizationRun(LegalizationDB<int>& db)
{
  // hard code from dreamPlace
  int tmpNumBinsX = 1;
  int tmpNumBinsY = 64;
  abacusLegalizationCPU(
      db.init_x, db.init_y, 
      db.node_size_x, db.node_size_y, db.node_weights,
      db.x, db.y, 
      db.xl, db.yl, db.xh, db.yh, 
      db.site_width, db.row_height, 
      tmpNumBinsX, tmpNumBinsY,
      db.num_nodes, 
      db.num_movable_nodes
      );
  return 0;
}

inline void
AbacusLegal::initLegalizationDB(LegalizationDB<int>& db)
{
  db_->initLegalizationDB(db);
}

void 
AbacusLegal::run()
{
  hr_clock_rep timer_start = get_globaltime(); 
  dreamplacePrint(kINFO, "Starting Abacus Legalization\n");
  // exit if no DB
  if (!isCommonDBReady()) {
    dreamplacePrint(kINFO, "Exit Abacus Legalization w/o DB\n");
    return;
  }

  LegalizationDB<int> db;
  initLegalizationDB(db);
  abacusLegalizationRun(db);

  hr_clock_rep timer_stop = get_globaltime(); 
  dreamplacePrint(kINFO, "Abacus legalization takes %g ms\n", (timer_stop-timer_start)*get_timer_period());
}

DREAMPLACE_END_NAMESPACE
