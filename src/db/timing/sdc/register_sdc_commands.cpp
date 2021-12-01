/**
 * @file register_sdc_commands.cpp
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

#include "db/timing/sdc/register_sdc_commands.h"
#include "db/timing/sdc/parse_sdc_commands.h"
#include "infra/command_manager.h"

namespace open_edi {
namespace db {

using Command = open_edi::infra::Command;
using CommandManager = open_edi::infra::CommandManager;
using OptionDataType = open_edi::infra::OptionDataType;

// general purpose commands
void registerSdcGeneralPurposeCommands(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        parseSdcCurrentInstance,  
        "current_instance", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("instance", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetHierarchySeparator,
        "set_hierarchy_separator", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("separator", OptionDataType::kString, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetUnits,
        "set_units", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-capacitance", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-resistance", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-time", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-voltage", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-current", OptionDataType::kString, false,"\n") +
        cmd_manager->createOption("-power", OptionDataType::kString, false, "\n")
    );
    assert(cmd);
}

// object access commands
void registerSdcObjectAccessCommands(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        parseSdcAllClocks,
        "all_clocks", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcAllInputs,
        "all_inputs", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-level_sensitive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-edge_triggered", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kStringList, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcAllOutputs,
        "all_outputs", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-level_sensitive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-edge_triggered", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcAllRegisters,
        "all_registers", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-no_hierarchy", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-hsc", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_clock", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_clock", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-cells", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-data_pins", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock_pins", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-slave_clock_pins", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-async_pins", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-output_pins", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-master_slave", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-level_sensitive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-edge_triggered", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcCurrentDesign,
        "current_design", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("design_name", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetDesigns,
        "get_designs", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("design_name", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetCells,
        "get_cells", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-hierarchical", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-of_objects", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetClocks,
        "get_clocks", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetLibCells,
        "get_lib_cells", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-hsc", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetLibPins,
        "get_libs_pins", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetLibs,
        "get_libs", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetNets,
        "get_nets", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-hierarchical", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-of_objects", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-hsc", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetPins,
        "get_pins", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-hierarchical", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-of_objects", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-hsc", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGetPorts,
        "get_ports", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-regexp", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-nocase", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("patterns", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);
}

// timing constraints
void registerSdcTimingConstraints(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        parseSdcCreateClock,
        "create_clock", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-period", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-name", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-waveform", OptionDataType::kDoubleList, false, "\n") +
        cmd_manager->createOption("port_pin_list", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-add", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcCreateGeneratedClock,
        "create_generated_clock", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-name", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-source", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-edges", OptionDataType::kIntList, false, "\n") +
        cmd_manager->createOption("-divide_by", OptionDataType::kInt, false, "\n") +
        cmd_manager->createOption("-multiply_by", OptionDataType::kInt, false, "\n") +
        cmd_manager->createOption("-edge_shift", OptionDataType::kDoubleList, false, "\n") +
        cmd_manager->createOption("-duty_cycle", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("-invert", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("port_pin_list", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-add", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-master_clock", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcGroupPath,
        "group_path", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-name", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-default", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-weight", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-fall_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetClockGatingCheck,
        "set_clock_gating_check", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-setup", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("-hold", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-high", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-low", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetClockGroups,
        "set_clock_groups", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-name", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-physically_exclusive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-logically_exclusive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-asynchronous", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-allow_paths", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-group", OptionDataType::kString, true, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetClockLatency,
        "set_clock_latency", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("delay", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-dynamic", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-source", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-early", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-late", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kStringList, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetSense,
        "set_sense", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-type", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-non_unate", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clocks", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-positive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-negative", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock_leaf", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-stop_propagation", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-pulse", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetClockTransition,
        "set_clock_transition", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("transition", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("clock_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetClockUncertainty,
        "set_clock_uncertainty", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("uncertainty", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-setup", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-hold", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetDataCheck,
        "set_data_check", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-setup", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-hold", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetDisableTiming,
        "set_disable_timing", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetFalsePath,
        "set_false_path", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-setup", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-hold", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-fall_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetIdealLatency,
        "set_ideal_latency", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetIdealNetwork,
        "set_ideal_network", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-no_propagate", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetIdealTransition,
        "set_ideal_transition", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetInputDelay,
        "set_input_delay", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-clock_fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-level_sensitive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-add_delay", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("delay_value", OptionDataType::kDouble,  true, "\n") +
        cmd_manager->createOption("-reference_pin", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("port_pin_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-network_latency_included", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-source_latency_included", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxDelay,
        "set_max_delay", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("delay_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList,  false, "\n") +
        cmd_manager->createOption("-through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-fall_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-ignore_clock_latency", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxTimeBorrow,
        "set_max_time_borrow", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("delay_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMinDelay,
        "set_min_delay", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("delay_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-fall_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-ignore_clock_latency", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMinPulseWidth,
        "set_min_pulse_width", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-low", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-high", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMulticyclePath,
        "set_multicycle_path", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("path_multiplier", OptionDataType::kInt, true, "\n") +
        cmd_manager->createOption("-setup", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-hold", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-start", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-end", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_to", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-rise_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-fall_from", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-fall_through", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-comment", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetOutputDelay,
        "set_output_delay", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-clock_fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-level_sensitive", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-add_delay", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("delay_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-reference_pin", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("port_pin_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-network_latency_included", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-source_latency_included", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetPropagatedClock,
        "set_propagated_clock", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);
}

// environment commands
void registerSdcEnvironmentCommands(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        parseSdcSetCaseAnalysis,
        "set_case_analysis", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kString, true, "\n") +
        cmd_manager->createOption("port_or_pin_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetDrive,
        "set_drive", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("resistance", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetDrivingCell,
        "set_driving_cell", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-lib_cell", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-library", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-pin", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-from_pin", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-dont_scale", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-no_design_rule", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-input_transition_rise", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("-input_transition_fall", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-clock_fall", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetFanoutLoad,
        "set_fanout_load", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetInputTransition,
        "set_input_transition", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("transition", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-clock_fall", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetLoad,
        "set_load", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-substract_pin_load", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-pin_load", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-wire_load", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("objects", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetLogicDc,
        "set_logic_dc", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetLogicOne,
        "set_logic_one", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetLogicZero,
        "set_logic_zero", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxArea,
        "set_max_area", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("area_value", OptionDataType::kDouble, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxCapacitance,
        "set_max_capacitance", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxFanout,
        "set_max_fanout", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("fanout_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxTransition,
        "set_max_transition", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("transition_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-clock_path", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMinCapacitance,
        "set_min_capacitance", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetOperatingConditions,
        "set_operating_conditions", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-analysis_type", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-library", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-max_library", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-object_list", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-min_library", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("condition", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetPortFanoutNumber,
        "set_port_fanout_number", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("fanout_number", OptionDataType::kInt, true, "\n") +
        cmd_manager->createOption("port_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetResistance,
        "set_resistance", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("net_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetTimingDerate,
        "set_timing_derate", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("derate_value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-rise", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-fall", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-early", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-late", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-static", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-dynamic", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-increment", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-clock", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-data", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-net_delay", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-cell_delay", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-cell_check", OptionDataType::kBoolNoValue, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetVoltage,
        "set_voltage", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("value", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-object_list", OptionDataType::kStringList, true, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kDouble, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetWireLoadMinBlockSize,
        "set_wire_load_min_block_size", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("size", OptionDataType::kDouble, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetWireLoadMode,
        "set_wire_load_mode", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("mode_name", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetWireLoadModel,
        "set_wire_load_model", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-name", OptionDataType::kString, true, "\n") +
        cmd_manager->createOption("-library", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetWireLoadSelectionGroup,
        "set_wire_load_selection_group", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-group_name", OptionDataType::kString, true, "\n") +
        cmd_manager->createOption("-library", OptionDataType::kStringList, false, "\n") +
        cmd_manager->createOption("-min", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("-max", OptionDataType::kBoolNoValue, false, "\n") +
        cmd_manager->createOption("object_list", OptionDataType::kStringList, false, "\n")
    );
    assert(cmd);
}

// multivoltage power commands
void registerSdcMultivoltagePowerCommands(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        parseSdcCreateVoltageArea,
        "create_voltage_area", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-name", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-coordinate", OptionDataType::kDoubleList, false, "\n") +
        cmd_manager->createOption("-guard_band_x", OptionDataType::kInt, false, "\n") +
        cmd_manager->createOption("-guard_band_y", OptionDataType::kInt, false, "\n") +
        cmd_manager->createOption("cell_list", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetLevelShifterStrategy,
        "set_level_shifter_strategy", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-rule", OptionDataType::kString, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetLevelShifterThreshold,
        "set_level_shifter_threshold", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-voltage", OptionDataType::kDouble, false, "\n") +
        cmd_manager->createOption("-percent", OptionDataType::kDouble, false, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxDynamicPower,
        "set_max_dynamic_power", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("power", OptionDataType::kDouble, true,"\n") +
        cmd_manager->createOption("-unit", OptionDataType::kString, false,"\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        parseSdcSetMaxLeakagePower,
        "set_max_leakage_power", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("power", OptionDataType::kDouble, true, "\n") +
        cmd_manager->createOption("-unit", OptionDataType::kString, false, "\n")
    );
    assert(cmd);
}

// internal commands
void registerSdcInternalCommands(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        readSdc,
        "read_sdc", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("-version", OptionDataType::kString, false, "\n") +
        cmd_manager->createOption("sdc_files", OptionDataType::kStringList, true, "\n")
    );
    assert(cmd);

    cmd = cmd_manager->createCommand(itp,
        writeSdc,
        "write_sdc", "\n",
        cmd_manager->createOption("-view", OptionDataType::kString, true, "\n") +
        cmd_manager->createOption("-version", OptionDataType::kInt, false, "\n") +
        cmd_manager->createOption("sdc_file", OptionDataType::kString, true, "\n")
    );
    assert(cmd);
}

// main register sdc commands manager
void registerSdcCommands(Tcl_Interp *itp) {
    registerSdcGeneralPurposeCommands(itp);
    registerSdcObjectAccessCommands(itp);
    registerSdcTimingConstraints(itp);
    registerSdcEnvironmentCommands(itp);
    registerSdcMultivoltagePowerCommands(itp);
    registerSdcInternalCommands(itp);
}

};
};
