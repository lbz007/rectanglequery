/*************************************************************************
    > File Name: lpdp_place.cpp
    > Author: Fei
    > Mail:
    > Created Time: Fri 25 Sep 2020 03:20:14 PM CDT
 ************************************************************************/
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "flow/src/lpdp_place.h"
#include "utility/src/Msg.h"

DREAMPLACE_BEGIN_NAMESPACE

// class LegalDetailedDB member functions
bool
LpdpPlace::isLegalCheckPass()
{
  if (getFlowSteps() & kLegalityCheck) {
    LegalityCheck legalityCheck;
    return (0 == legalityCheck.run()) ? true : false;
  }
  return true;
}

void
LpdpPlace::run()
{
  dreamplacePrint(kINFO, "Starting LP_DP run \n");

  // exit if db is not ready
  if (!isDBReady()) {
    return;
  }

  /// legalization place

  // macro legalize
  if (getFlowSteps() & kMacroLegalize) {
    MacroLegal macroLegalize;
    macroLegalize.run();
  }

  // greedy legalize
  if (getFlowSteps() & kGreedLegalize) {
    GreedyLegal greedyLegalize;
    greedyLegalize.run();
  }

  // legality check
  if (!isLegalCheckPass()) {
    return;
  }
  // abacus legalize
  if (getFlowSteps() & kAbacusLegalize) {
    AbacusLegal abacusLegalize;
    abacusLegalize.run();
  }

  db_->reportHPWL("Legalization");
  db_->reportFLUTE("Legalization");

  if (!isLegalCheckPass()) {
    return;
  }

  /// detail place
  // k reorder
  if (getFlowSteps() & kKReorder1) {
    KReorder kReorder;
    kReorder.run();
  }

  if (!isLegalCheckPass()) {
    return;
  }

  // independent set matching
  if (getFlowSteps() & kIndependentSM) {
    IndependentSetMatching independentSM;
    independentSM.run();
  }

  if (!isLegalCheckPass()) {
    return;
  }

  // global swap
  if (getFlowSteps() & kGlobalSwap) {
    GlobalSwap globalSwap;
    globalSwap.run();
  }

  if (!isLegalCheckPass()) {
    return;
  }

  // k reoder again
  if (getFlowSteps() & kKReorder2) {
    KReorder kReorder;
    kReorder.run();
  }

  db_->reportHPWL("Detail Place");
  db_->reportFLUTE("Detail Place");

  dreamplacePrint(kINFO, "Finish LP_DP run \n");
}

DREAMPLACE_END_NAMESPACE
