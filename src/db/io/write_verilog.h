/* @file  write_verilog.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_DB_IO_WRITE_VERILOG_H_
#define SRC_DB_IO_WRITE_VERILOG_H_
#include "infra/command_manager.h"

namespace open_edi {
namespace db {
using namespace open_edi::infra;

    int writeVerilog(std::string& file_name);
    int cmdWriteVerilog(Command* cmd);
}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_IO_WRITE_VERILOG_H_
