/*************************************************************************
    > File Name: 
    > Author: Fei
    > Mail:
    > Created Time: Fri 25 Sep 2020 03:20:14 PM CDT
 ************************************************************************/
#include "lp_dp/legality_check/src/legality_check.h"
#include "utility/src/Msg.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

DREAMPLACE_BEGIN_NAMESPACE

//return value:
//0: legal ok.
//1: not legal.
//-1: error (db not ready).
int LegalityCheck::run()
{
  hr_clock_rep timer_start = get_globaltime(); 
  dreamplacePrint(kINFO, "Starting Legality Check\n");
  // exit if no DB
  if (!isCommonDBReady()) {
    dreamplacePrint(kERROR, "Error out\n");
    return -1;
  }
  int rtn=legalityCheckRun(getDB());
  if (rtn != 0)
  {
      dreamplacePrint(kWARN, "Legality check failed.\n");
  }
  else
  {
      dreamplacePrint(kINFO, "Legality check clean.\n");
  }
  hr_clock_rep timer_stop = get_globaltime(); 
  dreamplacePrint(kINFO, "Legality check takes %g ms\n", (timer_stop-timer_start)*get_timer_period());
  return rtn;
}

DREAMPLACE_END_NAMESPACE
