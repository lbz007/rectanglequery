/**
 * @file parse_sdc_commands.cpp
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

#include <boost/algorithm/string.hpp>
#include "db/core/db.h"
#include "db/timing/sdc/parse_sdc_commands.h"
#include "db/timing/sdc/sdc.h"
#include "db/timing/timinglib/analysis_view.h"
#include "db/timing/timinglib/analysis_mode.h"
#include "db/timing/timinglib/timinglib_cell.h"
#include "db/timing/timinglib/libset.h"
#include "db/timing/timinglib/timinglib_lib.h"
#include "infra/command_manager.h"

namespace open_edi {
namespace db {

using Command = open_edi::infra::Command;
using CommandManager = open_edi::infra::CommandManager;

AnalysisView* getOrCreateDefaultView() {
    Timing *timing_db = getTimingLib();
    if (!timing_db) {
        message->issueMsg("SDC", 1, kError);
        return nullptr;
    }
    std::string default_name = "default";
    auto view = timing_db->getAnalysisView(default_name);
    if (!view) {
        AnalysisMode *mode = timing_db->createAnalysisMode(default_name);
        if (!mode) {
            message->issueMsg("SDC", 2, kError, default_name.c_str());
            return nullptr;
        }
        auto corner = timing_db->createAnalysisCorner(default_name);
        if (!corner) {
            message->issueMsg("SDC", 3, kError, default_name.c_str());
            return nullptr;
        }
        view = timing_db->createAnalysisView(default_name);
        if (!view) {
            message->issueMsg("SDC", 4, kError, default_name.c_str());
            return nullptr;
        }
        mode->addAnalysisView(view);
        corner->addAnalysisView(view);
        view->setAnalysisMode(mode->getId());
        view->setAnalysisCorner(corner->getId());
        view->setActive(true);
        view->setSetup(true);
        view->setHold(true);
        view->createSdc();
        timing_db->addActiveSetupView(view->getId());
        timing_db->addActiveHoldView(view->getId());
        message->info("Create default view successfully in sdc parser.\n");
    }
    return view;
}

AnalysisView* getOrCreateViewForSdc(const std::string &analysis_view_name) {
    if (analysis_view_name == "default") {
        return getOrCreateDefaultView();
    }
    Timing *timing_db = getTimingLib();
    if (!timing_db) {
        message->issueMsg("SDC", 1, kError);
        return nullptr;
    }
    AnalysisView *view  = timing_db->getAnalysisView(analysis_view_name);
    if (!view) {
        message->issueMsg("SDC", 6, kError, analysis_view_name.c_str());
        return nullptr;
    }
    return view;
}

SdcPtr getSdcFromCmd(Command* cmd, const std::string &command_line) {
    std::string analysis_view_name = "default";
    if (cmd->isOptionSet("-view")) {
        bool res = cmd->getOptionValue("-view", analysis_view_name);
        if (!res) {
            message->issueMsg("SDC", 7, kError, command_line.c_str());
            return nullptr;
        }
    } else {
        message->issueMsg("SDC", 8, kError, command_line.c_str());
    }
    AnalysisView* view = getOrCreateViewForSdc(analysis_view_name);
    if (!view) {
        message->issueMsg("SDC", 9, kError, analysis_view_name.c_str(), command_line.c_str());
        return nullptr;
    }
    return view->getSdc();
}


// For object name, tool search order : Clock > TCell > TTerm > Cell > Inst > Term > Pin > Net

// general purpose commands parser

int parseSdcCurrentInstance(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    }
    auto container = sdc->getCurrentInstanceContainer();
    auto inst = container->getData();
    std::string dir = "";
    if (cmd->isOptionSet("instance")) {
        bool res = cmd->getOptionValue("instance", dir);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-instance", command_line.c_str());
            return TCL_ERROR;
        }
    }
    const auto &design_container = sdc->getCurrentDesignContainer();
    SwitchInstResult result = inst->cd(design_container->getDesignId(), dir);
    switch (result) {
        case SwitchInstResult::kSuccessToCurrentDesign :
            message->info("Current instance is current design : %s\n", (design_container->getDesignName()).c_str());
            break;
        case SwitchInstResult::kSuccessToOriginInst :
            message->info("%s\n", (container->getInstName()).c_str());
            break;
        case SwitchInstResult::kSuccessToNewInst :
            message->info("Switch to new instance : %s\n", (container->getInstName()).c_str());
            break;
        case SwitchInstResult::kFailedAsOriginInstNotHier :
            message->info("Current instance %s is no-hierachical instance.\n", (container->getInstName()).c_str());
            break;
        case SwitchInstResult::kFailedAsNewInstNotHier :
            message->info("Can not switch to no-hierachical instance.\n");
            break;
        case SwitchInstResult::kFailedAsNewInstIsInvalid :
            message->info("Can not switch to %s.\n", dir.c_str());
            break;
        case SwitchInstResult::kFailedAsNewInstNotInCurrentDesign :
            message->info("Can not switch to instance that not in current design %s.\n", (design_container->getDesignName()).c_str());
            break;
        case SwitchInstResult::kUnknown :
        default :
            message->issueMsg("SDC", 26, kError, command_line.c_str(), (design_container->getDesignName()).c_str(), (container->getInstName()).c_str());
    }

    return TCL_OK;
}

int parseSdcSetUnits(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    TUnits* Tunits = getMainLibertyUnit();
    if (!Tunits) {
        message->issueMsg("SDC", 33, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getUnitsContainer();
    auto units = container->getData();
    units->setLibertyUnits(Tunits);
    const auto &check_unit = [&cmd, &command_line, &units](const std::string &option, const auto &func) ->bool {
        std::string option_value = "";
        bool res = cmd->getOptionValue(option.c_str(), option_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, option.c_str(), command_line.c_str());
            return false;
        }
        return func(*units, option_value);
    };
    std::unordered_map<std::string, std::function<bool(SetUnits &, const std::string &)> > options = {
        {"-capacitance", &SetUnits::setAndCheckCapacitance},
        {"-resistance", &SetUnits::setAndCheckResistance},
        {"-time", &SetUnits::setAndCheckTime},
        {"-voltage", &SetUnits::setAndCheckVoltage},
        {"-current", &SetUnits::setAndCheckCurrent},
        {"-power", &SetUnits::setAndCheckPower},
    };
    bool success = true;
    for (const auto &option_to_func : options) {
        const std::string &option = option_to_func.first;
        const auto &func = option_to_func.second;
        if (!(cmd->isOptionSet(option.c_str()))) {
            continue;
        }
        if (!check_unit(option, func)) {
            message->issueMsg("SDC", 11, kError, option.c_str(), command_line.c_str());
            success = false;
        }
    }
    if (!success) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

int parseSdcSetHierarchySeparator(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getHierarchySeparatorContainer();
    auto separator = container->getData();
    if (!(cmd->isOptionSet("separator"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("separator")) {
        std::string separator_str = "";
        bool res = cmd->getOptionValue("separator", separator_str);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "separator", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = separator->setAndCheck(separator_str);
        if (!success) {
            message->issueMsg("SDC", 11, kError, "separator", command_line.c_str());
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

// object access commands
int parseSdcAllClocks(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto &container = sdc->getAllClocksContainer();
    const auto &clock_container = sdc->getClockContainer();
    container->setData(clock_container);
    std::vector<std::string> clock_names;
    container->get_all_clock_names(clock_names);
    std::string output = "";
    output.reserve(10000);
    for (const auto &name : clock_names) {
        output.append(name).append(" ");
    }
    message->info("%s\n", output.c_str());
    return TCL_OK;
}

int parseSdcAllInputs(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-level_sensitive") and cmd->isOptionSet("-edge_triggered"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-level_sensitive")) {
        bool level_sensitive = false;
        bool res = cmd->getOptionValue("-level_sensitive", level_sensitive);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-level_sensitive", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", level_sensitive);
    }
    if (cmd->isOptionSet("-edge_triggered")) {
        bool edge_triggered = false;
        bool res = cmd->getOptionValue("-edge_triggered", edge_triggered);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-edge_triggered", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", edge_triggered);
    }

    if (cmd->isOptionSet("-clock")) {
        std::vector<std::string> clock;
        bool res = cmd->getOptionValue("-clock", clock);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& clock_name : clock) {

            message->info("get value %s \n", clock_name.c_str());
        }
    }
    return TCL_OK;
}

int parseSdcAllOutputs(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-level_sensitive") and cmd->isOptionSet("-edge_triggered"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-level_sensitive")) {
        bool level_sensitive = false;
        bool res = cmd->getOptionValue("-level_sensitive", level_sensitive);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-level_sensitive", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", level_sensitive);
    }

    if (cmd->isOptionSet("-edge_triggered")) {
        bool edge_triggered = false;
        bool res = cmd->getOptionValue("-edge_triggered", edge_triggered);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-edge_triggered", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", edge_triggered);
    }

    if (cmd->isOptionSet("-clock")) {
        std::vector<std::string> clock;
        bool res = cmd->getOptionValue("-clock", clock);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& clock_name : clock) {
            message->info("get value %s \n", clock_name.c_str());
        }
    }
    return TCL_OK;
}

int parseSdcAllRegisters(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (cmd->isOptionSet("-no_hierarchy")) {
        bool no_hierarchy = false;
        bool res = cmd->getOptionValue("-no_hierarchy", no_hierarchy);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-no_hierarchy", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", no_hierarchy);
    }
    if (cmd->isOptionSet("-cells")) {
        bool cells = false;
        bool res = cmd->getOptionValue("-cells", cells);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-cells", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", cells);
    }
    if (cmd->isOptionSet("-data_pins")) {
        bool data_pins = false;
        bool res = cmd->getOptionValue("-data_pins", data_pins);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-data_pins", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", data_pins);
    }
    if (cmd->isOptionSet("-clock_pins")) {
        bool clock_pins = false;
        bool res = cmd->getOptionValue("-clock_pins", clock_pins);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock_pins", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", clock_pins);
    }
    if (cmd->isOptionSet("-slave_clock_pins")) {
        bool slave_clock_pins = false;
        bool res = cmd->getOptionValue("-slave_clock_pins", slave_clock_pins);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-slave_clock_pins", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", slave_clock_pins);
    }
    if (cmd->isOptionSet("-async_pins")) {
        bool async_pins = false;
        bool res = cmd->getOptionValue("-async_pins", async_pins);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-async_pins", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", async_pins);
    }
    if (cmd->isOptionSet("-output_pins")) {
        bool output_pins = false;
        bool res = cmd->getOptionValue("-output_pins", output_pins);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-output_pins", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", output_pins);
    }
    if (cmd->isOptionSet("-level_sensitive")) {
        bool level_sensitive = false;
        bool res = cmd->getOptionValue("-level_sensitive", level_sensitive);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-level_sensitive", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", level_sensitive);
    }
    if (cmd->isOptionSet("-edge_triggered")) {
        bool edge_triggered = false;
        bool res = cmd->getOptionValue("-edge_triggered", edge_triggered);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-edge_triggered", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", edge_triggered);
    }
    if (cmd->isOptionSet("-master_slave")) {
        bool master_slave = false;
        bool res = cmd->getOptionValue("-master_slave", master_slave);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-master_slave", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", master_slave);
    }
    if (cmd->isOptionSet("-hsc")) {
        std::string hsc = "";
        bool res = cmd->getOptionValue("-hsc", hsc);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hsc", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", hsc.c_str());
    }
    if (cmd->isOptionSet("-clock")) {
        std::vector<std::string> clock;
        bool res = cmd->getOptionValue("-clock", clock);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& clock_name : clock) {
            message->info("get value %s \n", clock_name.c_str());
        }
    }

    if (cmd->isOptionSet("-rise_clock")) {
        std::vector<std::string> rise_clock;
        bool res = cmd->getOptionValue("-rise_clock", rise_clock);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_clock", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& rise_clock_name : rise_clock) {
            message->info("get value %s \n", rise_clock_name.c_str());
        }
    }
    if (cmd->isOptionSet("-fall_clock")) {
        std::vector<std::string> fall_clock;
        bool res = cmd->getOptionValue("-fall_clock", fall_clock);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_clock", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& fall_clock_name : fall_clock) {
            message->info("get value %s \n", fall_clock_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcCurrentDesign(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getCurrentDesignContainer();
    auto current_design = container->getData();
    std::string hier_cell_name = "";
    if (cmd->isOptionSet("design_name")) {
        bool res = cmd->getOptionValue("design_name", hier_cell_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "design_name", command_line.c_str());
            return TCL_ERROR;
        }
    }
    SwitchCurrentDesignResult result = current_design->cd(hier_cell_name);
    auto inst_container = sdc->getCurrentInstanceContainer();
    switch (result) {
        case SwitchCurrentDesignResult::kSuccessToOriginCell :
            message->info("%s\n", (container->getDesignName()).c_str());
            break;
        case SwitchCurrentDesignResult::kSuccessToNewCell : {
            auto inst = inst_container->getData();
            inst->cd(container->getDesignId(), "");
            message->info("Switch to new cell : %s\n", (container->getDesignName()).c_str());
            break;
        }
        case SwitchCurrentDesignResult::kFailedAsNewCellNotHier :
            message->info("Can not switch to no-hierachical cell : %s\n", hier_cell_name.c_str());
            break;
        case SwitchCurrentDesignResult::kFailedAsNewCellIsInvalid :
            message->info("Can not switch to cell : %s\n", hier_cell_name.c_str());
            break;
        case SwitchCurrentDesignResult::kUnknown :
        default :
            message->issueMsg("SDC", 26, kError, command_line.c_str(), (container->getDesignName()).c_str(), (inst_container->getInstName()).c_str());
    }

    return TCL_OK;
}

int parseSdcGetDesigns(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    std::string hier_cell_name = "";
    if (cmd->isOptionSet("design_name")) {
        bool res = cmd->getOptionValue("design_name", hier_cell_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "design_name", command_line.c_str());
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

int parseSdcGetCells(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if(!((cmd->isOptionSet("patterns") and !(cmd->isOptionSet("-of_objects"))) or 
        (cmd->isOptionSet("-of_objects") and !(cmd->isOptionSet("-of_objects"))) or 
        !(cmd->isOptionSet("patterns")))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-hsc")) {
        std::string hsc = "";
        bool res = cmd->getOptionValue("-hsc", hsc);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hsc", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", hsc.c_str());
    }
    if (cmd->isOptionSet("-hierarchical")) {
        bool hierarchical = false;
        bool res = cmd->getOptionValue("-hierarchical", hierarchical);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hierarchical", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", hierarchical);
    }
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-of_objects")) {
        std::vector<std::string> of_objects;
        bool res = cmd->getOptionValue("-of_objects", of_objects);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-of_objects", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& of_objects_name : of_objects) {
            message->info("get value %s \n", of_objects_name.c_str());
        }
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcGetClocks(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcGetLibCells(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("patterns"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-hsc")) {
        std::string hsc = "";
        bool res = cmd->getOptionValue("-hsc", hsc);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hsc", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", hsc.c_str());
    }
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcGetLibPins(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("patterns"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcGetLibs(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }
    return TCL_OK;
}

int parseSdcGetNets(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!((cmd->isOptionSet("patterns") and !(cmd->isOptionSet("-of_objects"))) or 
        (cmd->isOptionSet("-of_objects") and !(cmd->isOptionSet("-of_objects"))) or 
        !(cmd->isOptionSet("patterns")))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-hsc")) {
        std::string hsc = "";
        bool res = cmd->getOptionValue("-hsc", hsc);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hsc", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", hsc.c_str());
    }
    if (cmd->isOptionSet("-hierarchical")) {
        bool hierarchical = false;
        bool res = cmd->getOptionValue("-hierarchical", hierarchical);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hierarchical", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", hierarchical);
    }
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-of_objects")) {
        std::vector<std::string> of_objects;
        bool res = cmd->getOptionValue("-of_objects", of_objects);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-of_objects", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& of_objects_name : of_objects) {
            message->info("get value %s \n", of_objects_name.c_str());
        }
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcGetPins(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (cmd->isOptionSet("-hsc")) {
        std::string hsc = "";
        bool res = cmd->getOptionValue("-hsc", hsc);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hsc", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %s \n", hsc.c_str());
    }
    if (cmd->isOptionSet("-hierarchical")) {
        bool hierarchical = false;
        bool res = cmd->getOptionValue("-hierarchical", hierarchical);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hierarchical", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", hierarchical);
    }
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-of_objects")) {
        std::vector<std::string> of_objects;
        bool res = cmd->getOptionValue("-of_objects", of_objects);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-of_objects", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& of_objects_name : of_objects) {
            message->info("get value %s \n", of_objects_name.c_str());
        }
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }

    return TCL_OK;
}

int parseSdcGetPorts(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (cmd->isOptionSet("-nocase")) {
        bool nocase = false;
        bool res = cmd->getOptionValue("-nocase", nocase);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-nocase", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", nocase);
    }
    if (cmd->isOptionSet("-regexp")) {
        bool regexp = false;
        bool res = cmd->getOptionValue("-regexp", regexp);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-regexp", command_line.c_str());
            return TCL_ERROR;
        }
        message->info("get value %d \n", regexp);
    }
    if (cmd->isOptionSet("-patterns")) {
        std::vector<std::string> patterns;
        bool res = cmd->getOptionValue("-patterns", patterns);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-patterns", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& patterns_name : patterns) {
            message->info("get value %s \n", patterns_name.c_str());
        }
    }
    return TCL_OK;
}

// multivoltage power commands manager
int parseSdcCreateVoltageArea(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!((cmd->isOptionSet("-name") and cmd->isOptionSet("cell_list")))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getVoltageAreaContainer();
    auto container_data = container->getData();
    CreateVoltageAreaPtr voltage_area = std::make_shared<CreateVoltageArea>();
    if (cmd->isOptionSet("-name")) {
        std::string name = "";
        bool res = cmd->getOptionValue("-name", name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        voltage_area->setName(name);
    }
    if (cmd->isOptionSet("-coordinate")) {
        std::vector<double> coordinates;
        bool res = cmd->getOptionValue("-coordinate", coordinates);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-coordinate", command_line.c_str());
            return TCL_ERROR;
        }
        if (coordinates.size() % 4 != 0) {
            message->issueMsg("SDC", 11, kError, "-coordinate", command_line.c_str());
            return TCL_ERROR;
        }
        for (size_t index=0; index<coordinates.size(); index += 4) {
            voltage_area->add(BoxCoordinate(coordinates[index], coordinates[index+1], coordinates[index+2], coordinates[index+3]));
        }
    }
    if (cmd->isOptionSet("-guard_band_x")) {
        int guard_band_x;
        bool res = cmd->getOptionValue("-guard_band_x", guard_band_x);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-guard_band_x", command_line.c_str());
            return TCL_ERROR;
        }
        voltage_area->setGuardBandX(guard_band_x);
    }
    if (cmd->isOptionSet("-guard_band_y")) {
        int guard_band_y;
        bool res = cmd->getOptionValue("-guard_band_y", guard_band_y);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-guard_band_y", command_line.c_str());
            return TCL_ERROR;
        }
        voltage_area->setGuardBandY(guard_band_y);
    }
    if (cmd->isOptionSet("cell_list")) { //The "cell" in sdc means instance
        std::vector<std::string> inst_list;
        bool res = cmd->getOptionValue("cell_list", inst_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "cell_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& inst_name : inst_list) {
            container_data->addToInst(inst_name, voltage_area);
        }
    }

    return TCL_OK;
}

int parseSdcSetLevelShifterStrategy(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-rule"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getLevelShifterStrategyContainer();
    SetLevelShifterStrategy strategy;
    if (cmd->isOptionSet("-rule")) {
        std::string rule = "";
        bool res = cmd->getOptionValue("-rule", rule);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rule", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = strategy.setRule(rule);
        if (!success) {
            message->issueMsg("SDC", 11, kError, "-rule", command_line.c_str());
        }
    }
    container->setData(strategy);
    return TCL_OK;
}

int parseSdcSetLevelShifterThreshold(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-voltage"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getLevelShifterThresholdContainer();
    auto container_data = container->getData();
    if (cmd->isOptionSet("-voltage")) {
        double voltage = 0;
        bool res = cmd->getOptionValue("-voltage", voltage);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-voltage", command_line.c_str());
            return TCL_ERROR;
        }
        container_data->setVoltage(voltage);
    }
    if (cmd->isOptionSet("-percent")) {
        double percent = 0;
        bool res = cmd->getOptionValue("-percent", percent);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-percent", command_line.c_str());
            return TCL_ERROR;
        }
        container_data->setPercent(percent);
    }

    return TCL_OK;
}

int parseSdcSetMaxDynamicPower(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("power"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxDynamicPowerContainer();
    auto container_data = container->getData();
    SetMaxDynamicPower dynamic_power;
    double power = 0;
    if (cmd->isOptionSet("power")) {
        bool res = cmd->getOptionValue("power", power);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "power", command_line.c_str());
            return TCL_ERROR;
        }
    }
    std::string unit = "W";
    if (cmd->isOptionSet("-unit")) {
        bool res = cmd->getOptionValue("-unit", unit);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-unit", command_line.c_str());
            return TCL_ERROR;
        }
    }
    dynamic_power.set(power, unit);
    const auto &design_container = sdc->getCurrentDesignContainer();
    const auto &design_cell_id = design_container->getDesignId();
    container_data->addToCurrentDesign(design_cell_id, dynamic_power);
    return TCL_OK;
}

int parseSdcSetMaxLeakagePower(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("power"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    SetMaxLeakagePower power;
    double power_value = 0;
    if (cmd->isOptionSet("power")) {
        bool res = cmd->getOptionValue("power", power_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "power", command_line.c_str());
            return TCL_ERROR;
        }
    }
    std::string unit = "";
    if (cmd->isOptionSet("-unit")) {
        bool res = cmd->getOptionValue("-unit", unit);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-unit", command_line.c_str());
            return TCL_ERROR;
        }
    }
    const auto &sdc_units = sdc->getUnitsContainer();
    const float default_scale = sdc_units->getSdcPowerUnits();
    power.set(static_cast<float>(power_value), unit, default_scale);
    auto container = sdc->getMaxLeakagePowerContainer();
    auto container_data = container->getData();
    const auto &design_container = sdc->getCurrentDesignContainer();
    const auto &design_cell_id = design_container->getDesignId();
    container_data->addToCurrentDesign(design_cell_id, power);
    return TCL_OK;
}

// timing constraints commands
int parseSdcCreateClock(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-period") and (cmd->isOptionSet("-name") or cmd->isOptionSet("port_pin_list")))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-add") and !cmd->isOptionSet("-name")) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);

    CreateClockPtr create_clock = std::make_shared<CreateClock>();
    ClockPtr clock = std::make_shared<Clock>();
    if (cmd->isOptionSet("-name")) {
        std::string name = "";
        bool res = cmd->getOptionValue("-name", name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        clock->setName(name);
    }
    if (cmd->isOptionSet("-period")) {
        double period = 0.0;
        bool res = cmd->getOptionValue("-period", period);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-period", command_line.c_str());
            return TCL_ERROR;
        }
        clock->setPeriod(period);
    }
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        create_clock->setComment(comment);
    }
    if (cmd->isOptionSet("-waveform")) {
        std::vector<double> waveform_list;
        bool res = cmd->getOptionValue("-waveform", waveform_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-waveform", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &waveform : waveform_list) {
            clock->addWaveform(static_cast<float>(waveform));
        }
    }
    if (cmd->isOptionSet("-add")) {
        create_clock->setAdd();
    }
    std::vector<std::string> port_pin_list;
    if (cmd->isOptionSet("port_pin_list")) { //sdc2.1 not support net object source
        bool res = cmd->getOptionValue("port_pin_list", port_pin_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_pin_list", command_line.c_str());
            return TCL_ERROR;
        }
        if (clock->getName() == "" and !port_pin_list.empty()) {
            clock->setName(port_pin_list.front());
        }
    } else {
        clock->setVirtual();
    }
    ClockId clock_id = container->getClockId(clock->getName());
    bool clock_already_created = false;
    if (clock_id != kInvalidClockId) {
        clock_already_created = true;
    }
    if (port_pin_list.empty()) {
        if (clock_already_created) {
            container_data->updateClock(clock, create_clock, clock_id);
        } else {
            container_data->addVirtualClock(clock, create_clock);
        }
    } else {
        std::unordered_set<ObjectId> pin_ids;
        for (const std::string &pin_name : port_pin_list) {
            const auto &pin = getPinByFullName(pin_name);
            if (!pin) {
                message->issueMsg("SDC", 13, kError, (clock->getName()).c_str(), pin_name.c_str(), command_line.c_str());
                return TCL_ERROR;
            }
            pin_ids.emplace(pin->getId());
        }
        if (clock_already_created) {
            container_data->updateClock(clock, create_clock, clock_id);
        } else {
            container_data->addClock(clock, create_clock);
        }
        for (const auto &pin_id : pin_ids) {
            container_data->addClockPin(pin_id, clock, create_clock->isAdd());
        }
    }

    return TCL_OK;
}

int parseSdcCreateGeneratedClock(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-source") and cmd->isOptionSet("port_pin_list") and
        (!cmd->isOptionSet("-multiply_by")) and cmd->isOptionSet("-divide_by") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-add") and !(cmd->isOptionSet("-name") and cmd->isOptionSet("-master_clock"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockContainer();
    auto container_data = container->getData();

    CreateGeneratedClockPtr generated_clock = std::make_shared<CreateGeneratedClock>();
    ClockPtr clock = std::make_shared<Clock>();
    if (cmd->isOptionSet("-name")) {
        std::string name = "";
        bool res = cmd->getOptionValue("-name", name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        clock->setName(name);
    }
    if (cmd->isOptionSet("-source")) {
        std::vector<std::string> source_list;
        bool res = cmd->getOptionValue("-source", source_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-source", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &pin_name : source_list) {
            bool success = generated_clock->addSourceMasterPin(pin_name);
            if (!success) {
                message->issueMsg("SDC", 14, kError, pin_name.c_str(), "-source", command_line.c_str());
            }
        }
    }
    if (cmd->isOptionSet("-edges")) {
        std::vector<int> edges;
        bool res = cmd->getOptionValue("-edges", edges);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-edges", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &edge : edges) {
            generated_clock->addEdge(static_cast<float>(edge));
        }
    }
    if (cmd->isOptionSet("-divide_by")) {
        int divide_by = 0;
        bool res = cmd->getOptionValue("-divide_by", divide_by);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-divide_by", command_line.c_str());
            return TCL_ERROR;
        }
        generated_clock->setDividedBy(divide_by);
    }
    if (cmd->isOptionSet("-multiply_by")) {
        int multiply_by = 0;
        bool res = cmd->getOptionValue("-multiply_by", multiply_by);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-multiply_by", command_line.c_str());
            return TCL_ERROR;
        }
        generated_clock->setMultiplyBy(multiply_by);
    }
    if (cmd->isOptionSet("-edge_shift")) {
        std::vector<double> edge_shift_list;
        bool res = cmd->getOptionValue("-edge_shift", edge_shift_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-edge_shift", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &edge_shift : edge_shift_list) {
            generated_clock->addEdgeShift(static_cast<float>(edge_shift));
        }
    }
    if (cmd->isOptionSet("-duty_cycle")) {
        double duty_cycle = 0.0;
        bool res = cmd->getOptionValue("-duty_cycle", duty_cycle);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-duty_cycle", command_line.c_str());
            return TCL_ERROR;
        }
        generated_clock->setDutyCycle(static_cast<float>(duty_cycle));
    }
    if (cmd->isOptionSet("-invert")) {
        generated_clock->setInvert();
    }
    if (cmd->isOptionSet("-add")) {
        generated_clock->setAdd();
    }
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        generated_clock->setComment(comment);
    }
    std::vector<std::string> port_pin_list;
    if (cmd->isOptionSet("port_pin_list")) { //sdc2.1 not support net object source
        bool res = cmd->getOptionValue("port_pin_list", port_pin_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_pin_list", command_line.c_str());
            return TCL_ERROR;
        }
        if (clock->getName() == "" and !port_pin_list.empty()) {
            clock->setName(port_pin_list.front());
        }
    } else {
        clock->setVirtual();
    }
    if (cmd->isOptionSet("-master_clock")) {
        std::string master_clock;
        bool res = cmd->getOptionValue("-master_clock", master_clock);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-master_clock", command_line.c_str());
            return TCL_ERROR;
        }
        const ClockId &master_clock_id = container->getClockId(master_clock);
        if (master_clock_id == kInvalidClockId) {
            message->issueMsg("SDC", 15, kError, master_clock.c_str(), command_line.c_str());
            return TCL_ERROR;
        }
        generated_clock->setMasterClock(master_clock_id);
    }
    ClockId clock_id = container->getClockId(clock->getName());
    bool clock_already_created = false;
    if (clock_id != kInvalidClockId) {
        clock_already_created = true;
    }
    if (port_pin_list.empty()) {
        if (clock_already_created) {
            container_data->updateClock(clock, generated_clock, clock_id);
        } else {
            container_data->addVirtualClock(clock, generated_clock);
        }
    } else {
        std::unordered_set<ObjectId> pin_ids;
        for (const std::string &pin_name : port_pin_list) {
            const auto &pin = getPinByFullName(pin_name);
            if (!pin) {
                message->issueMsg("SDC", 13, kError, (clock->getName()).c_str(), pin_name.c_str(), command_line.c_str());
                return TCL_ERROR;
            }
            pin_ids.emplace(pin->getId());
        }
        if (clock_already_created) {
            container_data->updateClock(clock, generated_clock, clock_id);
        } else {
            container_data->addClock(clock, generated_clock);
        }
        for (const auto &pin_id : pin_ids) {
            container_data->addClockPin(pin_id, clock, generated_clock->isAdd());
        }
    }

    return TCL_OK;
}

template<typename U>
int parseFromThroughTo(U &path_ptr, Command *cmd, const SdcClockContainerPtr &clock_container, const std::string &command_line) {
    std::vector<std::string> from_list;
    PathNodesPtr &from = path_ptr->getFrom();
    if (cmd->isOptionSet("-from")) {
        bool res = cmd->getOptionValue("-from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-from", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("-rise_from")) {
        bool res = cmd->getOptionValue("-rise_from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_from", command_line.c_str());
            return TCL_ERROR;
        }
        from->setRise();
    }
    if (cmd->isOptionSet("-fall_from")) {
        bool res = cmd->getOptionValue("-fall_from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_from", command_line.c_str());
            return TCL_ERROR;
        }
        from->setFall();
    }
    from->checkFlags();
    for (const auto &from_object_name : from_list) {
        const ClockId &clock_id = clock_container->getClockId(from_object_name);
        if (clock_id != kInvalidClockId) {
            from->addClock(clock_id);
            continue;
        }
        bool success = from->addInst(from_object_name);
        if (success) {
            continue;
        }
        success = from->addPin(from_object_name);
        if (success) {
            continue;
        }
        message->issueMsg("SDC", 16, kError, from_object_name, command_line.c_str());
    }
    std::vector<std::string> to_list;
    PathNodesPtr &to = path_ptr->getTo();
    if (cmd->isOptionSet("-to")) {
        bool res = cmd->getOptionValue("-to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-to", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("-rise_to")) {
        bool res = cmd->getOptionValue("-rise_to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_to", command_line.c_str());
            return TCL_ERROR;
        }
        to->setRise();
    }
    if (cmd->isOptionSet("-fall_to")) {
        bool res = cmd->getOptionValue("-fall_to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_to", command_line.c_str());
            return TCL_ERROR;
        }
        to->setFall();
    }
    for (const auto &to_object_name : to_list) {
        const ClockId &clock_id = clock_container->getClockId(to_object_name);
        if (clock_id != kInvalidClockId) {
            to->addClock(clock_id);
            continue;
        }
        bool success = to->addInst(to_object_name);
        if (success) {
            continue;
        }
        success = to->addPin(to_object_name);
        if (success) {
            continue;
        }
        message->issueMsg("SDC", 16, kError, to_object_name.c_str(), command_line.c_str());
    }
    to->checkFlags();

    auto parse_through = [&path_ptr, &command_line](const std::vector<std::pair<std::string, int> > &through_lists, bool rise = false, bool fall = false) {
        for (const std::pair<std::string, int> &through_list_to_id : through_lists) {
            const std::string &through_list_str = through_list_to_id.first;
            const int index = through_list_to_id.second;
            std::vector<std::string> through_list;
            boost::split(through_list, through_list_str, boost::is_any_of(" "), boost::token_compress_on);
            PathThroughNodesPtr through = std::make_shared<PathThroughNodes>();
            if (rise) {
                through->setRise();
            }
            if (fall) {
                through->setFall();
            }
            through->checkFlags();
            through->setIndex(index);
            for (const auto &through_object_name : through_list) {
                bool success = through->addInst(through_object_name);
                if (success) {
                    continue;
                }
                success = through->addPin(through_object_name);
                if (success) {
                    continue;
                }
                success = through->addNet(through_object_name);
                if (!success) {
                    message->issueMsg("SDC", 16, kError, through_object_name, command_line.c_str());
                }
            }
            path_ptr->addThrough(through);
        }
    };
    if (cmd->isOptionSet("-through")) {
        std::vector<std::pair<std::string, int> > through_lists;
        bool res = cmd->getOptionValue("-through", through_lists);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-through", command_line.c_str());
            return TCL_ERROR;
        }
        parse_through(through_lists);
    }
    if (cmd->isOptionSet("-rise_through")) {
        std::vector<std::pair<std::string, int> > rise_through_lists;
        bool res = cmd->getOptionValue("-rise_through", rise_through_lists);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_through", command_line.c_str());
            return TCL_ERROR;
        }
        constexpr bool rise = true;
        constexpr bool fall = false;
        parse_through(rise_through_lists, rise, fall);
    }
    if (cmd->isOptionSet("-fall_through")) {
        std::vector<std::pair<std::string, int> > fall_through_lists;
        bool res = cmd->getOptionValue("-fall_through", fall_through_lists);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_through", command_line.c_str());
            return TCL_ERROR;
        }
        constexpr bool rise = false;
        constexpr bool fall = true;
        parse_through(fall_through_lists, rise, fall);
    }
    path_ptr->sortThroughs();

    return TCL_OK;
}

int parseSdcGroupPath(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( (cmd->isOptionSet("-name") and (!cmd->isOptionSet("-default"))) or
        (cmd->isOptionSet("-default") and (!cmd->isOptionSet("-name"))) or
        (!cmd->isOptionSet("-name")) and
        (cmd->isOptionSet("-from") != cmd->isOptionSet("-rise_from") != cmd->isOptionSet("-fall_from")))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getGroupPathContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    GroupPathPtr group_path = std::make_shared<GroupPath>();
    container_data->add(group_path);
    const auto &clock_container = sdc->getClockContainer();

    if (cmd->isOptionSet("-name")) {
        std::string name = "";
        bool res = cmd->getOptionValue("-name", name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        group_path->setName(name);
    }
    if (cmd->isOptionSet("-default")) {
        group_path->setDefaultValue();
    }
    if (cmd->isOptionSet("-weight")) {
        double weight = 0.0;
        bool res = cmd->getOptionValue("-weight", weight);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-weight", command_line.c_str());
            return TCL_ERROR;
        }
        group_path->setWeight(static_cast<float>(weight));
    }
    parseFromThroughTo(group_path, cmd, clock_container, command_line);
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        group_path->setComment(comment);
    }
    return TCL_OK;
}

int parseSdcSetClockGatingCheck(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(  (cmd->isOptionSet("-setup") or cmd->isOptionSet("-hold") or cmd->isOptionSet("-high") or cmd->isOptionSet("-low")) and
            (!(cmd->isOptionSet("-high") and cmd->isOptionSet("-low"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockGatingCheckContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetClockGatingCheckPtr clock_gating_check = std::make_shared<SetClockGatingCheck>();

    if (cmd->isOptionSet("-setup")) {
        double setup = 0.0;
        bool res = cmd->getOptionValue("-setup", setup);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-setup", command_line.c_str());
            return TCL_ERROR;
        }
        clock_gating_check->setSetup(setup);
    }
    if (cmd->isOptionSet("-hold")) {
        double hold = 0.0;
        bool res = cmd->getOptionValue("-hold", hold);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-hold", command_line.c_str());
            return TCL_ERROR;
        }
        clock_gating_check->setHold(hold);
    }
    if (cmd->isOptionSet("-rise")) {
        clock_gating_check->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        clock_gating_check->setFall();
    }
    if (cmd->isOptionSet("-high")) {
        clock_gating_check->setHigh();
    }
    if (cmd->isOptionSet("-low")) {
        clock_gating_check->setLow();
    }
    std::vector<SetClockGatingCheckPtr> clock_gating_checks;
    container_data->split(clock_gating_checks, clock_gating_check);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();

        for (const auto &object_name : object_list) {
            const ClockId &clock_id = clock_container->getClockId(object_name);
            for (const auto &check : clock_gating_checks) {
                if (clock_id != kInvalidClockId) {
                    container_data->addToClock(clock_id, check);
                    continue;
                }
                bool success = container_data->addToInst(object_name, check);
                if (success) {
                    continue;
                }
                success = container_data->addToPin(object_name, check);
                if (success) {
                    continue;
                }
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    } else {
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        for (const auto &check : clock_gating_checks) {
            container_data->addToCurrentDesign(design_cell_id, check);
        }
    }

    return TCL_OK;
}

int parseSdcSetClockGroups(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(  cmd->isOptionSet("-physically_exclusive") != cmd->isOptionSet("-logically_exclusive") !=
            cmd->isOptionSet("-asynchronous") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockGroupsContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);

    SetClockGroupsPtr clock_groups = std::make_shared<SetClockGroups>();
    if (cmd->isOptionSet("-name")) {
        std::string name = "";
        bool res = cmd->getOptionValue("-name", name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        clock_groups->setName(name);
    }
    if (cmd->isOptionSet("-physically_exclusive")) {
        bool physically_exclusive = false;
        bool res = cmd->getOptionValue("-physically_exclusive", physically_exclusive);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-physically_exclusive", command_line.c_str());
            return TCL_ERROR;
        }
        clock_groups->setRelationType(RelationshipType::kPhysicallyExclusive);
    }
    if (cmd->isOptionSet("-logically_exclusive")) {
        bool logically_exclusive = false;
        bool res = cmd->getOptionValue("-logically_exclusive", logically_exclusive);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-logically_exclusive", command_line.c_str());
            return TCL_ERROR;
        }
        clock_groups->setRelationType(RelationshipType::kLogicallyExclusive);
    }
    if (cmd->isOptionSet("-asynchronous")) {
        bool asynchronous = false;
        bool res = cmd->getOptionValue("-asynchronous", asynchronous);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-asynchronous", command_line.c_str());
            return TCL_ERROR;
        }
        clock_groups->setRelationType(RelationshipType::kAsynchronous);
    }
    if (cmd->isOptionSet("-allow_paths")) {
        bool allow_paths = false;
        bool res = cmd->getOptionValue("-allow_paths", allow_paths);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-allow_paths", command_line.c_str());
            return TCL_ERROR;
        }
        clock_groups->setRelationType(RelationshipType::kAsynchronousAllowPaths);
    }
    const auto &clock_container = sdc->getClockContainer();
    if (cmd->isOptionSet("-group")) {
        std::vector<std::pair<std::string, int> > groups;
        bool res = cmd->getOptionValue("-group", groups);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-group", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &group: groups) {
            const std::string &clock_names_str = group.first;
            std::vector<std::string> clock_names;
            boost::split(clock_names, clock_names_str, boost::is_any_of(" "), boost::token_compress_on);
            std::unordered_set<ClockId> clock_ids;
            for (const auto &clock_name : clock_names) {
                const ClockId &clock_id = clock_container->getClockId(clock_name);
                if (clock_id == kInvalidClockId) {
                    message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
                    continue;
                }
                clock_ids.emplace(clock_id);
            }
            clock_groups->addToGroups(std::move(clock_ids));
        }
    }
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        clock_groups->setComment(comment);
    }
    container_data->addGroup(clock_groups);
    //TODO no need to fetch all clocks if group number is more than one.
    std::vector<ClockId> all_clocks;
    clock_container->getClockIds(all_clocks);
    std::unordered_set<ClockId> all_clocks_set(all_clocks.begin(), all_clocks.end());
    container_data->buildClockRelationship(clock_groups, all_clocks_set);
    return TCL_OK;
}

int parseSdcSetClockLatency(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(  cmd->isOptionSet("delay") and cmd->isOptionSet("object_list") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockLatencyContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetClockLatencyPtr clock_latency = std::make_shared<SetClockLatency>();
    if (cmd->isOptionSet("delay")) {
        double delay = 0.0;
        bool res = cmd->getOptionValue("delay", delay);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "delay", command_line.c_str());
            return TCL_ERROR;
        }
        clock_latency->setDelay(delay);
    }
    if (cmd->isOptionSet("-rise")) {
        clock_latency->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        clock_latency->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        clock_latency->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        clock_latency->setMax();
    }
    if (cmd->isOptionSet("-dynamic")) {
        clock_latency->setDynamic();
    }
    if (cmd->isOptionSet("-source")) {
        clock_latency->setSource();
    }
    if (cmd->isOptionSet("-early")) {
        clock_latency->setEarly();
    }
    if (cmd->isOptionSet("-late")) {
        clock_latency->setLate();
    }
    const auto &clock_container = sdc->getClockContainer();
    bool is_pin_clock_latency = false;
    ClockLatencyOnPinPtr pin_clock_latency = std::make_shared<ClockLatencyOnPin>(*clock_latency);
    if (cmd->isOptionSet("-clock")) {
        std::vector<std::string> clock_list;
        bool res = cmd->getOptionValue("-clock", clock_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &clock_name : clock_list) {
            const auto &clock_id = clock_container->getClockId(clock_name);
            if (clock_id == kInvalidClockId) {
                message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
                continue;
            }
            pin_clock_latency->addClock(clock_id);
            is_pin_clock_latency = true;
        }
    }
    std::vector<ClockLatencyOnPinPtr> pin_clock_latencys;
    std::vector<SetClockLatencyPtr> clock_latencys;
    container_data->split(pin_clock_latencys, pin_clock_latency);
    container_data->split(clock_latencys, clock_latency);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            const auto &clock_id = clock_container->getClockId(object_name);
            bool success = false;
            if (clock_id != kInvalidClockId) {
                if (is_pin_clock_latency) {
                    message->issueMsg("SDC", 36, kWarn, "-clock", "object is a clock object", command_line.c_str());
                    continue;
                }
                for (const auto &latency : clock_latencys) {
                    success = container_data->addToClock(clock_id, latency);
                }
            } else {
                for (const auto &latency : pin_clock_latencys) {
                    success = container_data->addToPin(object_name, latency);
                }
            }
            if (!success) {
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetSense(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("object_list") and (cmd->isOptionSet("-positive") !=
        cmd->isOptionSet("-negative") != cmd->isOptionSet("-pulse") !=
        cmd->isOptionSet("-stop_propagation") !=
        (cmd->isOptionSet("-non_unate") and cmd->isOptionSet("-clocks")) ) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getSenseContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetSensePtr sense = std::make_shared<SetSense>();

    if (cmd->isOptionSet("-type")) {
        std::string type;
        bool res = cmd->getOptionValue("-type", type);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-type", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = sense->setType(type);
        if (!success) {
            message->issueMsg("SDC", 11, kError, "-type", command_line.c_str());
        }
    }
    if (cmd->isOptionSet("-non_unate")) {
        sense->setNonUnate();
    }
    if (cmd->isOptionSet("-clocks")) {
        std::vector<std::string> clocks_list;
        bool res = cmd->getOptionValue("-clocks", clocks_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clocks", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        for (const auto &clock_name : clocks_list) {
            const auto &clock_id = clock_container->getClockId(clock_name);
            if (clock_id == kInvalidClockId) {
                message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
                continue;
            }
            sense->addClock(clock_id);
        }
    }
    if (cmd->isOptionSet("-positive")) {
        sense->setPositive();
    }
    if (cmd->isOptionSet("-negative")) {
        sense->setNegative();
    }
    if (cmd->isOptionSet("-clock_leaf")) {
        sense->setClockLeaf();
    }
    if (cmd->isOptionSet("-stop_propagation")) {
        sense->setStopPropation();
    }
    if (cmd->isOptionSet("-pulse")) {
        std::string pulse;
        bool res = cmd->getOptionValue("-pulse", pulse);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-pulse", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = sense->setPulse(pulse);
        if (!success) {
            message->issueMsg("SDC", 11, kError, "-pulse", command_line.c_str());
        }
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &pin_name : object_list) {
            bool success = container_data->addToPin(pin_name, sense);
            if (!success) {
                message->issueMsg("SDC", 17, kError, pin_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetClockTransition(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(  cmd->isOptionSet("transition") and cmd->isOptionSet("clock_list") and
            (!(cmd->isOptionSet("-rise") and cmd->isOptionSet("-fall"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockTransitionContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetClockTransitionPtr clock_transition = std::make_shared<SetClockTransition>();

    if (cmd->isOptionSet("transition")) {
        double transition = 0.0;
        bool res = cmd->getOptionValue("transition", transition);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "transition", command_line.c_str());
            return TCL_ERROR;
        }
        clock_transition->setTransition(transition);
    }
    if (cmd->isOptionSet("-rise")) {
        clock_transition->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        clock_transition->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        clock_transition->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        clock_transition->setMax();
    }
    std::vector<SetClockTransitionPtr> clock_transitions;
    container_data->split(clock_transitions, clock_transition);
    if (cmd->isOptionSet("clock_list")) {
        std::vector<std::string> clock_list;
        bool res = cmd->getOptionValue("clock_list", clock_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "clock_list", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        for (const auto &clock_name : clock_list) {
            const auto &clock_id = clock_container->getClockId(clock_name);
            if (clock_id == kInvalidClockId) {
                message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
                continue;
            }
            for (const auto &trans : clock_transitions) {
                container_data->add(clock_id, trans);
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetClockUncertainty(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(  cmd->isOptionSet("uncertainty") and cmd->isOptionSet("object_list") != 
            ((cmd->isOptionSet("-from") != cmd->isOptionSet("-rise_from") != cmd->isOptionSet("-fall_from")) and
            (cmd->isOptionSet("-to") != cmd->isOptionSet("-rise_to") != cmd->isOptionSet("-fall_to"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getClockUncertaintyContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetClockUncertaintyPtr clock_uncertainty = std::make_shared<SetClockUncertainty>();

    if (cmd->isOptionSet("-setup")) {
        clock_uncertainty->setSetup();
    }
    if (cmd->isOptionSet("-hold")) {
        clock_uncertainty->setHold();
    }
    if (cmd->isOptionSet("uncertainty")) {
        double uncertainty = 0.0;
        bool res = cmd->getOptionValue("uncertainty", uncertainty);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "uncertainty", command_line.c_str());
            return TCL_ERROR;
        }
        clock_uncertainty->setUncertainty(uncertainty);
    }
    InterClockUncertaintyPtr inter_clock_uncertainty = std::make_shared<InterClockUncertainty>(*clock_uncertainty); 
    std::vector<std::string> from_list;
    if (cmd->isOptionSet("-from")) {
        bool res = cmd->getOptionValue("-from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-from", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->setRiseFrom();
        inter_clock_uncertainty->setFallFrom();
    }
    if (cmd->isOptionSet("-rise_from")) {
        bool res = cmd->getOptionValue("-rise_from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_from", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->setRiseFrom();
        inter_clock_uncertainty->resetFallFrom();
    }
    if (cmd->isOptionSet("-fall_from")) {
        bool res = cmd->getOptionValue("-fall_from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_from", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->resetRiseFrom();
        inter_clock_uncertainty->setFallFrom();
    }
    std::vector<std::string> to_list;
    if (cmd->isOptionSet("-to")) {
        bool res = cmd->getOptionValue("-to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-to", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->setRiseTo();
        inter_clock_uncertainty->setFallTo();
    }
    if (cmd->isOptionSet("-rise_to")) {
        bool res = cmd->getOptionValue("-rise_to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_to", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->setRiseTo();
        inter_clock_uncertainty->resetFallTo();
    }
    if (cmd->isOptionSet("-fall_to")) {
        bool res = cmd->getOptionValue("-fall_to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_to", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->resetRiseTo();
        inter_clock_uncertainty->setFallTo();
    }
    if (cmd->isOptionSet("-rise")) {
        bool rise = false;
        bool res = cmd->getOptionValue("-rise", rise);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->setRiseTo();
        inter_clock_uncertainty->resetFallTo();
    }
    if (cmd->isOptionSet("-fall")) {
        bool fall = false;
        bool res = cmd->getOptionValue("-fall", fall);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall", command_line.c_str());
            return TCL_ERROR;
        }
        inter_clock_uncertainty->setFallTo();
        inter_clock_uncertainty->resetRiseTo();
    }
    bool is_inter_clock_uncertainty = !from_list.empty() and !to_list.empty();
    const auto &clock_container = sdc->getClockContainer();
    std::vector<SetClockUncertaintyPtr> clock_uncertaintys;
    std::vector<InterClockUncertaintyPtr> inter_clock_uncertaintys;
    container_data->split(clock_uncertaintys, clock_uncertainty);
    container_data->split(inter_clock_uncertaintys, inter_clock_uncertainty);
    if (!is_inter_clock_uncertainty) {
        if (cmd->isOptionSet("object_list")) {
            std::vector<std::string> object_list;
            bool res = cmd->getOptionValue("object_list", object_list);
            if (!res) {
                message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
                return TCL_ERROR;
            }
            for (const auto &object_name : object_list) {
                const auto &clock_id = clock_container->getClockId(object_name);
                for (const auto &uncertainty : clock_uncertaintys) {
                    if (clock_id != kInvalidClockId) {
                        container_data->addToClock(clock_id, uncertainty);
                        continue;
                    }
                    bool success = container_data->addToPin(object_name, uncertainty);
                    if (success) {
                        continue;
                    }
                    message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
                }
            }
        }
    } else {
        for (const auto &from_clock_name : from_list) {
            const auto &from_clock_id = clock_container->getClockId(from_clock_name);
            if (from_clock_id == kInvalidClockId) {
                message->issueMsg("SDC", 15, kError, from_clock_name.c_str(), command_line.c_str());
                continue;
            }
            for (const auto &to_clock_name : to_list) {
                const auto &to_clock_id = clock_container->getClockId(to_clock_name);
                if (to_clock_id == kInvalidClockId) {
                    message->issueMsg("SDC", 15, kError, to_clock_name.c_str(), command_line.c_str());
                    continue;
                }
                for (const auto &uncertainty : inter_clock_uncertaintys) {
                    container_data->addToInterClock(InterClockPair(from_clock_id, to_clock_id), uncertainty);
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetDataCheck(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(  (cmd->isOptionSet("-rise_from") or cmd->isOptionSet("-from") or cmd->isOptionSet("-fall_from")) and
            (cmd->isOptionSet("-to") or cmd->isOptionSet("-rise_to") or cmd->isOptionSet("-fall_to")) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getDataCheckContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetDataCheckPtr data_check = std::make_shared<SetDataCheck>();
    std::vector<std::string> from_list;
    if (cmd->isOptionSet("-from")) {
        bool res = cmd->getOptionValue("-from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-from", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setRiseFrom();
        data_check->setFallFrom();
    }
    if (cmd->isOptionSet("-rise_from")) {
        bool res = cmd->getOptionValue("-rise_from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_from", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setRiseFrom();
        data_check->resetFallFrom();
    }
    if (cmd->isOptionSet("-fall_from")) {
        bool res = cmd->getOptionValue("-fall_from", from_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_from", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setFallFrom();
        data_check->resetRiseFrom();
    }
    std::vector<std::string> to_list;
    if (cmd->isOptionSet("-to")) {
        bool res = cmd->getOptionValue("-to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-to", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setRiseTo();
        data_check->setFallTo();
    }
    if (cmd->isOptionSet("-rise_to")) {
        bool res = cmd->getOptionValue("-rise_to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-rise_to", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setRiseTo();
        data_check->resetFallTo();
    }
    if (cmd->isOptionSet("-fall_to")) {
        bool res = cmd->getOptionValue("-fall_to", to_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-fall_to", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setFallTo();
        data_check->resetRiseTo();
    }
    if (cmd->isOptionSet("-setup")) {
        data_check->setSetup();
    }
    if (cmd->isOptionSet("-hold")) {
        data_check->setHold();
    }
    if (cmd->isOptionSet("-clock")) {
        std::vector<std::string> clock_list;
        bool res = cmd->getOptionValue("-clock", clock_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        for (const auto &clock_name : clock_list) {
            const auto &clock_id = clock_container->getClockId(clock_name);
            if (clock_id == kInvalidClockId) {
                message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
                continue;
            }
            data_check->setClock(clock_id);  //only take the first valid clock if multiple clock specified
            break;
        }
    }
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        data_check->setValue(value);
        std::vector<SetDataCheckPtr> checks;
        container_data->split(checks, data_check);
        for (const auto &from_pin_name : from_list) {
            for (const auto &to_pin_name : to_list) {
                for (const auto &check : checks) {
                    bool success = container_data->addCheck(from_pin_name, to_pin_name, check);
                    if (!success) {
                        message->issueMsg("SDC", 18, kError, from_pin_name.c_str(), to_pin_name.c_str(), command_line.c_str());
                    }
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetDisableTiming(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("object_list") and (!(cmd->isOptionSet("-to") !=  cmd->isOptionSet("-from"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getDisableTimingContainer();
    auto container_data = container->getData();
    std::string from = "";
    if (cmd->isOptionSet("-from")) {
        bool res = cmd->getOptionValue("-from", from);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-from", command_line.c_str());
            return TCL_ERROR;
        }
    }
    std::string to = "";
    if (cmd->isOptionSet("-to")) {
        bool res = cmd->getOptionValue("-to", to);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-to", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        AnalysisCorner *corner = sdc->getAnalysisCorner();
        if (!corner) {
            message->issueMsg("SDC", 34, kError, command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            bool success = container_data->addToTCell(corner, object_name, from, to);
            if (success) {
                continue;
            }
            success = container_data->addToTTerm(corner, object_name, from, to);
            if (success) {
                continue;
            }
            success = container_data->addToInst(object_name, from, to);
            if (success) {
                continue;
            }
            success = container_data->addToPin(object_name, from, to);
            if (!success) {
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetFalsePath(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( (cmd->isOptionSet("-from") or cmd->isOptionSet("-to") or cmd->isOptionSet("-through")) and
            (!(cmd->isOptionSet("-rise") and cmd->isOptionSet("-fall"))) and
            (!(cmd->isOptionSet("-setup") and cmd->isOptionSet("-hold"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getFalsePathContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetFalsePathPtr false_path = std::make_shared<SetFalsePath>();
    container_data->add(false_path);
    const auto &clock_container = sdc->getClockContainer();
    if (cmd->isOptionSet("-setup")) {
        false_path->setSetup();
    }
    if (cmd->isOptionSet("-hold")) {
        false_path->setHold();
    }
    if (cmd->isOptionSet("-rise")) {
        false_path->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        false_path->setFall();
    }
    false_path->checkFlags();
    parseFromThroughTo(false_path, cmd, clock_container, command_line);
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        false_path->setComment(comment);
    }

    return TCL_OK;
}

int parseSdcSetIdealLatency(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("value") and cmd->isOptionSet("object_list") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getIdealLatencyContainer();
    auto container_data = container->getData();
    SetIdealLatencyPtr ideal_latency = std::make_shared<SetIdealLatency>();
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        ideal_latency->setValue(static_cast<float>(value));
    }
    if (cmd->isOptionSet("-rise")) {
        ideal_latency->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        ideal_latency->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        ideal_latency->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        ideal_latency->setMax();
    }
    std::vector<SetIdealLatencyPtr> latencys;
    container_data->split(latencys, ideal_latency);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &pin_name : object_list) {
            for (const auto &latency : latencys) {
                bool success = container_data->add(pin_name, latency);
                if (!success) {
                    message->issueMsg("SDC", 17, kError, pin_name.c_str(), command_line.c_str());
                }
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetIdealNetwork(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("object_list") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getIdealNetworkContainer();
    auto container_data = container->getData();
    bool no_propagate = false;
    if (cmd->isOptionSet("-no_propagate")) {
        bool res = cmd->getOptionValue("-no_propagate", no_propagate);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-no_propagate", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            bool success = container_data->addToPin(object_name, no_propagate);
            if (success) {
                continue;
            }
            success = container_data->addToNet(object_name, no_propagate);
            if (!success) {
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetIdealTransition(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("value") and cmd->isOptionSet("object_list") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getIdealTransitionContainer();
    auto container_data = container->getData();
    SetIdealTransitionPtr ideal_transition = std::make_shared<SetIdealTransition>();
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        ideal_transition->setValue(static_cast<float>(value));
    }
    if (cmd->isOptionSet("-rise")) {
        ideal_transition->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        ideal_transition->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        ideal_transition->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        ideal_transition->setMax();
    }
    std::vector<SetIdealTransitionPtr> transitions;
    container_data->split(transitions, ideal_transition);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &pin_name : object_list) {
            for (const auto &transition : transitions) {
                bool success = container_data->add(pin_name, transition);
                if (!success) {
                    message->issueMsg("SDC", 17, kError, pin_name, command_line.c_str());
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetInputDelay(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("delay_value") and cmd->isOptionSet("port_pin_list") and
           (!( (cmd->isOptionSet("-clock_fall") or cmd->isOptionSet("-level_sensitive")) and
           (!(cmd->isOptionSet("-clock"))) )) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getInputDelayContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetInputDelayPtr input_delay = std::make_shared<SetInputDelay>();
    if (cmd->isOptionSet("-clock")) {
        std::string clock_name;
        bool res = cmd->getOptionValue("-clock", clock_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        const auto &clock_id = clock_container->getClockId(clock_name);
        if (clock_id == kInvalidClockId) {
            message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
            return TCL_ERROR;
        }
        input_delay->setClock(clock_id);
    }
    if (cmd->isOptionSet("-clock_fall")) {
        input_delay->setClockFall();
    }
    if (cmd->isOptionSet("-level_sensitive")) {
        input_delay->setLevelSensitive();
    }
    if (cmd->isOptionSet("-rise")) {
        input_delay->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        input_delay->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        input_delay->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        input_delay->setMax();
    }
    if (cmd->isOptionSet("-add_delay")) {
        input_delay->setAddDelay();
    }
    if (cmd->isOptionSet("delay_value")) {
        double delay_value = 0.0;
        bool res = cmd->getOptionValue("delay_value", delay_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "delay_value", command_line.c_str());
            return TCL_ERROR;
        }
        input_delay->setDelayValue(delay_value);
    }
    if (cmd->isOptionSet("-reference_pin")) {
        std::string reference_pin_name;
        bool res = cmd->getOptionValue("-reference_pin", reference_pin_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-reference_pin", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = input_delay->setReferencePin(reference_pin_name);
        if (!success) {
            message->issueMsg("SDC", 14, kError, reference_pin_name, "-reference_pin", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("-network_latency_included")) {
        input_delay->setNetworkLatencyIncluded();
    }
    if (cmd->isOptionSet("-source_latency_included")) {
        input_delay->setSourceLatencyIncluded();
    }
    std::vector<SetInputDelayPtr> delays;
    container_data->split(delays, input_delay);
    if (cmd->isOptionSet("port_pin_list")) {
        std::vector<std::string> port_pin_list;
        bool res = cmd->getOptionValue("port_pin_list", port_pin_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_pin_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_pin_name : port_pin_list) {
            for (const auto &delay : delays) {
                bool success = container_data->add(port_pin_name, delay);
                if (!success) {
                    message->issueMsg("SDC", 17, kError, port_pin_name.c_str(), command_line.c_str());
                    continue;
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetMaxDelay(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("delay_value") and (!(cmd->isOptionSet("-rise") and cmd->isOptionSet("-fall"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxDelayContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetMaxDelayPtr delay = std::make_shared<SetMaxDelay>();
    container_data->add(delay);
    const auto &clock_container = sdc->getClockContainer();
    if (cmd->isOptionSet("delay_value")) {
        double delay_value = 0.0;
        bool res = cmd->getOptionValue("delay_value", delay_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "delay_value", command_line.c_str());
            return TCL_ERROR;
        }
        delay->setDelayValue(static_cast<float>(delay_value));
    }
    if (cmd->isOptionSet("-rise")) {
        delay->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        delay->setFall();
    }
    delay->checkFlags();
    parseFromThroughTo(delay, cmd, clock_container, command_line);
    if (cmd->isOptionSet("-ignore_clock_latency")) {
        delay->setIgnoreClockLatency();
    }
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        delay->setComment(comment);
    }

    return TCL_OK;
}

int parseSdcSetMaxTimeBorrow(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("delay_value") and cmd->isOptionSet("object_list") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxTimeBorrowContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    const auto &clock_container = sdc->getClockContainer();

    SetMaxTimeBorrow borrow;
    if (cmd->isOptionSet("delay_value")) {
        double delay_value = 0.0;
        bool res = cmd->getOptionValue("delay_value", delay_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "delay_value", command_line.c_str());
            return TCL_ERROR;
        }
        borrow.setValue(delay_value);
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            const auto &clock_id = clock_container->getClockId(object_name);
            if (clock_id != kInvalidClockId) {
                container_data->addToClock(clock_id, borrow);
                continue;
            }
            bool success = container_data->addToInst(object_name, borrow);
            if (success) {
                continue;
            }
            success = container_data->addToPin(object_name, borrow);
            if (success) {
                continue;
            }
            message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
        }
    }
    return TCL_OK;
}

int parseSdcSetMinDelay(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("delay_value") and (!(cmd->isOptionSet("-rise") and cmd->isOptionSet("-fall"))) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMinDelayContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetMinDelayPtr delay = std::make_shared<SetMinDelay>();
    container_data->add(delay);
    const auto &clock_container = sdc->getClockContainer();
    if (cmd->isOptionSet("delay_value")) {
        double delay_value = 0.0;
        bool res = cmd->getOptionValue("delay_value", delay_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "delay_value", command_line.c_str());
            return TCL_ERROR;
        }
        delay->setDelayValue(static_cast<float>(delay_value));
    }
    if (cmd->isOptionSet("-rise")) {
        delay->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        delay->setFall();
    }
    delay->checkFlags();
    parseFromThroughTo(delay, cmd, clock_container, command_line);
    if (cmd->isOptionSet("-ignore_clock_latency")) {
        delay->setIgnoreClockLatency();
    }
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        delay->setComment(comment);
    }

    return TCL_OK;
}

int parseSdcSetMinPulseWidth(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("value") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMinPulseWidthContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetMinPulseWidthPtr pulse_width = std::make_shared<SetMinPulseWidth>();

    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        pulse_width->setValue(static_cast<float>(value));
    }
    if (cmd->isOptionSet("-low")) {
        pulse_width->setLow();
    }
    if (cmd->isOptionSet("-high")) {
        pulse_width->setHigh();
    }
    std::vector<SetMinPulseWidthPtr> pulse_widths;
    container_data->split(pulse_widths, pulse_width);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        for (const auto &object_name : object_list) {
            const auto &clock_id = clock_container->getClockId(object_name);
            for (const auto &width : pulse_widths) {
                if (clock_id != kInvalidClockId) {
                    container_data->addToClock(clock_id, width);
                    continue;
                }
                bool success = container_data->addToInst(object_name, width);
                if (success) {
                    continue;
                }
                success = container_data->addToPin(object_name, width);
                if (success) {
                    continue;
                }
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    } else {
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        for (const auto &width : pulse_widths) {
            container_data->addToCurrentDesign(design_cell_id, width);
        }
    }

    return TCL_OK;
}

int parseSdcSetMulticyclePath(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("path_multiplier") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMulticyclePathContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetMulticyclePathPtr multi_cycle_path = std::make_shared<SetMulticyclePath>();
    container_data->add(multi_cycle_path);
    const auto &clock_container = sdc->getClockContainer();
    if (cmd->isOptionSet("path_multiplier")) {
        int path_multiplier = 0;
        bool res = cmd->getOptionValue("path_multiplier", path_multiplier);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "path_multiplier", command_line.c_str());
            return TCL_ERROR;
        }
        multi_cycle_path->setPathMultiplier(static_cast<UInt32>(path_multiplier));
    }
    if (cmd->isOptionSet("-setup")) {
        multi_cycle_path->setSetup();
    }
    if (cmd->isOptionSet("-hold")) {
        multi_cycle_path->setHold();
    }
    if (cmd->isOptionSet("-rise")) {
        multi_cycle_path->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        multi_cycle_path->setFall();
    }
    if (cmd->isOptionSet("-start")) {
        multi_cycle_path->setStart();
    }
    if (cmd->isOptionSet("-end")) {
        multi_cycle_path->setEnd();
    }
    multi_cycle_path->checkFlags();
    parseFromThroughTo(multi_cycle_path, cmd, clock_container, command_line);
    if (cmd->isOptionSet("-comment")) {
        std::string comment = "";
        bool res = cmd->getOptionValue("-comment", comment);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-comment", command_line.c_str());
            return TCL_ERROR;
        }
        multi_cycle_path->setComment(comment);
    }

    return TCL_OK;
}

int parseSdcSetOutputDelay(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("delay_value") and cmd->isOptionSet("port_pin_list") and
           (!( (cmd->isOptionSet("-clock_fall") or cmd->isOptionSet("-level_sensitive")) and
           (!(cmd->isOptionSet("-clock"))) )) )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getOutputDelayContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetOutputDelayPtr output_delay = std::make_shared<SetOutputDelay>();
    if (cmd->isOptionSet("-clock")) {
        std::string clock_name;
        bool res = cmd->getOptionValue("-clock", clock_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-clock", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        const auto &clock_id = clock_container->getClockId(clock_name);
        if (clock_id == kInvalidClockId) {
            message->issueMsg("SDC", 15, kError, clock_name.c_str(), command_line.c_str());
            return TCL_ERROR;
        }
        output_delay->setClock(clock_id);
    }
    if (cmd->isOptionSet("-clock_fall")) {
        output_delay->setClockFall();
    }
    if (cmd->isOptionSet("-level_sensitive")) {
        output_delay->setLevelSensitive();
    }
    if (cmd->isOptionSet("-rise")) {
        output_delay->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        output_delay->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        output_delay->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        output_delay->setMax();
    }
    if (cmd->isOptionSet("-add_delay")) {
        output_delay->setAddDelay();
    }
    if (cmd->isOptionSet("delay_value")) {
        double delay_value = 0.0;
        bool res = cmd->getOptionValue("delay_value", delay_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "delay_value", command_line.c_str());
            return TCL_ERROR;
        }
        output_delay->setDelayValue(delay_value);
    }
    if (cmd->isOptionSet("-reference_pin")) {
        std::string reference_pin_name;
        bool res = cmd->getOptionValue("-reference_pin", reference_pin_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-reference_pin", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = output_delay->setReferencePin(reference_pin_name);
        if (!success) {
            message->issueMsg("SDC", 14, kError, reference_pin_name, "-reference_pin", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("-network_latency_included")) {
        output_delay->setNetworkLatencyIncluded();
    }
    if (cmd->isOptionSet("-source_latency_included")) {
        output_delay->setSourceLatencyIncluded();
    }
    std::vector<SetOutputDelayPtr> output_delays;
    container_data->split(output_delays, output_delay);
    if (cmd->isOptionSet("port_pin_list")) {
        std::vector<std::string> port_pin_list;
        bool res = cmd->getOptionValue("port_pin_list", port_pin_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-port_pin_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_pin_name : port_pin_list) {
            for (const auto &delay : output_delays) {
                bool success = container_data->add(port_pin_name, delay);
                if (!success) {
                    message->issueMsg("SDC", 17, kError, port_pin_name.c_str(), command_line.c_str());
                    continue;
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetPropagatedClock(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("object_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getPropagatedClockContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    auto clock_container = sdc->getClockContainer();
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            ClockPtr clock = clock_container->getClock(object_name);
            bool success = container_data->addToClock(clock);
            if (success) {
                continue;
            }
            success = container_data->addToPin(object_name);
            if (!success) {
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

// environment commands manager
int parseSdcSetCaseAnalysis(ClientData cld, Tcl_Interp *itp, int argc, const char *argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( cmd->isOptionSet("value") and cmd->isOptionSet("port_or_pin_list") )) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getCaseAnalysisContainer();
    auto container_data = container->getData();
    SetCaseAnalysis case_analysis;
    if (cmd->isOptionSet("value")) {
        std::string value;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = case_analysis.setValue(value);
        if (!success) {
            message->issueMsg("SDC", 11, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("port_or_pin_list")) {
        std::vector<std::string> port_or_pin_list;
        bool res = cmd->getOptionValue("port_or_pin_list", port_or_pin_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_or_pin_list", argv);
            return TCL_ERROR;
        }
        for (const auto& pin_name : port_or_pin_list) {
            bool success = container_data->add(pin_name, case_analysis);
            if (!success) {
                message->issueMsg("SDC", 17, kError, pin_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetDrive(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("resistance") and cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getDriveContainer();
    auto container_data = container->getData();
    SetDrivePtr drive = std::make_shared<SetDrive>();
    if (cmd->isOptionSet("resistance")) {
        double resistance = 0.0;
        bool res = cmd->getOptionValue("resistance", resistance);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "resistance", command_line.c_str());
            return TCL_ERROR;
        }
        drive->setResistance(resistance);
    }
    if (cmd->isOptionSet("-rise")) {
        drive->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        drive->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        drive->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        drive->setMax();
    }
    std::vector<SetDrivePtr> drives;
    container_data->split(drives, drive);
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_name : port_list) {
            //TODO should only set_drive to input or inout pins.
            for (const auto &res : drives) {
                bool success = container_data->add(port_name, res);
                if (!success) {
                    message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
                }
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetDrivingCell(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("port_list") and cmd->isOptionSet("-lib_cell"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getDrivingCellContainer();
    auto container_data = container->getData();
    SetDrivingCellPtr driving_cell = std::make_shared<SetDrivingCell>();
    std::string lib_cell = "";
    if (cmd->isOptionSet("-lib_cell")) {
        bool res = cmd->getOptionValue("-lib_cell", lib_cell);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-lib_cell", command_line.c_str());
            return TCL_ERROR;
        }
    }
    std::string library = "";
    if (cmd->isOptionSet("-library")) {
        bool res = cmd->getOptionValue("-library", library);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
    }
    AnalysisCorner *corner = sdc->getAnalysisCorner();
    if (!corner) {
        message->issueMsg("SDC", 34, kError, command_line.c_str());
        return TCL_ERROR;
    }
    TCell* tcell = getLibCellInCorner(corner, lib_cell, library);
    if (!tcell) {
        message->issueMsg("SDC", 19, kError, lib_cell.c_str(), library.c_str(), command_line.c_str());
        return TCL_ERROR;
    }
    std::string from_tterm_name = "";
    if (cmd->isOptionSet("-from_pin")) {
        bool res = cmd->getOptionValue("-from_pin", from_tterm_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-from_pin", command_line.c_str());
            return TCL_ERROR;
        }
    }
    std::string to_tterm_name = "";
    if (cmd->isOptionSet("-pin")) {
        bool res = cmd->getOptionValue("-pin", to_tterm_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-pin", command_line.c_str());
            return TCL_ERROR;
        }
    }
    bool success = driving_cell->checkArc(tcell, from_tterm_name, to_tterm_name);
    if (!success) {
        message->issueMsg("SDC", 37, kError, "Failed to find arc", command_line.c_str());
    }
    if (cmd->isOptionSet("-rise")) {
        driving_cell->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        driving_cell->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        driving_cell->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        driving_cell->setMax();
    }
    if (cmd->isOptionSet("-dont_scale")) {
        driving_cell->setDontScale();
    }
    if (cmd->isOptionSet("-no_design_rule")) {
        driving_cell->setNoDesignRule();
    }
    if (cmd->isOptionSet("-input_transition_rise")) {
        double input_transition_rise = 0.0;
        bool res = cmd->getOptionValue("-input_transition_rise", input_transition_rise);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-input_transition_rise", command_line.c_str());
            return TCL_ERROR;
        }
        driving_cell->setInputTransitionRise(input_transition_rise);
    }
    if (cmd->isOptionSet("-input_transition_fall")) {
        double input_transition_fall = 0.0;
        bool res = cmd->getOptionValue("-input_transition_fall", input_transition_fall);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-input_transition_fall", command_line.c_str());
            return TCL_ERROR;
        }
        driving_cell->setInputTransitionFall(input_transition_fall);
    }
    if (cmd->isOptionSet("-clock")) {
        message->issueMsg("SDC", 36, kWarn, "-clock", "deprecated", command_line.c_str());
    }
    if (cmd->isOptionSet("-clock_fall")) {
        message->issueMsg("SDC", 36, kWarn, "-clock_fall", "deprecated", command_line.c_str());
    }
    std::vector<SetDrivingCellPtr> driving_cells;
    container_data->split(driving_cells, driving_cell);
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_name : port_list) {
            for (const auto &cell : driving_cells) {
                bool success = container_data->add(port_name, cell);
                if (!success) {
                    message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
                }
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetFanoutLoad(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("value") and cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getFanoutLoadContainer();
    auto container_data = container->getData();
    SetFanoutLoad fanout;
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        fanout.setValue(static_cast<float>(value));
    }
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_name : port_list) {
            //TODO should only consider the output ports
            bool success = container_data->add(port_name, fanout);
            if (!success) {
                message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetInputTransition(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("transition") and cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getInputTransitionContainer();
    auto container_data = container->getData();
    SetInputTransitionPtr input_transition = std::make_shared<SetInputTransition>();
    if (cmd->isOptionSet("-rise")) {
        input_transition->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        input_transition->setFall();
    }
    if (cmd->isOptionSet("-min")) {
        input_transition->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        input_transition->setMax();
    }
    if (cmd->isOptionSet("transition")) {
        double transition = 0.0;
        bool res = cmd->getOptionValue("transition", transition);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "transition", command_line.c_str());
            return TCL_ERROR;
        }
        input_transition->setTransition(transition);
    }
    if (cmd->isOptionSet("-clock")) {
        message->issueMsg("SDC", 36, kWarn, "-clock", "deprecated", command_line.c_str());
    }
    if (cmd->isOptionSet("-clock_fall")) {
        message->issueMsg("SDC", 36, kWarn, "-clock_fall", "deprecated", command_line.c_str());
    }
    std::vector<SetInputTransitionPtr> input_transitions;
    container_data->split(input_transitions, input_transition);
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_name : port_list) {
            for (const auto &transition : input_transitions) {
                bool success = container_data->add(port_name, transition);
                if (!success) {
                    message->issueMsg("SDC", 14, kError, port_name.c_str(), "port_list", command_line.c_str());
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetLoad(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("value") and cmd->isOptionSet("objects"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getLoadContainer();
    auto container_data = container->getData();
    SetLoadPtr load = std::make_shared<SetLoad>();
    if (cmd->isOptionSet("-min")) {
        load->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        load->setMax();
    }
    if (cmd->isOptionSet("-rise")) {
        load->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        load->setFall();
    }
    if (cmd->isOptionSet("-substract_pin_load")) {
        load->setSubstractPinLoad();
    }
    if (cmd->isOptionSet("-pin_load")) {
        load->setPinLoad();
    }
    if (cmd->isOptionSet("-wire_load")) {
        load->setWireLoad();
    }
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        load->setCap(static_cast<float>(value));
    }
    load->checkFlags();
    std::vector<SetLoadPtr> loads;
    container_data->split(loads, load);
    if (cmd->isOptionSet("objects")) {
        std::vector<std::string> objects_list;
        bool res = cmd->getOptionValue("objects", objects_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "objects", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : objects_list) {
            for (const auto &load_cap : loads) {
                bool success = container_data->addToPort(object_name, load_cap);
                if (success) {
                    continue;
                }
                success = container_data->addToNet(object_name, load_cap);
                if (!success) {
                    message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetLogicDc(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getLogicContainer();
    auto container_data = container->getData();
    SetLogic logic;
    logic.setValue(LogicValue::kDontCare);
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& port_name : port_list) {
            bool success = container_data->add(port_name, logic);
            if (!success) {
                message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetLogicOne(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getLogicContainer();
    auto container_data = container->getData();
    SetLogic logic;
    logic.setValue(LogicValue::kOne);
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& port_name : port_list) {
            bool success = container_data->add(port_name, logic);
            if (!success) {
                message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetLogicZero(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getLogicContainer();
    auto container_data = container->getData();
    SetLogic logic;
    logic.setValue(LogicValue::kZero);
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& port_name : port_list) {
            bool success = container_data->add(port_name, logic);
            if (!success) {
                message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetMaxArea(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("area_value"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxAreaContainer();
    auto container_data = container->getData();
    SetMaxArea area;
    if (cmd->isOptionSet("area_value")) {
        double area_value = 0.0;
        bool res = cmd->getOptionValue("area_value", area_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "area_value", command_line.c_str());
            return TCL_ERROR;
        }
        area.setAreaValue(area_value);
    }
    const auto &design_container = sdc->getCurrentDesignContainer();
    const auto &design_cell_id = design_container->getDesignId();
    container_data->add(design_cell_id, area);

    return TCL_OK;
}

int parseSdcSetMaxCapacitance(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("value") and cmd->isOptionSet("object_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxCapacitanceContainer();
    auto container_data = container->getData();
    SetMaxCapacitancePtr max_cap = std::make_shared<SetMaxCapacitance>();
    if (cmd->isOptionSet("value")) {
        double capacitance_value = 0.0;
        bool res = cmd->getOptionValue("value", capacitance_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        max_cap->setCapValue(static_cast<float>(capacitance_value));
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            bool success = container_data->addToPin(object_name, max_cap);
            if (!success) {
                message->issueMsg("SDC", 17, kError, object_name.c_str(), command_line.c_str());
                continue;
            }
        }
        //TODO Not support to add to current design
    }
    return TCL_OK;
}

int parseSdcSetMaxFanout(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("fanout_value") and cmd->isOptionSet("object_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxFanoutContainer();
    auto container_data = container->getData();
    SetMaxFanout fanout;
    if (cmd->isOptionSet("fanout_value")) {
        double fanout_value = 0.0;
        bool res = cmd->getOptionValue("fanout_value", fanout_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "fanout_value", command_line.c_str());
            return TCL_ERROR;
        }
        fanout.setFanoutValue(static_cast<float>(fanout_value));
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_name = design_container->getDesignName();
        const auto &design_cell_id = design_container->getDesignId();
        for (const auto &object_name : object_list) {
            if (object_name == design_cell_name) {
                container_data->addToCurrentDesign(design_cell_id, fanout);
                continue;
            }
            //TODO need to check the port is input port
            bool success = container_data->addToPort(object_name, fanout);
            if (success) {
                continue;
            }
            message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
        }
    }
    return TCL_OK;
}

int parseSdcSetMaxTransition(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("transition_value") and cmd->isOptionSet("object_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMaxTransitionContainer();
    auto container_data = container->getData();
    container_data->setSdc(sdc);
    SetMaxTransitionPtr transition = std::make_shared<SetMaxTransition>();
    if (cmd->isOptionSet("transition_value")) {
        double transition_value = 0.0;
        bool res = cmd->getOptionValue("transition_value", transition_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "transition_value", command_line.c_str());
            return TCL_ERROR;
        }
        transition->setTransitionValue(transition_value);
    }
    if (cmd->isOptionSet("-clock_path")) {
        transition->setClockPath();
    }
    if (cmd->isOptionSet("-fall")) {
        transition->setFall();
    }
    if (cmd->isOptionSet("-rise")) {
        transition->setRise();
    }
    std::vector<SetMaxTransitionPtr> transitions;
    container_data->split(transitions, transition);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &clock_container = sdc->getClockContainer();
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_name = design_container->getDesignName();
        const auto &design_cell_id = design_container->getDesignId();
        for (const auto &object_name : object_list) {
            const ClockId &clock_id = clock_container->getClockId(object_name);
            for (const auto &trans : transitions) {
                if (clock_id != kInvalidClockId) {
                    container_data->addToClock(clock_id, trans);
                    continue;
                }
                if (object_name == design_cell_name) {
                    container_data->addToCurrentDesign(design_cell_id, trans);
                    continue;
                }
                bool success = container_data->addToPin(object_name, trans);
                if (success) {
                    continue;
                }
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetMinCapacitance(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("value") and cmd->isOptionSet("object_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getMinCapacitanceContainer();
    auto container_data = container->getData();
    SetMinCapacitancePtr min_cap = std::make_shared<SetMinCapacitance>();
    if (cmd->isOptionSet("value")) {
        double capacitance_value = 0.0;
        bool res = cmd->getOptionValue("value", capacitance_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        min_cap->setCapValue(static_cast<float>(capacitance_value));
    }
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            //TODO need to check whether it is input or input ports
            bool success = container_data->addToPin(object_name, min_cap);
            if (!success) {
                message->issueMsg("SDC", 17, kError, object_name.c_str(), command_line.c_str());
                continue;
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetOperatingConditions(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    //TODO syntax check for operating condition
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getOperatingConditionsContainer();
    auto container_data = container->getData();
    SetOperatingConditionsPtr operating_condition = std::make_shared<SetOperatingConditions>();
    if (cmd->isOptionSet("-analysis_type")) {
        std::string analysis_type = "";
        bool res = cmd->getOptionValue("-analysis_type", analysis_type);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-analysis_type", command_line.c_str());
            return TCL_ERROR;
        }
        operating_condition->setValue(analysis_type);
    }
    AnalysisCorner *corner = sdc->getAnalysisCorner();
    if (!corner) {
        message->issueMsg("SDC", 34, kError, command_line.c_str());
        return TCL_ERROR;
    }
    LibSet* lib_set = corner->getLibset();
    if (!lib_set) {
        message->issueMsg("SDC", 31, open_edi::util::kWarn, (corner->getName()).c_str());
        return TCL_ERROR;
    }
    //TODO liberty search priority:
    // 1) Option "-library"
    // 2) Option "-min_library" for min_condition ; Option "-max_library" for max_condition
    // 3) One of the libraries in the link_path (main liberty?)
    TLib *lib = nullptr;
    if (cmd->isOptionSet("-library")) {
        std::vector<std::string> library_list;
        bool res = cmd->getOptionValue("-library", library_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
        //Only take the first valid liberty, TODO need to add a warning message
        const std::string &library = library_list.front();
        lib = lib_set->getTLib(library);
        if (lib == nullptr) {
            message->issueMsg("SDC", 11, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
    }
    TLib *min_lib = nullptr;
    if (cmd->isOptionSet("-min_library")) {
        std::vector<std::string> min_library_list;
        bool res = cmd->getOptionValue("-min_library", min_library_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-min_library", command_line.c_str());
            return TCL_ERROR;
        }
        //Only take the first valid liberty, TODO need to add a warning message
        const std::string &min_library = min_library_list.front();
        min_lib = lib_set->getTLib(min_library);
        if (!min_lib) {
            message->issueMsg("SDC", 11, kError, "-min_library", command_line.c_str());
            return TCL_ERROR;
        }
    }
    TLib *max_lib = nullptr;
    if (cmd->isOptionSet("-max_library")) {
        std::vector<std::string> max_library_list;
        bool res = cmd->getOptionValue("-max_library", max_library_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-max_library_list", command_line.c_str());
            return TCL_ERROR;
        }
        //Only take the first valid liberty, TODO need to add a warning message
        const std::string &max_library = max_library_list.front();
        max_lib = lib_set->getTLib(max_library);
        if (!max_lib) {
            message->issueMsg("SDC", 11, kError, "-max_library", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("-min")) {
        std::string min = "";
        bool res = cmd->getOptionValue("-min", min);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-min", command_line.c_str());
            return TCL_ERROR;
        }
        assert(min_lib);
        const auto &min_condition = min_lib->getOperatingConditions(min);
        if (!min_condition) {
            message->issueMsg("SDC", 11, kError, "-min", command_line.c_str());
            return TCL_ERROR;
        }
        operating_condition->setMinCondition(min_condition->getId());
    }
    if (cmd->isOptionSet("-max")) {
        std::string max = "";
        bool res = cmd->getOptionValue("-max", max);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-max", command_line.c_str());
            return TCL_ERROR;
        }
        assert(max_lib);
        const auto &max_condition = max_lib->getOperatingConditions(max);
        if (!max_condition) {
            message->issueMsg("SDC", 11, kError, "-max", command_line.c_str());
            return TCL_ERROR;
        }
        operating_condition->setMaxCondition(max_condition->getId());
    }
    if (cmd->isOptionSet("condition")) {
        std::string condition_str = "";
        bool res = cmd->getOptionValue("condition", condition_str);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "condition", command_line.c_str());
            return TCL_ERROR;
        }
        assert(lib);
        const auto &condition = lib->getOperatingConditions(condition_str);
        if (!condition) {
            message->issueMsg("SDC", 11, kError, "condition", command_line.c_str());
            return TCL_ERROR;
        }
        operating_condition->setCondition(condition->getId());
    }
    if (cmd->isOptionSet("-object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("-object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            bool success = container_data->addToInst(object_name, operating_condition);
            if (success) {
                continue;
            }
            success = container_data->addToPort(object_name, operating_condition);
            if (!success) {
                message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
                return TCL_ERROR;
            }
        }
    } else {
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        container_data->addToCurrentDesign(design_cell_id, operating_condition);
    }
    return TCL_OK;
}

int parseSdcSetPortFanoutNumber(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("fanout_number") and cmd->isOptionSet("port_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getPortFanoutNumberContainer();
    auto container_data = container->getData();
    SetPortFanoutNumber fanout;
    if (cmd->isOptionSet("fanout_number")) {
        int fanout_number = 0;
        bool res = cmd->getOptionValue("fanout_number", fanout_number);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "fanout_number", command_line.c_str());
            return TCL_ERROR;
        }
        fanout.setFanoutNumber(static_cast<UInt32>(fanout_number));
    }
    if (cmd->isOptionSet("port_list")) {
        std::vector<std::string> port_list;
        bool res = cmd->getOptionValue("port_list", port_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "port_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &port_name : port_list) {
            bool success = container_data->add(port_name, fanout);
            if (!success) {
                message->issueMsg("SDC", 17, kError, port_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetResistance(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("value") and cmd->isOptionSet("net_list"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getResistanceContainer();
    auto container_data = container->getData();
    SetResistancePtr resistance = std::make_shared<SetResistance>();
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        resistance->setValue(static_cast<float>(value));
    }
    if (cmd->isOptionSet("-min")) {
        resistance->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        resistance->setMax();
    }
    std::vector<SetResistancePtr> resistances;
    container_data->split(resistances, resistance);
    if (cmd->isOptionSet("net_list")) {
        std::vector<std::string> net_list;
        bool res = cmd->getOptionValue("net_list", net_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "net_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &net_name : net_list) {
            for (const auto &res : resistances) {
                bool success = container_data->add(net_name, res);
                if (!success) {
                    message->issueMsg("SDC", 20, kError, net_name.c_str(), command_line.c_str());
                }
            }
        }
    }

    return TCL_OK;
}

int parseSdcSetTimingDerate(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!( (cmd->isOptionSet("derate_value") and (cmd->isOptionSet("-early") and !(cmd->isOptionSet("-late")))
        or (cmd->isOptionSet("-late") and !(cmd->isOptionSet("-early")))))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getTimingDerateContainer();
    auto container_data = container->getData();
    SetTimingDeratePtr timing_derate = std::make_shared<SetTimingDerate>();
    if (cmd->isOptionSet("derate_value")) {
        double derate_value = 0.0;
        bool res = cmd->getOptionValue("derate_value", derate_value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "derate_value", command_line.c_str());
            return TCL_ERROR;
        }
        timing_derate->setDerateValue(derate_value);
    }
    if (cmd->isOptionSet("-min")) {
        timing_derate->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        timing_derate->setMax();
    }
    if (cmd->isOptionSet("-rise")) {
        timing_derate->setRise();
    }
    if (cmd->isOptionSet("-fall")) {
        timing_derate->setFall();
    }
    if (cmd->isOptionSet("-early ")) {
        timing_derate->setEarly();
    }
    if (cmd->isOptionSet("-late")) {
        timing_derate->setLate();
    }
    if (cmd->isOptionSet("-static")) {
        timing_derate->setStaticType();
    }
    if (cmd->isOptionSet("-dynamic")) {
        timing_derate->setDynamicType();
    }
    if (cmd->isOptionSet("-increment")) {
        timing_derate->setIncrement();
    }
    if (cmd->isOptionSet("-clock")) {
        timing_derate->setClock();
    }
    if (cmd->isOptionSet("-data")) {
        timing_derate->setData();
    }
    if (cmd->isOptionSet("-net_delay")) {
        timing_derate->setNetDelay();
    }
    if (cmd->isOptionSet("-cell_delay")) {
        timing_derate->setCellDelay();
    }
    if (cmd->isOptionSet("-cell_check")) {
        timing_derate->setCellCheck();
    }
    timing_derate->checkFlags();
    std::vector<SetTimingDeratePtr> timing_derates;
    container_data->split(timing_derates, timing_derate);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        AnalysisCorner *corner = sdc->getAnalysisCorner();
        if (!corner) {
            message->issueMsg("SDC", 34, kError, command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            //TODO need special handling for increment
            for (const auto &derate : timing_derates) {
                bool success = container_data->addToTCell(corner, object_name, derate);
                if (success) {
                    continue;
                }
                success = container_data->addToInst(object_name, derate);
                if (success) {
                    continue;
                }
                success = container_data->addToNet(object_name, derate);
                if (!success) {
                    message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
                }
            }
        }
    } else {
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        for (const auto &derate : timing_derates) {
            container_data->addToCurrentDesign(design_cell_id, derate);
        }
    }

    return TCL_OK;
}

int parseSdcSetVoltage(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("value"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getVoltageContainer();
    auto container_data = container->getData();
    SetVoltagePtr voltage = std::make_shared<SetVoltage>();
    if (cmd->isOptionSet("value")) {
        double value = 0.0;
        bool res = cmd->getOptionValue("value", value);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "value", command_line.c_str());
            return TCL_ERROR;
        }
        voltage->setMaxCaseVoltage(static_cast<float>(value));
    }
    if (cmd->isOptionSet("-min")) {
        double min = 0.0;
        bool res = cmd->getOptionValue("-min", min);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-min", command_line.c_str());
            return TCL_ERROR;
        }
        voltage->setMinCaseVoltage(static_cast<float>(min));
    }
    if (cmd->isOptionSet("-object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("-object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &power_net_name : object_list) {
            bool success = container_data->addToPowerNet(power_net_name, voltage);
            if (!success) {
                message->issueMsg("SDC", 20, kError, power_net_name.c_str(), command_line.c_str());
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetWireLoadMinBlockSize(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("size"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getWireLoadMinBlockSizeContainer();
    auto container_data = container->getData();
    SetWireLoadMinBlockSize min_block_size;
    if (cmd->isOptionSet("size")) {
        double size = 0.0;
        bool res = cmd->getOptionValue("size", size);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "size", command_line.c_str());
            return TCL_ERROR;
        }
        min_block_size.setBlockSize(static_cast<float>(size));
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        container_data->add(design_cell_id, min_block_size);
    }
    return TCL_OK;
}

int parseSdcSetWireLoadMode(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("mode_name"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getWireLoadModeContainer();
    auto container_data = container->getData();
    SetWireLoadMode mode;
    if (cmd->isOptionSet("mode_name")) {
        std::string mode_name = "";
        bool res = cmd->getOptionValue("mode_name", mode_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "mode_name", command_line.c_str());
            return TCL_ERROR;
        }
        bool success = mode.set(mode_name);
        if (!success) {
            message->issueMsg("SDC", 11, kError, mode_name.c_str(), command_line.c_str());
        }
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        container_data->add(design_cell_id, mode);
    }
    return TCL_OK;
}

int parseSdcSetWireLoadModel(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("-name"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getWireLoadModelContainer();
    auto container_data = container->getData();
    SetWireLoadModelPtr model = std::make_shared<SetWireLoadModel>();
    AnalysisCorner *corner = sdc->getAnalysisCorner();
    if (!corner) {
        message->issueMsg("SDC", 34, kError, command_line.c_str());
        return TCL_ERROR;
    }
    LibSet* lib_set = corner->getLibset();
    if (!lib_set) {
        message->issueMsg("SDC", 31, open_edi::util::kWarn, (corner->getName()).c_str());
        return TCL_ERROR;
    }
    TLib *lib = nullptr;
    if (cmd->isOptionSet("-library")) {
        std::vector<std::string> library_list;
        bool res = cmd->getOptionValue("-library", library_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
        //Only take the first valid liberty, TODO need to add a warning message
        const std::string &library = library_list.front();
        lib = lib_set->getTLib(library);
        if (!lib) {
            //TODO need continue to get lib from main liberty
            message->issueMsg("SDC", 11, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("-name")) {
        std::string model_name = "";
        bool res = cmd->getOptionValue("-name", model_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &wire_load = lib->getWireLoad(model_name);
        if (!wire_load) {
            message->issueMsg("SDC", 11, kError, "-name", command_line.c_str());
            return TCL_ERROR;
        }
        model->setWireLoad(wire_load->getId());
    }
    if (cmd->isOptionSet("-min")) {
        bool min = false;
        bool res = cmd->getOptionValue("-min", min);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-min", command_line.c_str());
            return TCL_ERROR;
        }
        model->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        bool max = false;
        bool res = cmd->getOptionValue("-max", max);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-max", command_line.c_str());
            return TCL_ERROR;
        }
        model->setMax();
    }
    std::vector<SetWireLoadModelPtr> models;
    container_data->split(models, model);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto &object_name : object_list) {
            for (const auto &load_model : models) {
                bool success = container_data->addToCell(object_name, load_model);
                if (success) {
                    continue;
                }
                success = container_data->addToPin(object_name, load_model);
                if (!success) {
                    message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
                }
            }
        }
    }
    return TCL_OK;
}

int parseSdcSetWireLoadSelectionGroup(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("group_name"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    SdcPtr sdc = getSdcFromCmd(cmd, command_line);
    if (!sdc) {
        message->issueMsg("SDC", 27, kError, command_line.c_str());
        return TCL_ERROR;
    };
    auto container = sdc->getWireLoadSelectionGroupContainer();
    auto container_data = container->getData();
    SetWireLoadSelectionGroupPtr selection = std::make_shared<SetWireLoadSelectionGroup>();
    AnalysisCorner *corner = sdc->getAnalysisCorner();
    if (!corner) {
        message->issueMsg("SDC", 34, kError, command_line.c_str());
        return TCL_ERROR;
    }
    LibSet* lib_set = corner->getLibset();
    if (!lib_set) {
        message->issueMsg("SDC", 31, open_edi::util::kWarn, (corner->getName()).c_str());
        return TCL_ERROR;
    }
    TLib *lib = nullptr;
    if (cmd->isOptionSet("-library")) {
        std::vector<std::string> library_list;
        bool res = cmd->getOptionValue("-library", library_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
        //Only take the first valid liberty, TODO need to add a warning message
        const std::string &library = library_list.front();
        lib = lib_set->getTLib(library);
        if (!lib) {
            //TODO need continue to get lib from main liberty
            message->issueMsg("SDC", 11, kError, "-library", command_line.c_str());
            return TCL_ERROR;
        }
    }
    if (cmd->isOptionSet("group_name")) {
        std::string group_name = "";
        bool res = cmd->getOptionValue("group_name", group_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "group_name", command_line.c_str());
            return TCL_ERROR;
        }
        const auto &wire_load_selection = lib->getWireLoadSelection(group_name);
        if (!wire_load_selection) {
            message->issueMsg("SDC", 11, kError, "group_name", command_line.c_str());
            return TCL_ERROR;
        }
        selection->setSelection(wire_load_selection->getId());
    }
    if (cmd->isOptionSet("-min")) {
        bool min = false;
        bool res = cmd->getOptionValue("-min", min);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-min", command_line.c_str());
            return TCL_ERROR;
        }
        selection->setMin();
    }
    if (cmd->isOptionSet("-max")) {
        bool max = false;
        bool res = cmd->getOptionValue("-max", max);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-max", command_line.c_str());
            return TCL_ERROR;
        }
        selection->setMax();
    }
    std::vector<SetWireLoadSelectionGroupPtr> selection_groups;
    container_data->split(selection_groups, selection);
    if (cmd->isOptionSet("object_list")) {
        std::vector<std::string> object_list;
        bool res = cmd->getOptionValue("object_list", object_list);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "object_list", command_line.c_str());
            return TCL_ERROR;
        }
        for (const auto& object_name : object_list) {
            for (const auto &sel : selection_groups) {
                bool success = container_data->addToInst(object_name, sel);
                if (!success) {
                    message->issueMsg("SDC", 16, kError, object_name.c_str(), command_line.c_str());
                }
            }
        }
    } else {
        const auto &design_container = sdc->getCurrentDesignContainer();
        const auto &design_cell_id = design_container->getDesignId();
        for (const auto &sel : selection_groups) {
            container_data->addToCurrentDesign(design_cell_id, sel);
        }
    }

    return TCL_OK;
}

int readSdc(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!(cmd->isOptionSet("sdc_files"))) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-version")) {
        std::string version = "2.1";
        bool res = cmd->getOptionValue("-version", version);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-version", command_line.c_str());
            return TCL_ERROR;
        }
        if (version != "2.1") {
            message->issueMsg("SDC", 21, open_edi::util::kWarn, command_line.c_str());
        }
    }
    std::string analysis_view_name = "default";
    if (cmd->isOptionSet("-view")) {
        bool res = cmd->getOptionValue("-view", analysis_view_name);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-view", command_line.c_str());
            return TCL_ERROR;
        }
    }
    std::vector<std::string> sdc_files;
    bool res = cmd->getOptionValue("sdc_files", sdc_files);
    if (!res) {
        message->issueMsg("SDC", 10, kError, "sdc_files", command_line.c_str());
        return TCL_ERROR;
    }
    if (sdc_files.empty()) {
        message->issueMsg("SDC", 22, kError, command_line.c_str());
        return TCL_ERROR;
    }
    AnalysisView* view = getOrCreateViewForSdc(analysis_view_name);
    if (!view) {
        message->issueMsg("SDC", 9, kError, analysis_view_name.c_str(), command_line.c_str());
        return TCL_ERROR;
    }
    if (!(view->getSdc())) {
        message->issueMsg("SDC", 24, kError, analysis_view_name.c_str(), command_line.c_str());
        return TCL_ERROR;
    }
    //upate sdc files in mode
    AnalysisMode *mode = view->getAnalysisMode();
    if (!mode) {
        message->issueMsg("SDC", 5, kError, analysis_view_name.c_str());
        return TCL_ERROR;
    }
    for (const auto &file : sdc_files) {
        mode->addConstraintFile(file);
    }
    message->info("Reading sdc in view %s\n", (view->getName()).c_str());
    std::string tcl_commands;
    getSdcFileContents(tcl_commands, sdc_files, view->getName());
    if (tcl_commands.empty()) {
        message->issueMsg("SDC", 23, kError, analysis_view_name.c_str(), command_line.c_str());
        return TCL_ERROR;
    }
    bool status = Tcl_Eval(itp, tcl_commands.c_str());
    if (status == TCL_ERROR) {
        message->issueMsg("SDC", 35, kError, analysis_view_name.c_str(), command_line.c_str());
        return TCL_ERROR;
    }
    message->info("Read sdc in view %s successfully.\n\n", (view->getName()).c_str());
    return TCL_OK;
}

int writeSdc(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    Command* cmd = CommandManager::parseCommand(argc, argv);
    assert(cmd);
    const std::string &command_line = cmd->getCurrentCmd();
    if (!cmd->isOptionSet("-view") or !cmd->isOptionSet("sdc_file")) {
        message->issueMsg("SDC", 12, kError, command_line.c_str());
        return TCL_ERROR;
    }
    if (cmd->isOptionSet("-version")) {
        std::string version = "2.1";
        bool res = cmd->getOptionValue("-version", version);
        if (!res) {
            message->issueMsg("SDC", 10, kError, "-version", command_line.c_str());
            return TCL_ERROR;
        }
        if (version != "2.1") {
            message->issueMsg("SDC", 21, open_edi::util::kWarn, command_line.c_str());
        }
    }
    std::string analysis_view_name = "";
    bool res = cmd->getOptionValue("-view", analysis_view_name);
    if (!res) {
        message->issueMsg("SDC", 10, kError, "-view", command_line.c_str());
        return TCL_ERROR;
    }
    Timing *timing_db = getTimingLib();
    if (!timing_db) {
        message->issueMsg("SDC", 1, kError);
        return TCL_ERROR;
    }
    AnalysisView* view = timing_db->getAnalysisView(analysis_view_name);
    if (!view) {
        message->issueMsg("SDC", 6, kError, analysis_view_name.c_str());
        return TCL_ERROR;
    }
    auto &sdc = view->getSdc();
    if (!sdc) {
        message->issueMsg("SDC", 24, kError, analysis_view_name.c_str());
        return TCL_ERROR;
    }
    std::string sdc_file = "";
    res = cmd->getOptionValue("sdc_file", sdc_file);
    if (!res) {
        message->issueMsg("SDC", 10, kError, "sdc_file", command_line.c_str());
        return TCL_ERROR;
    }
    std::ofstream output(sdc_file);
    if (!output) {
        message->issueMsg("SDC", 25, kError, sdc_file.c_str());
        return TCL_ERROR;
    }
    output << *sdc << std::endl;
    message->info("Write sdc file %s for view %s successfully.\n", sdc_file.c_str(), analysis_view_name.c_str());

    return TCL_OK;
}

}
}
