/**
 * @file environment_commands.h
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

#ifndef EDI_DB_TIMING_SDC_ENVIRONMENT_COMMANDS_H_
#define EDI_DB_TIMING_SDC_ENVIRONMENT_COMMANDS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "db/core/object.h"
#include "db/timing/timinglib/timinglib_wireload.h"
#include "db/timing/timinglib/timinglib_opcond.h"
#include "db/timing/sdc/command_get_set_property.h"
#include "db/timing/sdc/clock.h"
#include "db/timing/sdc/sdc_common.h"
#include "db/timing/sdc/base_container_data.h"

namespace open_edi {
namespace db {

using UInt32 = open_edi::util::UInt32;
using Bits8 = open_edi::util::Bits8;

enum class CaseValue : Bits8 { kZero = 0, kOne, kRise, kFall, kUnknown };
std::string toString(const CaseValue &value);
class SetCaseAnalysis {
  public:
    bool setValue(std::string& input);
    bool operator==(const SetCaseAnalysis &rhs) const;
    bool is_constant() const;

  private:
    CaseValue value_ = CaseValue::kUnknown;

  public:
    COMMAND_GET_SET_VAR(value, Value)
};
using SetCaseAnalysisPtr = std::shared_ptr<SetCaseAnalysis>;

class CaseAnalysisCotnainerData {
  public:
    bool add(const std::string &pin_name, const SetCaseAnalysis &case_analysis) { return ContainerDataAccess::addToPinValueMap(pin_to_case_analysis_, pin_name, case_analysis); }

  private:
    std::unordered_map<ObjectId, SetCaseAnalysis> pin_to_case_analysis_;

  public:
    COMMAND_GET_SET_VAR(pin_to_case_analysis, PinToCaseAnalysis)
};
using CaseAnalysisCotnainerDataPtr = std::shared_ptr<CaseAnalysisCotnainerData>;

class SetDrive {
  public:
    SetDrive();
    bool operator==(const SetDrive &rhs) const;

  private:
    float resistance_  = 0.0;

  public:
    COMMAND_GET_SET_VAR(resistance, Resistance)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetDrivePtr = std::shared_ptr<SetDrive>;

class DriveContainerData {
  public:
    bool add(const std::string &port_name, const SetDrivePtr &drive) { return ContainerDataAccess::addToPortValueMap(port_drives_, port_name, drive); }

  private:
    std::unordered_multimap<ObjectId, SetDrivePtr> port_drives_;

  public:
    COMMAND_GET_SET_VAR(port_drives, PortDrives)
    SPLIT_TWO_GROUP(SetDrive, SetDrivePtr, Rise, Fall, Min, Max)
};
using DriveContainerDataPtr = std::shared_ptr<DriveContainerData>;

class SetDrivingCellCondition {
  public:
    SetDrivingCellCondition();
    SetDrivingCellCondition(bool is_rise, bool is_min, bool no_design_rule=false, bool dont_scale=false) :
        rise_(is_rise), fall_(!is_rise), min_(is_min), max_(!is_min), no_design_rule_(no_design_rule), dont_scale_(dont_scale)
        {}
    bool operator==(const SetDrivingCellCondition &rhs) const;

  //private:
  //  ClockId clock_ = kInvalidClockId; // Deprecated

  public:
    //COMMAND_GET_SET_VAR(clock, Clock) // Deprecated
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
    COMMAND_GET_SET_FLAG(no_design_rule, NoDesignRule)
    COMMAND_GET_SET_FLAG(dont_scale, DontScale) // Disappeared in newer version
    //COMMAND_GET_SET_FLAG(clock_fall, ClockFall) // Deprecated
};
using SetDrivingCellConditionPtr = std::shared_ptr<SetDrivingCellCondition>;

class SetDrivingCell {
  public:
    using Base = SetDrivingCellCondition;
    bool checkArc(TCell *tcell, const std::string &from_tterm_name, const std::string &to_tterm_name);
    bool operator==(const SetDrivingCell &rhs) const; //TODO use function hasSameCondition instead

  private:
    SetDrivingCellCondition condition_;
    float input_transition_rise_ = 0.0;
    float input_transition_fall_ = 0.0;
    ObjectId from_tterm_ = UNINIT_OBJECT_ID;
    ObjectId to_tterm_ = UNINIT_OBJECT_ID;

  public:
    COMMAND_GET_SET_VAR(condition, Condition)
    COMMAND_GET_SET_VAR(input_transition_rise, InputTransitionRise)
    COMMAND_GET_SET_VAR(input_transition_fall, InputTransitionFall)
    COMMAND_GET_SET_VAR(from_tterm, FromTTerm)
    COMMAND_GET_SET_VAR(to_tterm, ToTTerm)

  public:
    //CONDITION_GET_SET_VAR(Clock) // Deprecated
    CONDITION_GET_SET_FLAG(Rise)
    CONDITION_GET_SET_FLAG(Fall)
    CONDITION_GET_SET_FLAG(Min)
    CONDITION_GET_SET_FLAG(Max)
    CONDITION_GET_SET_FLAG(NoDesignRule)
    CONDITION_GET_SET_FLAG(DontScale) // Disappeared in newer version
    //CONDITION_GET_SET_FLAG(ClockFall) // Deprecated
};
using SetDrivingCellPtr = std::shared_ptr<SetDrivingCell>;

class DrivingCellData {
  public:
    bool add(const std::string &port_name, const SetDrivingCellPtr &driving_cell) { return ContainerDataAccess::addToPortValueMap(port_driving_cell_, port_name, driving_cell); }

  private:
    std::unordered_multimap<ObjectId, SetDrivingCellPtr> port_driving_cell_;

  public:
    COMMAND_GET_SET_VAR(port_driving_cell, PortDrivingCell)
  
  public:
    SPLIT_TWO_GROUP(SetDrivingCell, SetDrivingCellPtr, Rise, Fall, Min, Max)
};
using DrivingCellDataPtr = std::shared_ptr<DrivingCellData>;

class SetFanoutLoad {
  public:
    bool operator==(const SetFanoutLoad &rhs) const;

  private:
    float value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(value, Value)
};
using SetFanoutLoadPtr = std::shared_ptr<SetFanoutLoad>;

class FanoutLoadContainerData {
  public:
    bool add(const std::string &port_name, const SetFanoutLoad &fanout) { return ContainerDataAccess::addToPortValueMap(port_fanout_load_, port_name, fanout); }

  private:
    std::unordered_map<ObjectId, SetFanoutLoad> port_fanout_load_;

  public:
    COMMAND_GET_SET_VAR(port_fanout_load, PortFanoutLoad)
};
using FanoutLoadContainerDataPtr = std::shared_ptr<FanoutLoadContainerData>;

class SetInputTransition {
  public:
    SetInputTransition();
    bool operator==(const SetInputTransition &rhs) const;

  private:
    float transition_ = 0.0;
    //ClockId clock_ = kInvalidClockId; // Deprecated

  public:
    COMMAND_GET_SET_VAR(transition, Transition)
    //COMMAND_GET_SET_VAR(clock, Clock) // Deprecated
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
    //COMMAND_GET_SET_FLAG(clock_fall, ClockFall) // Deprecated
};
using SetInputTransitionPtr = std::shared_ptr<SetInputTransition>;

class InputTransitionData {
  public:
    bool add(const std::string &port_name, const SetInputTransitionPtr &input_transition) { return ContainerDataAccess::addToPortValueMap(port_input_transitions_, port_name, input_transition); }

  private:
    std::unordered_multimap<ObjectId, SetInputTransitionPtr> port_input_transitions_;

  public:
    COMMAND_GET_SET_VAR(port_input_transitions, PortInputTransitions)
    SPLIT_TWO_GROUP(SetInputTransition, SetInputTransitionPtr, Rise, Fall, Min, Max)
};
using InputTransitionDataPtr = std::shared_ptr<InputTransitionData>;

class SetLoad {
  public:
    SetLoad();
    void checkFlags();
    bool operator==(const SetLoad &rhs) const;

  private:
    float cap_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(cap, Cap)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
    COMMAND_GET_SET_FLAG(rise, Rise) // Not in sdc 2.1, but in newer version
    COMMAND_GET_SET_FLAG(fall, Fall) // Not in sdc 2.1, but in newer version
    COMMAND_GET_SET_FLAG(substract_pin_load, SubstractPinLoad)
    COMMAND_GET_SET_FLAG(pin_load, PinLoad) //Default is pin_load if wire_load is not specified
    COMMAND_GET_SET_FLAG(wire_load, WireLoad)
};
using SetLoadPtr = std::shared_ptr<SetLoad>;

class LoadContainerData {
  public:
    bool addToPort(const std::string &port_name, const SetLoadPtr &load) { return ContainerDataAccess::addToPortValueMap(port_load_caps_, port_name, load); }
    bool addToNet(const std::string &net_name, const SetLoadPtr &load) { return ContainerDataAccess::addToNetValueMap(net_load_caps_, net_name, load); }

  private:
    std::unordered_multimap<ObjectId, SetLoadPtr> port_load_caps_;
    std::unordered_multimap<ObjectId, SetLoadPtr> net_load_caps_;

  public:
    COMMAND_GET_SET_VAR(port_load_caps, PortLoadCaps)
    COMMAND_GET_SET_VAR(net_load_caps, NetLoadCaps)
    SPLIT_TWO_GROUP(SetLoad, SetLoadPtr, Min, Max, Rise, Fall)
};
using LoadContainerDataPtr = std::shared_ptr<LoadContainerData>;

enum class LogicValue : Bits8 {kZero = 0, kOne, kDontCare, kUnknown};
std::string toString(const LogicValue &value);
class SetLogic {
  public:
    bool operator==(const SetLogic &rhs) const;
    bool is_constant() const;

  private:
    LogicValue value_ = LogicValue::kUnknown;

  public:
    COMMAND_GET_SET_VAR(value, Value)
};
using SetLogicPtr = std::shared_ptr<SetLogic>;

class LogicContainerData {
  public:
    bool add(const std::string &port_name, const SetLogic &logic) { return ContainerDataAccess::addToPortValueMap(port_logic_value_, port_name, logic); }

  private:
    std::unordered_map<ObjectId, SetLogic> port_logic_value_;

  public:
    COMMAND_GET_SET_VAR(port_logic_value, PortLogicValue)
};
using LogicContainerDataPtr = std::shared_ptr<LogicContainerData>;

class SetMaxArea {
  public:
    bool operator==(const SetMaxArea &rhs) const;

  private:
    float area_value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(area_value, AreaValue)
};
using SetMaxAreaPtr= std::shared_ptr<SetMaxArea>;

class MaxAreaContainerData {
  public:
    bool add(const ObjectId &cell_id, const SetMaxArea &area) { return ContainerDataAccess::addToCurrentDesignValueMap(design_max_area_, cell_id, area); }

  private:
    std::unordered_map<ObjectId, SetMaxArea> design_max_area_;

  public:
    COMMAND_GET_SET_VAR(design_max_area, DesignMaxArea)
};
using MaxAreaContainerDataPtr = std::shared_ptr<MaxAreaContainerData>;

class SetMaxCapacitance {
  public:
    bool operator==(const SetMaxCapacitance &rhs) const;

  private:
    float cap_value_ = 0.0;
    //TODO add more vars according to other references

  public:
    COMMAND_GET_SET_VAR(cap_value, CapValue)
};
using SetMaxCapacitancePtr = std::shared_ptr<SetMaxCapacitance>;

class MaxCapacitanceContainerData {
  public:
    bool addToPin(const std::string &pin_name, const SetMaxCapacitancePtr &cap) { return ContainerDataAccess::addToPinValueMap(pin_max_caps_, pin_name, cap); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetMaxCapacitancePtr &cap) { return ContainerDataAccess::addToCurrentDesignValueMap(design_max_caps_, cell_id, cap); }

  private:
    //TODO not support capacitance on clock object and term object yet
    std::unordered_map<ObjectId, SetMaxCapacitancePtr> pin_max_caps_;
    std::unordered_map<ObjectId, SetMaxCapacitancePtr> design_max_caps_;

  public:
    COMMAND_GET_SET_VAR(pin_max_caps, PinMaxCaps)
    COMMAND_GET_SET_VAR(design_max_caps, DesignMaxCaps)
};
using MaxCapacitanceContainerDataPtr = std::shared_ptr<MaxCapacitanceContainerData>;

class SetMaxFanout {
  public:
    bool operator==(const SetMaxFanout &rhs) const;

  private:
    float fanout_value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(fanout_value, FanoutValue)
};
using SetMaxFanoutPtr = std::shared_ptr<SetMaxFanout>;

class MaxFanoutContainerData {
  public:
    bool addToPort(const std::string &port_name, const SetMaxFanout &fanout) { return ContainerDataAccess::addToPortValueMap(port_max_fanout_, port_name, fanout); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetMaxFanout &fanout) { return ContainerDataAccess::addToCurrentDesignValueMap(design_max_fanout_, cell_id, fanout); }

  private:
    std::unordered_map<ObjectId, SetMaxFanout> port_max_fanout_;
    std::unordered_map<ObjectId, SetMaxFanout> design_max_fanout_;

  public:
    COMMAND_GET_SET_VAR(port_max_fanout, PortMaxFanout)
    COMMAND_GET_SET_VAR(design_max_fanout, DesignMaxFanout)
};
using MaxFanoutContainerDataPtr = std::shared_ptr<MaxFanoutContainerData>;

class SetMaxTransition {
  public:
    SetMaxTransition();
    bool operator==(const SetMaxTransition &rhs) const;

  private:
    float transition_value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(transition_value, TransitionValue)
    COMMAND_GET_SET_FLAG(clock_path, ClockPath)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
};
using SetMaxTransitionPtr = std::shared_ptr<SetMaxTransition>;

class MaxTransitionContainerData : public BaseContainerData {
  public:
    //clock path transition will be set to pins
    bool addToPin(const std::string &pin_name, const SetMaxTransitionPtr &transition) { return ContainerDataAccess::addToPinValueMap(pin_max_transition_, pin_name, transition); }
    bool addToClock(const ClockId &clock_id, const SetMaxTransitionPtr &transition) { return ContainerDataAccess::addToClockValueMap(clock_max_transition_, clock_id, transition); }
    bool addToCurrentDesign(const ObjectId &design_cell_id, const SetMaxTransitionPtr &transition) { return ContainerDataAccess::addToCurrentDesignValueMap(design_max_transition_, design_cell_id, transition); }

  private:
    std::unordered_multimap<ObjectId, SetMaxTransitionPtr> pin_max_transition_;
    std::unordered_multimap<ClockId, SetMaxTransitionPtr> clock_max_transition_;
    std::unordered_multimap<ObjectId, SetMaxTransitionPtr> design_max_transition_;

  public:
    COMMAND_GET_SET_VAR(pin_max_transition, PinMaxTransition)
    COMMAND_GET_SET_VAR(clock_max_transition, ClockMaxTransition)
    COMMAND_GET_SET_VAR(design_max_transition, DesignMaxTransition)
    SPLIT_ONE_GROUP(SetMaxTransition, SetMaxTransitionPtr, Rise, Fall)
};
using MaxTransitionContainerDataPtr = std::shared_ptr<MaxTransitionContainerData>;

class SetMinCapacitance {
  public:
    bool operator==(const SetMinCapacitance &rhs) const;

  private:
    float cap_value_ = 0.0;
    //TODO add more vars according to other references

  public:
    COMMAND_GET_SET_VAR(cap_value, CapValue)
};
using SetMinCapacitancePtr = std::shared_ptr<SetMinCapacitance>;

class MinCapacitanceContainerData {
  public:
    bool addToPin(const std::string &pin_name, const SetMinCapacitancePtr &cap) { return ContainerDataAccess::addToPinValueMap(pin_min_caps_, pin_name, cap); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetMinCapacitancePtr &cap) { return ContainerDataAccess::addToCurrentDesignValueMap(design_min_caps_ ,cell_id, cap); }

  private:
    //TODO not support capacitance on clock object and term object yet
    std::unordered_map<ObjectId, SetMinCapacitancePtr> pin_min_caps_;
    std::unordered_map<ObjectId, SetMinCapacitancePtr> design_min_caps_;

  public:
    COMMAND_GET_SET_VAR(pin_min_caps, PinMinCaps)
    COMMAND_GET_SET_VAR(design_min_caps, DesignMinCaps)
};
using MinCapacitanceContainerDataPtr = std::shared_ptr<MinCapacitanceContainerData>;

enum class AnalysisType {kSingle = 0, kBcWc, kOnChipVariation, kUnknown};
std::string toString(const AnalysisType &type);
class SetOperatingConditions {
  public:
    bool setValue(std::string& input);
    bool operator==(const SetOperatingConditions &rhs) const;

  private:
    AnalysisType analysis_type_ = AnalysisType::kUnknown;
    ObjectId min_condition_ = UNINIT_OBJECT_ID;
    ObjectId max_condition_ = UNINIT_OBJECT_ID;
    ObjectId condition_ = UNINIT_OBJECT_ID;

  public:
    COMMAND_GET_SET_VAR(analysis_type, AnalysisType)
    COMMAND_GET_SET_VAR(min_condition, MinCondition)
    COMMAND_GET_SET_VAR(max_condition, MaxCondition)
    COMMAND_GET_SET_VAR(condition, Condition)
};
using SetOperatingConditionsPtr = std::shared_ptr<SetOperatingConditions>;

class OperatingConditionsContainerData {
  public:
    bool addToPort(const std::string &port_name, const SetOperatingConditionsPtr &condition) { return ContainerDataAccess::addToPortValueMap(port_operating_conditions_, port_name, condition); }
    bool addToInst(const std::string &inst_name, const SetOperatingConditionsPtr &condition) { return ContainerDataAccess::addToInstValueMap(inst_operating_conditions_, inst_name, condition); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetOperatingConditionsPtr &condition) { return ContainerDataAccess::addToCurrentDesignValueMap(cell_operating_conditions_, cell_id, condition); }

  private:
    std::unordered_map<ObjectId, SetOperatingConditionsPtr> port_operating_conditions_;
    std::unordered_map<ObjectId, SetOperatingConditionsPtr> inst_operating_conditions_;
    std::unordered_map<ObjectId, SetOperatingConditionsPtr> cell_operating_conditions_;

  public:
    COMMAND_GET_SET_VAR(port_operating_conditions, PortOperatingConditions);
    COMMAND_GET_SET_VAR(inst_operating_conditions, InstOperatingConditions);
    COMMAND_GET_SET_VAR(cell_operating_conditions, CellOperatingConditions);
};
using OperatingConditionsContainerDataPtr = std::shared_ptr<OperatingConditionsContainerData>;

class SetPortFanoutNumber {
  public:
    bool operator==(const SetPortFanoutNumber &rhs) const;

  private:
    UInt32 fanout_number_ = 0;

  public:
    //TODO -min -max not support in sdc2.1
    COMMAND_GET_SET_VAR(fanout_number, FanoutNumber)
};
using SetPortFanoutNumberPtr = std::shared_ptr<SetPortFanoutNumber>;

class PortFanoutNumberContainerData {
  public:
    bool add(const std::string &port_name, const SetPortFanoutNumber &fanout) { return ContainerDataAccess::addToPortValueMap(port_fanout_number_, port_name, fanout); }

  private:
    std::unordered_multimap<ObjectId, SetPortFanoutNumber> port_fanout_number_;

  public:
    COMMAND_GET_SET_VAR(port_fanout_number, PortFanoutNumber)
};
using PortFanoutNumberContainerDataPtr = std::shared_ptr<PortFanoutNumberContainerData>;

class SetResistance {
  public:
    SetResistance();
    bool operator==(const SetResistance &rhs) const;

  private:
    float value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(value, Value)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetResistancePtr = std::shared_ptr<SetResistance>;

class ResistanceContainerData {
  public:
    bool add(const std::string &net_name, const SetResistancePtr &res) { return ContainerDataAccess::addToNetValueMap(net_resistance_, net_name, res); }

  private:
    std::unordered_multimap<ObjectId, SetResistancePtr> net_resistance_;

  public:
    COMMAND_GET_SET_VAR(net_resistance, NetResistance)
    SPLIT_ONE_GROUP(SetResistance, SetResistancePtr, Min, Max)
};
using ResistanceContainerDataPtr = std::shared_ptr<ResistanceContainerData>;

class SetTimingDerate {
  public:
    SetTimingDerate();
    void checkFlags();
    bool operator==(const SetTimingDerate &rhs) const;

  private:
    float derate_value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(derate_value, DerateValue)
    COMMAND_GET_SET_FLAG(early, Early)
    COMMAND_GET_SET_FLAG(late, Late) // Must specify one of early or late
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
    COMMAND_GET_SET_FLAG(data, Data)
    COMMAND_GET_SET_FLAG(clock, Clock)
    COMMAND_GET_SET_FLAG(static_type, StaticType)
    COMMAND_GET_SET_FLAG(dynamic_type, DynamicType)
    COMMAND_GET_SET_FLAG(net_delay, NetDelay) // TODO split net_delay/cell_delay/cell_check?
    COMMAND_GET_SET_FLAG(cell_delay, CellDelay)
    COMMAND_GET_SET_FLAG(cell_check, CellCheck)
    COMMAND_GET_SET_FLAG(increment, Increment)
};
using SetTimingDeratePtr = std::shared_ptr<SetTimingDerate>;

class TimingDerateContainerData {
  public:
    bool addToNet(const std::string &net_name, const SetTimingDeratePtr &derate) { return ContainerDataAccess::addToNetValueMap(net_timing_derate_, net_name, derate); }
    bool addToInst(const std::string &inst_name, const SetTimingDeratePtr &derate) { return ContainerDataAccess::addToInstValueMap(inst_timing_derate_, inst_name, derate); }
    bool addToTCell(AnalysisCorner *corner, const std::string &tcell_name, const SetTimingDeratePtr &derate) { return ContainerDataAccess::addToTCellValueMap(tcell_timing_derate_, corner, tcell_name, derate); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetTimingDeratePtr &derate) { return ContainerDataAccess::addToCurrentDesignValueMap(design_timing_derate_, cell_id, derate); }

  private:
    std::unordered_multimap<ObjectId, SetTimingDeratePtr> net_timing_derate_;
    std::unordered_multimap<ObjectId, SetTimingDeratePtr> inst_timing_derate_;
    std::unordered_multimap<ObjectId, SetTimingDeratePtr> tcell_timing_derate_;
    std::unordered_map<ObjectId, SetTimingDeratePtr> design_timing_derate_;

  public:
    COMMAND_GET_SET_VAR(net_timing_derate, NetTimingDerate)
    COMMAND_GET_SET_VAR(inst_timing_derate, InstTimingDerate)
    COMMAND_GET_SET_VAR(tcell_timing_derate, TCellTimingDerate)
    SPLIT_FOUR_GROUP(SetTimingDerate, SetTimingDeratePtr, Rise, Fall, Min, Max, Data, Clock, StaticType, DynamicType)
};
using TimingDerateContainerDataPtr = std::shared_ptr<TimingDerateContainerData>;

class SetVoltage {
  public:
    bool operator==(const SetVoltage &rhs) const;

  private:
    float max_case_voltage_ = 0.0;
    float min_case_voltage_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(max_case_voltage, MaxCaseVoltage)
    COMMAND_GET_SET_VAR(min_case_voltage, MinCaseVoltage)
};
using SetVoltagePtr = std::shared_ptr<SetVoltage>;

class VoltageContainerData {
  public:
    bool addToPowerNet(const std::string &power_net_name, const SetVoltagePtr &voltage) { return ContainerDataAccess::addToPowerNetValueMap(power_net_voltage_, power_net_name, voltage); }
    bool addToPowerPin(const std::string &power_pin_name, const SetVoltagePtr &voltage) { return ContainerDataAccess::addToPinValueMap(power_pin_voltage_, power_pin_name, voltage); }

  private:
    std::unordered_multimap<ObjectId, SetVoltagePtr> power_net_voltage_;
    std::unordered_multimap<ObjectId, SetVoltagePtr> power_pin_voltage_; //Not support in sdc2.1

  public:
    COMMAND_GET_SET_VAR(power_net_voltage, PowerNetVoltage)
    COMMAND_GET_SET_VAR(power_pin_voltage, PowerPinVoltage)
};
using VoltageContainerDataPtr = std::shared_ptr<VoltageContainerData>;

class SetWireLoadMinBlockSize {
  public:
    bool operator==(const SetWireLoadMinBlockSize &rhs) const;

  private:
    float block_size_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(block_size, BlockSize)
};
using SetWireLoadMinBlockSizePtr = std::shared_ptr<SetWireLoadMinBlockSize>;

class WireLoadMinBlockSizeContainerData {
  public:
    bool add(const ObjectId &design_cell_id, const SetWireLoadMinBlockSize &size) {  return ContainerDataAccess::addToCurrentDesignValueMap(design_min_block_size_, design_cell_id, size); }

  private:
    std::unordered_map<ObjectId, SetWireLoadMinBlockSize> design_min_block_size_;

  public:
    COMMAND_GET_SET_VAR(design_min_block_size, DesignMinBlockSize)
};
using WireLoadMinBlockSizeContainerDataPtr = std::shared_ptr<WireLoadMinBlockSizeContainerData>;

class SetWireLoadMode {
  public:
    bool set(const std::string &mode_name);
    bool operator==(const SetWireLoadMode &rhs) const;

  private:
    WireLoadMode mode_ = WireLoadMode::kUnknown;

  public:
    COMMAND_GET_SET_VAR(mode, Mode)
};
using SetWireLoadModePtr = std::shared_ptr<SetWireLoadMode>;

class WireLoadModeContainerData {
  public:
    bool add(const ObjectId &design_cell_id, const SetWireLoadMode &mode) { return ContainerDataAccess::addToCurrentDesignValueMap(design_wire_load_mode_, design_cell_id, mode); }

  private:
    std::unordered_map<ObjectId, SetWireLoadMode> design_wire_load_mode_;

  public:
    COMMAND_GET_SET_VAR(design_wire_load_mode, DesignWireLoadMode)
};
using WireLoadModeContainerDataPtr = std::shared_ptr<WireLoadModeContainerData>;

class SetWireLoadModel {
  public:
    SetWireLoadModel();
    bool operator==(const SetWireLoadModel &rhs) const;

  private:
    ObjectId wire_load_ = UNINIT_OBJECT_ID;

  public:
    COMMAND_GET_SET_VAR(wire_load, WireLoad)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetWireLoadModelPtr = std::shared_ptr<SetWireLoadModel>;

class WireLoadModelContainerData {
  public:
    bool addToCell(const std::string &cell_name, const SetWireLoadModelPtr &model) { return ContainerDataAccess::addToCellValueMap(cell_wire_load_model_, cell_name, model); }
    bool addToPin(const std::string &port_name, const SetWireLoadModelPtr &model) { return ContainerDataAccess::addToPortValueMap(pin_wire_load_model_, port_name, model); }

  private:
    std::unordered_multimap<ObjectId, SetWireLoadModelPtr> cell_wire_load_model_;
    std::unordered_multimap<ObjectId, SetWireLoadModelPtr> pin_wire_load_model_;

  public:
    COMMAND_GET_SET_VAR(cell_wire_load_model, CellWireLoadModel)
    COMMAND_GET_SET_VAR(pin_wire_load_model, PinWireLoadModel)
    SPLIT_ONE_GROUP(SetWireLoadModel, SetWireLoadModelPtr, Min, Max)
};
using WireLoadModelContainerDataPtr = std::shared_ptr<WireLoadModelContainerData>;

class SetWireLoadSelectionGroup {
  public:
    SetWireLoadSelectionGroup();
    bool operator==(const SetWireLoadSelectionGroup &rhs) const;

  private:
    ObjectId selection_;

  public:
    COMMAND_GET_SET_VAR(selection, Selection)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetWireLoadSelectionGroupPtr = std::shared_ptr<SetWireLoadSelectionGroup>;

class WireLoadSelectionGroupContainerData {
  public:
    bool addToCurrentDesign(const ObjectId &design_cell_id, const SetWireLoadSelectionGroupPtr &group) { return ContainerDataAccess::addToCurrentDesignValueMap(cell_wire_selection_group_, design_cell_id, group); }
    bool addToInst(const std::string &inst_name, const SetWireLoadSelectionGroupPtr &group) { return ContainerDataAccess::addToInstValueMap(inst_wire_selection_group_, inst_name, group); }

  private:
    std::unordered_map<ObjectId, SetWireLoadSelectionGroupPtr> cell_wire_selection_group_;
    std::unordered_map<ObjectId, SetWireLoadSelectionGroupPtr> inst_wire_selection_group_;

  public:
    COMMAND_GET_SET_VAR(cell_wire_selection_group, CellWireSelectionGroup)
    COMMAND_GET_SET_VAR(inst_wire_selection_group, InstWireSelectionGroup)
    SPLIT_ONE_GROUP(SetWireLoadSelectionGroup, SetWireLoadSelectionGroupPtr, Min, Max)
};
using WireLoadSelectionGroupContainerDataPtr = std::shared_ptr<WireLoadSelectionGroupContainerData>;


}
}
#endif // EDI_DB_TIMING_SDC_ENVIRONMENT_COMMANDS_H_
