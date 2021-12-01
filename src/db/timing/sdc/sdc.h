/**
 * @file sdc.h
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

#ifndef EDI_DB_TIMING_SDC_SDC_H_
#define EDI_DB_TIMING_SDC_SDC_H_

#include "db/timing/sdc/sdc_data.h"

namespace open_edi {
namespace db {

class Sdc {
  public:
    Sdc()=default;
    Sdc(ObjectId view_id) { analysis_view_id_ = view_id; }
    Sdc(const Sdc &rhs)=delete;
    Sdc &operator=(const Sdc &rhs)=delete;
    ~Sdc()=default;

    // APIs for data shared between containers' data, called by base_container_data.h
    void removeClockFromContainers(const ClockId &clock_id);
    std::string clockIdToName(const ClockId &clock_id);

    //get
    const ObjectId getAnalysisViewId() const { return analysis_view_id_; }
    AnalysisView *getAnalysisView() const { return Object::addr<AnalysisView>(analysis_view_id_); }
    AnalysisCorner *getAnalysisCorner() const;
    AnalysisMode *getAnalysisMode() const;

    //get
    SdcCurrentInstanceContainerPtr &getCurrentInstanceContainer() { return getOrCreate<SdcCurrentInstanceContainer>(inst_container_); }
    SdcHierarchySeparatorContainerPtr &getHierarchySeparatorContainer() { return getOrCreate<SdcHierarchySeparatorContainer>(separator_container_); }
    SdcUnitsContainerPtr &getUnitsContainer() { return getOrCreate<SdcUnitsContainer>(units_container_); }

    SdcClockContainerPtr &getClockContainer() { return getOrCreate<SdcClockContainer>(clock_container_); }
    SdcGroupPathContainerPtr &getGroupPathContainer() { return getOrCreate<SdcGroupPathContainer>(group_path_container_); }
    SdcClockGatingCheckContainerPtr &getClockGatingCheckContainer() { return getOrCreate<SdcClockGatingCheckContainer>(clock_gating_check_container_); }
    SdcClockGroupsContainerPtr &getClockGroupsContainer() { return getOrCreate<SdcClockGroupsContainer>(clock_groups_container_); }
    SdcClockLatencyContainerPtr &getClockLatencyContainer() { return getOrCreate<SdcClockLatencyContainer>(clock_latency_container_); }
    SdcSenseContainerPtr &getSenseContainer() { return getOrCreate<SdcSenseContainer>(sense_container_); }
    SdcClockTransitionContainerPtr &getClockTransitionContainer() { return getOrCreate<SdcClockTransitionContainer>(clock_transition_container_); }
    SdcClockUncertaintyContainerPtr &getClockUncertaintyContainer() { return getOrCreate<SdcClockUncertaintyContainer>(clock_uncertainty_container_); }
    SdcDataCheckContainerPtr &getDataCheckContainer() { return getOrCreate<SdcDataCheckContainer>(data_check_container_); }
    SdcDisableTimingContainerPtr &getDisableTimingContainer() { return getOrCreate<SdcDisableTimingContainer>(disable_timing_container_); }
    SdcFalsePathContainerPtr &getFalsePathContainer() { return getOrCreate<SdcFalsePathContainer>(false_path_container_); }
    SdcIdealLatencyContainerPtr &getIdealLatencyContainer() { return getOrCreate<SdcIdealLatencyContainer>(ideal_latency_container_); }
    SdcIdealNetworkContainerPtr &getIdealNetworkContainer() { return getOrCreate<SdcIdealNetworkContainer>(ideal_network_container_); }
    SdcIdealTransitionContainerPtr &getIdealTransitionContainer() { return getOrCreate<SdcIdealTransitionContainer>(ideal_transition_container_); }
    SdcInputDelayContainerPtr &getInputDelayContainer() { return getOrCreate<SdcInputDelayContainer>(input_delay_container_); }
    SdcMaxDelayContainerPtr &getMaxDelayContainer() { return getOrCreate<SdcMaxDelayContainer>(max_delay_container_); }
    SdcMaxTimeBorrowContainerPtr &getMaxTimeBorrowContainer() { return getOrCreate<SdcMaxTimeBorrowContainer>(max_time_borrow_container_); }
    SdcMinDelayContainerPtr &getMinDelayContainer() { return getOrCreate<SdcMinDelayContainer>(min_delay_container_); }
    SdcMinPulseWidthContainerPtr &getMinPulseWidthContainer() { return getOrCreate<SdcMinPulseWidthContainer>(min_pulse_width_container_); }
    SdcMulticyclePathContainerPtr &getMulticyclePathContainer() { return getOrCreate<SdcMulticyclePathContainer>(multicycle_path_container_); }
    SdcOutputDelayContainerPtr &getOutputDelayContainer() { return getOrCreate<SdcOutputDelayContainer>(output_delay_container_); }
    SdcPropagatedClockContainerPtr &getPropagatedClockContainer() { return getOrCreate<SdcPropagatedClockContainer>(propagated_clock_container_); }

    SdcCaseAnalysisContainerPtr &getCaseAnalysisContainer() { return getOrCreate<SdcCaseAnalysisContainer>(case_analysis_container_); }
    SdcDriveContainerPtr &getDriveContainer() { return getOrCreate<SdcDriveContainer>(drive_container_); }
    SdcDrivingCellContainerPtr &getDrivingCellContainer() { return getOrCreate<SdcDrivingCellContainer>(driving_cell_container_); }
    SdcFanoutLoadContainerPtr &getFanoutLoadContainer() { return getOrCreate<SdcFanoutLoadContainer>(fanout_load_container_); }
    SdcInputTransitionContainerPtr &getInputTransitionContainer() { return getOrCreate<SdcInputTransitionContainer>(input_transition_container_); }
    SdcLoadContainerPtr &getLoadContainer() { return getOrCreate<SdcLoadContainer>(load_container_); }
    SdcLogicContainerPtr &getLogicContainer() { return getOrCreate<SdcLogicContainer>(logic_container_); }
    SdcMaxAreaContainerPtr &getMaxAreaContainer() { return getOrCreate<SdcMaxAreaContainer>(max_area_container_); }
    SdcMaxCapacitanceContainerPtr &getMaxCapacitanceContainer() { return getOrCreate<SdcMaxCapacitanceContainer>(max_capacitance_container_); }
    SdcMaxFanoutContainerPtr &getMaxFanoutContainer() { return getOrCreate<SdcMaxFanoutContainer>(max_fanout_container_);}
    SdcMaxTransitionContainerPtr &getMaxTransitionContainer() { return getOrCreate<SdcMaxTransitionContainer>(max_transition_container_); }
    SdcMinCapacitanceContainerPtr &getMinCapacitanceContainer() { return getOrCreate<SdcMinCapacitanceContainer>(min_capacitance_container_); }
    SdcOperatingConditionsContainerPtr &getOperatingConditionsContainer() { return getOrCreate<SdcOperatingConditionsContainer>(operating_conditions_container_); }
    SdcPortFanoutNumberContainerPtr &getPortFanoutNumberContainer() { return getOrCreate<SdcPortFanoutNumberContainer>(port_fanout_number_container_); }
    SdcResistanceContainerPtr &getResistanceContainer() { return getOrCreate<SdcResistanceContainer>(resistance_container_); }
    SdcTimingDerateContainerPtr &getTimingDerateContainer() { return getOrCreate<SdcTimingDerateContainer>(timing_derate_container_); }
    SdcVoltageContainerPtr &getVoltageContainer() { return getOrCreate<SdcVoltageContainer>(voltage_container_);}
    SdcWireLoadMinBlockSizeContainerPtr &getWireLoadMinBlockSizeContainer() { return getOrCreate<SdcWireLoadMinBlockSizeContainer>(wire_load_min_block_size_container_); }
    SdcWireLoadModeContainerPtr &getWireLoadModeContainer() { return getOrCreate<SdcWireLoadModeContainer>(wire_load_mode_container_); }
    SdcWireLoadModelContainerPtr &getWireLoadModelContainer() { return getOrCreate<SdcWireLoadModelContainer>(wire_load_model_container_); }
    SdcWireLoadSelectionGroupContainerPtr &getWireLoadSelectionGroupContainer() { return getOrCreate<SdcWireLoadSelectionGroupContainer>(wire_load_selection_group_container_); }

    SdcVoltageAreaContainerPtr &getVoltageAreaContainer() { return getOrCreate<SdcVoltageAreaContainer>(voltage_area_container_); };
    SdcLevelShifterStrategyContainerPtr &getLevelShifterStrategyContainer() { return getOrCreate<SdcLevelShifterStrategyContainer>(level_shifter_strategy_container_); }
    SdcLevelShifterThresholdContainerPtr &getLevelShifterThresholdContainer() { return getOrCreate<SdcLevelShifterThresholdContainer>(level_shifter_threshold_container_); }
    SdcMaxDynamicPowerContainerPtr &getMaxDynamicPowerContainer() { return getOrCreate<SdcMaxDynamicPowerContainer>(max_dynamic_power_container_); }
    SdcMaxLeakagePowerContainerPtr &getMaxLeakagePowerContainer() { return getOrCreate<SdcMaxLeakagePowerContainer>(max_leakage_power_container_); }

    //object access commands
    SdcAllClocksContainerPtr &getAllClocksContainer() { return getOrCreate<SdcAllClocksContainer>(all_clocks_container_); }
    SdcAllInputsContainerPtr &getAllInputsContainer() { return getOrCreate<SdcAllInputsContainer>(all_inputs_container_); }
    SdcAllRegistersContainerPtr &getAllRegistersContainer() { return getOrCreate<SdcAllRegistersContainer>(all_registers_container_); }
    SdcCurrentDesignContainerPtr &getCurrentDesignContainer() { return getOrCreate<SdcCurrentDesignContainer>(design_container_); }
    SdcGetCellsContainerPtr &getCellsContainer() { return getOrCreate<SdcGetCellsContainer>(get_cells_container_); }
    SdcGetClocksContainerPtr &getClocksContainer() { return getOrCreate<SdcGetClocksContainer>(get_clocks_container_); }
    SdcGetLibCellsContainerPtr &getLibCellsContainer() { return getOrCreate<SdcGetLibCellsContainer>(get_lib_cells_container_); }
    SdcGetLibPinsContainerPtr &getLibPinsContainer() { return getOrCreate<SdcGetLibPinsContainer>(get_lib_pins_container_); }
    SdcGetLibsContainerPtr &getLibsContainer() { return getOrCreate<SdcGetLibsContainer>(get_libs_container_); }
    SdcGetNetsContainerPtr &getNetsContainer() { return getOrCreate<SdcGetNetsContainer>(get_nets_container_); }
    SdcGetPinsContainerPtr &getPinsContainer() { return getOrCreate<SdcGetPinsContainer>(get_pins_container_); }
    SdcGetPortsContainerPtr &getPortsContainer() { return getOrCreate<SdcGetPortsContainer>(get_ports_container_); }

  public:
    template<typename U>
    std::shared_ptr<U> &getOrCreate(std::shared_ptr<U> &member_ptr) {
        if (!member_ptr) {
            member_ptr = std::make_shared<U>();
        }
        return member_ptr;
    }

    template<typename V>
    void print(std::ostream &os, V &member_ptr) {
        if (member_ptr) {
            os << *(member_ptr) << "\n";
        }
    }

    void print(std::ostream &os);

    friend std::ostream &operator<<(std::ostream &os, Sdc &rhs);

  private:
    SdcCurrentInstanceContainerPtr inst_container_ = nullptr;
    SdcHierarchySeparatorContainerPtr separator_container_ = nullptr;
    SdcUnitsContainerPtr units_container_ = nullptr;

    SdcClockContainerPtr clock_container_ = nullptr;
    SdcGroupPathContainerPtr group_path_container_ = nullptr;
    SdcClockGatingCheckContainerPtr clock_gating_check_container_ = nullptr;
    SdcClockGroupsContainerPtr clock_groups_container_ = nullptr;
    SdcClockLatencyContainerPtr clock_latency_container_ = nullptr;
    SdcSenseContainerPtr sense_container_ = nullptr;
    SdcClockTransitionContainerPtr clock_transition_container_ = nullptr;
    SdcClockUncertaintyContainerPtr clock_uncertainty_container_ = nullptr;
    SdcDataCheckContainerPtr data_check_container_ = nullptr;
    SdcDisableTimingContainerPtr disable_timing_container_ = nullptr;
    SdcFalsePathContainerPtr false_path_container_ = nullptr;
    SdcIdealLatencyContainerPtr ideal_latency_container_ = nullptr;
    SdcIdealNetworkContainerPtr ideal_network_container_ = nullptr;
    SdcIdealTransitionContainerPtr ideal_transition_container_ = nullptr;
    SdcInputDelayContainerPtr input_delay_container_ = nullptr;
    SdcMaxDelayContainerPtr max_delay_container_ = nullptr;
    SdcMaxTimeBorrowContainerPtr max_time_borrow_container_ = nullptr;
    SdcMinDelayContainerPtr min_delay_container_ = nullptr;
    SdcMinPulseWidthContainerPtr min_pulse_width_container_ = nullptr;
    SdcMulticyclePathContainerPtr multicycle_path_container_ = nullptr;
    SdcOutputDelayContainerPtr output_delay_container_ = nullptr;
    SdcPropagatedClockContainerPtr propagated_clock_container_ = nullptr;

    SdcCaseAnalysisContainerPtr case_analysis_container_ = nullptr;
    SdcDriveContainerPtr drive_container_ = nullptr;
    SdcDrivingCellContainerPtr driving_cell_container_ = nullptr;
    SdcFanoutLoadContainerPtr fanout_load_container_ = nullptr;
    SdcInputTransitionContainerPtr input_transition_container_ = nullptr;
    SdcLoadContainerPtr load_container_ = nullptr;
    SdcLogicContainerPtr logic_container_ = nullptr;
    SdcMaxAreaContainerPtr max_area_container_ = nullptr;
    SdcMaxCapacitanceContainerPtr max_capacitance_container_ = nullptr;
    SdcMaxFanoutContainerPtr max_fanout_container_ = nullptr;
    SdcMaxTransitionContainerPtr max_transition_container_ = nullptr;
    SdcMinCapacitanceContainerPtr min_capacitance_container_ = nullptr;
    SdcOperatingConditionsContainerPtr operating_conditions_container_ = nullptr;
    SdcPortFanoutNumberContainerPtr port_fanout_number_container_ = nullptr;
    SdcResistanceContainerPtr resistance_container_ = nullptr;
    SdcTimingDerateContainerPtr timing_derate_container_ = nullptr;
    SdcVoltageContainerPtr voltage_container_ = nullptr;
    SdcWireLoadMinBlockSizeContainerPtr wire_load_min_block_size_container_ = nullptr;
    SdcWireLoadModeContainerPtr wire_load_mode_container_ = nullptr;
    SdcWireLoadModelContainerPtr wire_load_model_container_ = nullptr;
    SdcWireLoadSelectionGroupContainerPtr wire_load_selection_group_container_ = nullptr;

    SdcVoltageAreaContainerPtr voltage_area_container_ = nullptr;
    SdcLevelShifterStrategyContainerPtr level_shifter_strategy_container_ = nullptr;
    SdcLevelShifterThresholdContainerPtr level_shifter_threshold_container_ = nullptr;
    SdcMaxDynamicPowerContainerPtr max_dynamic_power_container_ = nullptr;
    SdcMaxLeakagePowerContainerPtr max_leakage_power_container_ = nullptr;

    SdcAllClocksContainerPtr all_clocks_container_ = nullptr;
    SdcAllInputsContainerPtr all_inputs_container_ = nullptr;
    SdcAllRegistersContainerPtr all_registers_container_ = nullptr;
    SdcCurrentDesignContainerPtr design_container_ = nullptr;
    SdcGetCellsContainerPtr get_cells_container_ = nullptr;
    SdcGetClocksContainerPtr get_clocks_container_ = nullptr;
    SdcGetLibCellsContainerPtr get_lib_cells_container_ = nullptr;
    SdcGetLibPinsContainerPtr get_lib_pins_container_ = nullptr;
    SdcGetLibsContainerPtr get_libs_container_ = nullptr;
    SdcGetNetsContainerPtr get_nets_container_ = nullptr;
    SdcGetPinsContainerPtr get_pins_container_ = nullptr;
    SdcGetPortsContainerPtr get_ports_container_ = nullptr;

  private:
    ObjectId analysis_view_id_ = UNINIT_OBJECT_ID;

};
using SdcPtr = std::shared_ptr<Sdc>;

SdcPtr getSdc(ObjectId view_id);
SdcPtr getSdc(const std::string &view_name);

}
}


#endif // EDI_DB_TIMING_SDC_SDC_H_
