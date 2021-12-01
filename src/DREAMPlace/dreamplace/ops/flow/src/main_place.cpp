/*************************************************************************
    > File Name: 
    > Author: Fei
    > Mail:
    > Created Time: Fri 25 Sep 2020 03:20:14 PM CDT
 ************************************************************************/
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "flow/src/main_place.h"
#include "utility/src/Msg.h"

DREAMPLACE_BEGIN_NAMESPACE

// class LegalDetailedDB member functions

void
MainPlace::updateDB()
{
  db_->updateDB2EDI();
}

void
MainPlace::summaryMovement()
{
  db_->summaryMovement();
}

void
MainPlace::copyCur2InitLoc()
{
  db_->copyCur2InitLoc();
}

void
MainPlace::run()
{
  hr_clock_rep start_time = get_globaltime();
  // exit if db is not ready
  dreamplacePrint(kINFO, "Starting place_design main flow\n");
  if (!isDBReady()) {
    return;
  }

  if (runGPFlow()) {
    /// global place
    GlobalPlace gplace;
    if (gplace.run() != 0)
    {
      return;
    }
  }

  if (runDPFlow()) {
    // update global results to init loc
    if (runGPFlow()) {
      copyCur2InitLoc();
    }
    /// lpdp place 
    LpdpPlace lpdp;
    lpdp.run();
  }

  // update DB
  updateDB();

  hr_clock_rep stop_time = get_globaltime();
  dreamplacePrint(kINFO, "Main place time: %g ms\n", get_timer_period()*(stop_time - start_time));
}

void PlaceFactory::runPlacer(Para& para)
{
  if (runQPlace(para)) { // quadratic place
    QPlacer qplacer(para.getPartitionNum());
    qplacer.run();
  }

  if (runMPlace(para)) { // main place: GP, LP, DP
    MainPlace place(para);
    place.run();
  }
}

DREAMPLACE_END_NAMESPACE
