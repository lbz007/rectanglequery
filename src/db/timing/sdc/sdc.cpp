/**
* @file sdc.cpp
* @date 2020-11-25
* @brief
*
* Copyright (C) 2020 NIIC EDA
*
* All rights reserved.
*
* This software may be modified and distributed under the terms
*
* of the BSD license.  See the LICENSE file for details.
*/

#include "db/timing/sdc/sdc.h"
#include "db/timing/timinglib/analysis_view.h"
#include "db/timing/timinglib/analysis_corner.h"
#include "db/timing/timinglib/analysis_mode.h"

namespace open_edi {
namespace db {

void Sdc::removeClockFromContainers(const ClockId &clock_id) {
    getClockGatingCheckContainer()->removeClock(clock_id);
    getSenseContainer()->removeClock(clock_id);
    getClockTransitionContainer()->removeClock(clock_id);
    getInputDelayContainer()->removeClock(clock_id);
    getMaxTimeBorrowContainer()->removeClock(clock_id);
    getMinPulseWidthContainer()->removeClock(clock_id);
    getOutputDelayContainer()->removeClock(clock_id);
    getPropagatedClockContainer()->removeClock(clock_id);
    getMaxTransitionContainer()->removeClock(clock_id);
    getClockLatencyContainer()->removeClock(clock_id);
    getClockUncertaintyContainer()->removeClock(clock_id);
    getDataCheckContainer()->removeClock(clock_id);
    getClockGroupsContainer()->removeClock(clock_id);
    // Exceptions
    getGroupPathContainer()->removeClock(clock_id);
    getFalsePathContainer()->removeClock(clock_id);
    getMaxDelayContainer()->removeClock(clock_id);
    getMinDelayContainer()->removeClock(clock_id);
    getMulticyclePathContainer()->removeClock(clock_id);
}

std::string Sdc::clockIdToName(const ClockId &clock_id) {
    return getClockContainer()->getClockName(clock_id);
}

AnalysisCorner* Sdc::getAnalysisCorner() const {
    AnalysisView *view = this->getAnalysisView();
    if (!view) {
        return nullptr;
    }
    return view->getAnalysisCorner();
}

AnalysisMode* Sdc::getAnalysisMode() const {
    AnalysisView *view = this->getAnalysisView();
    if (!view) {
        return nullptr;
    }
    return view->getAnalysisMode();
}

void Sdc::print(std::ostream &os) {
    print(os, separator_container_);
    print(os, units_container_);

    print(os, clock_container_);
    print(os, group_path_container_);
    print(os, clock_gating_check_container_);
    print(os, clock_groups_container_);
    print(os, clock_latency_container_);
    print(os, sense_container_);
    print(os, clock_transition_container_);
    print(os, clock_uncertainty_container_);
    print(os, data_check_container_);
    print(os, disable_timing_container_);
    print(os, false_path_container_);
    print(os, ideal_latency_container_);
    print(os, ideal_network_container_);
    print(os, ideal_transition_container_);
    print(os, input_delay_container_);
    print(os, max_delay_container_);
    print(os, max_time_borrow_container_);
    print(os, min_delay_container_);
    print(os, min_pulse_width_container_);
    print(os, multicycle_path_container_);
    print(os, output_delay_container_);
    print(os, propagated_clock_container_);

    print(os, case_analysis_container_);
    print(os, drive_container_);
    print(os, driving_cell_container_);
    print(os, fanout_load_container_);
    print(os, input_transition_container_);
    print(os, load_container_);
    print(os, logic_container_);
    print(os, max_area_container_);
    print(os, max_capacitance_container_);
    print(os, max_fanout_container_);
    print(os, max_transition_container_);
    print(os, min_capacitance_container_);
    print(os, operating_conditions_container_);
    print(os, port_fanout_number_container_);
    print(os, resistance_container_);
    print(os, timing_derate_container_);
    print(os, voltage_container_);
    print(os, wire_load_min_block_size_container_);
    print(os, wire_load_mode_container_);
    print(os, wire_load_model_container_);
    print(os, wire_load_selection_group_container_);

    print(os, voltage_area_container_);;
    print(os, level_shifter_strategy_container_);
    print(os, level_shifter_threshold_container_);
    print(os, max_dynamic_power_container_);
    print(os, max_leakage_power_container_);
}

std::ostream &operator<<(std::ostream &os, Sdc &rhs) {
    rhs.print(os);
    return os;
}

SdcPtr getSdc(ObjectId view_id) {
    AnalysisView *view = Object::addr<AnalysisView>(view_id);
    if (!view) {
        message->issueMsg("SDC", 28, open_edi::util::kError, view_id);
        return nullptr;
    }
    return view->getSdc();
}

SdcPtr getSdc(const std::string &view_name) {
    const auto &timing_lib = getTimingLib();
    AnalysisView *view = timing_lib->getAnalysisView(view_name);
    if (!view) {
        message->issueMsg("SDC", 6, open_edi::util::kError, view_name.c_str());
        return nullptr;
    }
    return view->getSdc();
}

}
}
