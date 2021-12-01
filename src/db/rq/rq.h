/* @file  rq.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_DB_RQ_H_
#define SRC_DB_RQ_H_

#include "infra/command_manager.h"
#include "db/rq/data_model.h"

namespace open_edi {
namespace db {

using namespace open_edi::infra;

int initQuery();
int query(const Box &search_area);
int cleanupQuery();

int cmdInitQuery(Command* cmd);
int cmdQuery(Command* cmd);
int cmdCleanupQuery(Command* cmd);

}  // namespace db
}  // namespace open_edi

#endif  // SRC_DB_RQ_H_
