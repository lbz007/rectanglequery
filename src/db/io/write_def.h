/* @file  read_def.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "util/io_manager.h"
#include "infra/command_manager.h"

#ifndef SRC_DB_IO_WRITE_DEF_H_
#define SRC_DB_IO_WRITE_DEF_H_

namespace open_edi {
namespace db {
using namespace open_edi::infra;

int writeDef(char *def_file_name);
int cmdWriteDef(Command* cmd);

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_IO_WRITE_DEF_H_
