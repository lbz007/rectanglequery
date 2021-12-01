/**
 * @file multivoltage_power_commands.h
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

#ifndef EDI_DB_TIMING_SDC_MULTIVOLTAGE_POWER_COMMANDS_H_
#define EDI_DB_TIMING_SDC_MULTIVOLTAGE_POWER_COMMANDS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "db/timing/sdc/command_get_set_property.h"
#include "db/timing/sdc/sdc_common.h"

namespace open_edi {
namespace db {

class BoxCoordinate {
  public:
    BoxCoordinate(float llx, float lly, float urx, float ury) : llx_(llx), lly_(lly), urx_(urx), ury_(ury) {}

  private:
    float llx_ = 0.0;
    float lly_ = 0.0;
    float urx_ = 0.0;
    float ury_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(llx, LLX)
    COMMAND_GET_SET_VAR(lly, LLY)
    COMMAND_GET_SET_VAR(urx, URX)
    COMMAND_GET_SET_VAR(ury, URY)
};
using BoxCoordinatePtr = std::shared_ptr<BoxCoordinate>;

class CreateVoltageArea {
  public:
    void add(const BoxCoordinate &box) { coordinates_.emplace_back(box); }

  private:
    std::string name_ = "";
    std::vector<BoxCoordinate> coordinates_;
    int guard_band_x_ = 0;
    int guard_band_y_ = 0;

  public:
    COMMAND_GET_SET_VAR(name, Name)
    COMMAND_GET_SET_VAR(coordinates, Coordinates)
    COMMAND_GET_SET_VAR(guard_band_x, GuardBandX)
    COMMAND_GET_SET_VAR(guard_band_y, GuardBandY)
};
using CreateVoltageAreaPtr = std::shared_ptr<CreateVoltageArea>;

class VoltageAreaContainerData {
  public:
    bool addToInst(const std::string &inst_name, CreateVoltageAreaPtr &voltage_area) { return ContainerDataAccess::addToInstValueMap(inst_voltage_area_, inst_name, voltage_area); }

  private:
    std::unordered_map<ObjectId, CreateVoltageAreaPtr> inst_voltage_area_;

  public:
    COMMAND_GET_SET_VAR(inst_voltage_area, InstVoltageArea)
};
using VoltageAreaContainerDataPtr = std::shared_ptr<VoltageAreaContainerData>;

enum class LevelShifterStrategy : Bits8 {kAll = 0, kLowToHigh, kHighToLow, kUnknown};
inline std::string toString(const LevelShifterStrategy &strategy) {
    switch (strategy) {
        case LevelShifterStrategy::kAll:
            return "all";
        case LevelShifterStrategy::kLowToHigh:
            return "low_to_high";
        case LevelShifterStrategy::kHighToLow:
            return "high_to_low";
        default:
            return "Unknown";
    }
}

class SetLevelShifterStrategy {
  public:
    bool setRule(const std::string &rule_name);

  private:
    LevelShifterStrategy rule_ = LevelShifterStrategy::kUnknown;

  public:
    COMMAND_GET_SET_VAR(rule, Rule)
};
using SetLevelShifterStrategyPtr = std::shared_ptr<SetLevelShifterStrategy>;

class SetLevelShifterThreshold {
  private:
    float voltage_ = 0.0;
    float percent_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(voltage, Voltage)
    COMMAND_GET_SET_VAR(percent, Percent)
};
using SetLevelShifterThresholdPtr = std::shared_ptr<SetLevelShifterThreshold>;

class SetMaxDynamicPower {
  public:
    void set(const float &value, const std::string &unit);

  private:
    float power_value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(power_value, PowerValue)
};
using SetMaxDynamicPowerPtr = std::shared_ptr<SetMaxDynamicPower>;

class MaxDynamicPowerContainerData {
  public:
    bool addToCurrentDesign(const ObjectId &cell_id, const SetMaxDynamicPower &power) { return ContainerDataAccess::addToCurrentDesignValueMap(dynamic_power_, cell_id, power); }

  private:
    std::unordered_map<ObjectId, SetMaxDynamicPower> dynamic_power_;

  public:
    COMMAND_GET_SET_VAR(dynamic_power, DynamicPower)
};
using MaxDynamicPowerContainerDataPtr = std::shared_ptr<MaxDynamicPowerContainerData>;

class SetMaxLeakagePower {
  public:
    void set(const float &value, const std::string &unit, const float &default_scale);

  private:
    float power_value_  = 0.0; // w

  public:
    COMMAND_GET_SET_VAR(power_value, PowerValue)
};
using SetMaxLeakagePowerPtr = std::shared_ptr<SetMaxLeakagePower>;

class MaxLeakagePowerContainerData {
  public:
    bool addToCurrentDesign(const ObjectId &design_cell_id, const SetMaxLeakagePower &power) { return ContainerDataAccess::addToCurrentDesignValueMap(leakage_power_, design_cell_id, power); }

  private:
    std::unordered_map<ObjectId, SetMaxLeakagePower> leakage_power_;

  public:
    COMMAND_GET_SET_VAR(leakage_power, LeakagePower)
};
using MaxLeakagePowerContainerDataPtr = std::shared_ptr<MaxLeakagePowerContainerData>;



}
}
#endif //EDI_DB_TIMING_SDC_MULTIVOLTAGE_POWER_COMMANDS_H_
