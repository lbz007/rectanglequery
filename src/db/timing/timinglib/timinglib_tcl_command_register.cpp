/**
 * @file timinglib_tcl_command_register.cpp
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

#include "db/timing/timinglib/timinglib_tcl_command_register.h"
#include "infra/command_manager.h"

namespace open_edi {
namespace db {

using Command = open_edi::infra::Command;
using CommandManager = open_edi::infra::CommandManager;
using OptionDataType = open_edi::infra::OptionDataType;

void registerTimingLibCommands(Tcl_Interp *itp) {
    CommandManager* cmd_manager = CommandManager::getCommandManager();
    assert(cmd_manager);

    Command* cmd = cmd_manager->createCommand(itp,
        readMMMCFile,  
        "read_mmmc_file", "\n",
        cmd_manager->createOption("file", OptionDataType::kString, true, "\n")
    );
    assert(cmd);
}

};
};
