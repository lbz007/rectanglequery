/**
 * @file timinglib_tcl_command_register.h
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
#ifndef EDI_DB_TIMING_TIMINGLIB_TCL_COMMAND_REGISTER_H_
#define EDI_DB_TIMING_TIMINGLIB_TCL_COMMAND_REGISTER_H_

#include <tcl.h>
#include "db/timing/timinglib/timinglib_tcl_command_parser.h"

namespace open_edi {
namespace db {

void registerTimingLibCommands(Tcl_Interp *itp);

}  // namespace db
}  // namespace open_edi
#endif  // EDI_DB_TIMING_TIMINGLIB_TCL_COMMAND_REGISTER_H_
