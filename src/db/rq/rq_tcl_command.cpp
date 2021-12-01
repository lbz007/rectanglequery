
/* @file  rq_tcl_command.cpp
 * @date  June 2021
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NiiCEDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "db/rq/rq_tcl_command.h"
#include "db/rq/rq.h"

namespace open_edi {
namespace db {

// read LEF file
static int initQueryCommand(Command* cmd) {
    int result = cmdInitQuery(cmd);
    return result;
}

static int queryCommand(Command* cmd) {
    int result = cmdQuery(cmd);
    return result;
}

static int cleanupQueryCommand(Command* cmd) {
    int result = cmdCleanupQuery(cmd);
    return result;
}

void registerRQTclCommands(Tcl_Interp *itp) {
    CommandManager *cmd_manager = CommandManager::getCommandManager();
    Command *init_query_command = cmd_manager->createObjCommand(
        itp, initQueryCommand, "init_query", "Initialize query data\n",
        cmd_manager->createOption("check_design", OptionDataType::kBoolNoValue, false,
                               "check if design is loaded.\n"));

    Command *query_command = cmd_manager->createObjCommand(
        itp, queryCommand, "query", "Query data\n",
        cmd_manager->createOption("area", OptionDataType::kRect, false,
                               "search window size.\n"));

    Command *cleanup_query_command = cmd_manager->createObjCommand(
        itp, cleanupQueryCommand, "cleanup_query", "Initialize query data\n",
        cmd_manager->createOption("check_data", OptionDataType::kBoolNoValue, false,
                               "check if data is initialized.\n"));
}

}  // namespace db
}  // namespace open_edi
