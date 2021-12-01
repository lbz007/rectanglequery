/* @file  write_lef.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include <stdio.h>

#include "db/io/write_lef.h"
#include "db/tech/export_tech.h"
#include "infra/command_manager.h"

namespace open_edi {
namespace db {
using namespace open_edi::infra;

int cmdWriteLef(Command* cmd) {
    char *lef_file_name = nullptr;
    if (cmd == nullptr) return 0;
    if (cmd->isOptionSet("file")) {
        std::string file;
        bool res = cmd->getOptionValue("file", file);
        lef_file_name = const_cast<char *>(file.c_str());
        return writeLef(lef_file_name);
    }
    return 2;
}

int writeLef( char *lef_file_name) {
    int num_out_file = 0;

    message->info("\nWriting LEF\n");
    fflush(stdout);

    ExportTechLef dump(lef_file_name);
    dump.exportAll();

    message->info("\nWrite LEF successfully.\n");
    return 0;
}

}  // namespace db
}  // namespace open_edi
