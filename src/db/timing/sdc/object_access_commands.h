/**
 * @file object_access_commands.h
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

#ifndef EDI_DB_TIMING_SDC_OBJECT_ACCESS_COMMANDS_H_
#define EDI_DB_TIMING_SDC_OBJECT_ACCESS_COMMANDS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "db/core/object.h"
#include "db/timing/sdc/command_get_set_property.h"
#include "db/timing/sdc/clock.h"

namespace open_edi {
namespace db {

//class AllClocks place holder

class AllInputs {
  public:
    AllInputs();

  private:
    ClockId clock_id_ = kInvalidClockId;

  public:
    COMMAND_GET_SET_VAR(clock_id, ClockId)
    COMMAND_GET_SET_FLAG(level_sensitive, LevelSensitive)
    COMMAND_GET_SET_FLAG(edge_triggered, EdgeTriggered)
};
using AllInputsPtr = std::shared_ptr<AllInputs>;

class AllOutputs {
  public:
    AllOutputs();

  private:
    ClockId clock_id_ = kInvalidClockId;

  public:
    COMMAND_GET_SET_VAR(clock_id, ClockId)
    COMMAND_GET_SET_FLAG(level_sensitive, LevelSensitive)
    COMMAND_GET_SET_FLAG(edge_triggered, EdgeTriggered)
};
using AllOutputsPtr = std::shared_ptr<AllOutputs>;

class AllRegisters {
  public:
    AllRegisters();

  private:
    std::vector<ClockId> clocks_;
    std::vector<ClockId> rise_clocks_;
    std::vector<ClockId> fall_clocks_;

  public:
    COMMAND_GET_SET_VAR(clocks, Clocks)
    COMMAND_GET_SET_VAR(rise_clocks, RiseClocks)
    COMMAND_GET_SET_VAR(fall_clocks, FallClocks)
    COMMAND_GET_SET_FLAG(no_hierarchy, NoHierarchy)
    COMMAND_GET_SET_FLAG(cells, Cells)
    COMMAND_GET_SET_FLAG(data_pins, DataPins)
    COMMAND_GET_SET_FLAG(clock_pins, ClockPins)
    COMMAND_GET_SET_FLAG(slave_clock_pins, SlaveClockPins)
    COMMAND_GET_SET_FLAG(async_pins, AsyncPins)
    COMMAND_GET_SET_FLAG(output_pins, OutputPins)
    COMMAND_GET_SET_FLAG(level_sensitive, LevelSensitive)
    COMMAND_GET_SET_FLAG(edge_triggered, EdgeTriggered)
    COMMAND_GET_SET_FLAG(master_slave, MasterSlave)
};
using AllRegistersPtr = std::shared_ptr<AllRegisters>;

enum class SwitchCurrentDesignResult : Bits8 { kSuccessToOriginCell = 0, kSuccessToNewCell, kFailedAsNewCellNotHier, kFailedAsNewCellIsInvalid, kUnknown };

class CurrentDesign {
  public:
    CurrentDesign();
    void init();

    SwitchCurrentDesignResult cd(const std::string &hier_cell_name);

  private:
    ObjectId cell_id_ = UNINIT_OBJECT_ID;

  public:
    COMMAND_GET_SET_VAR(cell_id, CellId)
};
using CurrentDesignPtr = std::shared_ptr<CurrentDesign>;

class GetCells {
  public:
    GetCells();

  private:
    std::vector<ObjectId> of_objects_;
    std::string patterns_ = ".*";

  public:
    COMMAND_GET_SET_VAR(of_objects, OfObjects)
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_FLAG(hierarchical, Hierarchical)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetCellsPtr = std::shared_ptr<GetCells>;

class GetClocks {
  public:
    GetClocks();

  private:
    std::string patterns_ = ".*";

  public:
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetClocksPtr = std::shared_ptr<GetClocks>;

class GetLibCells {
  public:
    GetLibCells();

  private:
    std::string patterns_ = ".*";

  public:
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetLibCellsPtr = std::shared_ptr<GetLibCells>;

class GetLibPins {
  public:
    GetLibPins();

  private:
    std::string patterns_ = ".*";

  public:
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetLibPinsPtr = std::shared_ptr<GetLibPins>;

class GetLibs {
  public:
    GetLibs();

  private:
    std::string patterns_ = ".*";

  public:
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetLibsPtr = std::shared_ptr<GetLibs>;

class GetNets {
  public:
    GetNets();

  private:
    std::vector<ObjectId> of_objects_;

  public:
    COMMAND_GET_SET_VAR(of_objects, OfObjects)
    COMMAND_GET_SET_FLAG(hierarchical, Hierarchical)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetNetsPtr = std::shared_ptr<GetNets>;

class GetPins {
  public:
    GetPins();

  private:
    std::string patterns_ = ".*";
    std::vector<ObjectId> of_objects_;

  public:
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_VAR(of_objects, OfObjects)
    COMMAND_GET_SET_FLAG(hierarchical, Hierarchical)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetPinsPtr = std::shared_ptr<GetPins>;

class GetPorts {
  public:
    GetPorts();

  private:
    std::string patterns_ = ".*";

  public:
    COMMAND_GET_SET_VAR(patterns, Patterns)
    COMMAND_GET_SET_FLAG(regexp, Regexp)
    COMMAND_GET_SET_FLAG(nocase, NoCase)
};
using GetPortsPtr = std::shared_ptr<GetPorts>;


}
}

#endif //EDI_DB_TIMING_SDC_OBJECT_ACCESS_COMMANDS_H_
