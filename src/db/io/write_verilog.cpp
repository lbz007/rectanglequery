/* @file  write_verilog.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include <zlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <queue>

#include "db/io/write_verilog.h"
#include "db/core/db.h"
#include "db/core/object.h"
#include "db/util/array.h"
#include "util/message.h"
#include "util/io_manager.h"
#include "infra/command_manager.h"

namespace open_edi {
namespace db {
using namespace open_edi::infra;

/// @brief writeHeader Write header
///
/// @param io_manager
///
/// @return 
static bool writeHeader(IOManager &io_manager) {
    io_manager.write("/////////////////////////////////////////////\n"
                     "// Created by: NIICEDA tool\n"
                     "// Top Design: %s\n"
                     "/////////////////////////////////////////////\n",
                     getTopCell()->getName().c_str());
    return true;
}

/// @brief isSimpleCharactor Judge a charactor if is simple charactor in 
//  Verilog identity.
///
/// @param c
///
/// @return 
static bool isSimpleCharactor(char c) {
    if (('A' <= c && c <= 'Z') ||
        ('a' <= c && c <= 'z') ||
        ('0' <= c && c <= '9') ||
        ('$' == c) || ('_' == c)) {
        return true;
    }
    return false;
}

static std::string id2vl(std::string txt) {
    bool need_escape = false;
    bool is_bus_name = false;
    uint32_t len = txt.size();
    uint32_t i = 0;
    if (txt.size() > 1 && txt[0] == '\\') {
        txt = txt.substr(1);
    }
    while (i < len) {
        // has special character, escape it.
        if ('\\' == txt[i] && (i < len-1 && !isSimpleCharactor(txt[i+1]))) {
            txt.erase(i, 1);
            len--;
            need_escape = true;
            i++;
            continue;
        } else if ('[' == txt[i]) {
            is_bus_name = true;
            break;
        }
        i++;
    }
    if (need_escape) {
        if (is_bus_name) {
            txt.insert(i, " ");
            txt = "\\" + txt;
        } else {
            txt = "\\" + txt + " ";
        }
    }
    return txt;
}

/// @brief writeModule Write module
///
/// @param io_manager
/// @param cell
///
/// @return 
static bool writeModule(IOManager &io_manager, Cell *cell) {
    // term
    ObjectId terms = cell->getTerms();
    ArrayObject<ObjectId> *term_vector =
                                   cell->addr< ArrayObject<ObjectId> >(terms);
    if (!term_vector) {
        message->issueMsg("DBIO", 48, kError, "term", terms, cell->getName().c_str());
        return false;
    }
    io_manager.write("module %s (", id2vl(cell->getName()).c_str());
    bool first = true;
    Term *term = nullptr;
    for (int i = 0; i < term_vector->getSize(); ++i) {
        term = cell->addr<Term>((*term_vector)[i]);
        if (!term) {
            message->issueMsg("DBIO", 49, kError, "term",
                    (*term_vector)[i], cell->getName().c_str());
            return false;
        }
        if (!term->getIsWriteVerilog()) {
            continue;
        }
        if (first) {
            first = false;
            io_manager.write(" %s", id2vl(term->getName()).c_str());
        } else {
            io_manager.write(", %s", id2vl(term->getName()).c_str());
        }
    }
    io_manager.write(" );\n");

    for (int i = 0; i < term_vector->getSize(); ++i) {
        term = cell->addr<Term>((*term_vector)[i]);
        if (!term) {
            message->issueMsg("DBIO", 49, kError, "term",
                    (*term_vector)[i], cell->getName().c_str());
            return false;
        }
        if (!term->getIsWriteVerilog()) {
            continue;
        }
        std::string direction_str = term->getDirectionStr();
        toLower(direction_str);
        io_manager.write("  %s", direction_str.c_str());
        if (term->getHasRange()) {
            io_manager.write(" [%d:%d]",
                term->getRangeHigh(), term->getRangeLow());
        }
        io_manager.write(" %s;\n", id2vl(term->getName()).c_str());
    }

    // bus
    ObjectId buses = cell->getBuses();
    if (buses > 0) {
        Bus *bus = nullptr;
        ArrayObject<ObjectId> *bus_vector
                                  = cell->addr< ArrayObject<ObjectId> >(buses);
        if (!bus_vector) {
            message->issueMsg("DBIO", 48, kError, "bus",
                    buses, cell->getName().c_str());
            return false;
        }
        for (int i = 0; i < bus_vector->getSize(); ++i) {
            bus = cell->addr<Bus>((*bus_vector)[i]);
            if (!bus) {
                message->issueMsg("DBIO", 48, kError, "bus",
                        (*bus_vector)[i], cell->getName().c_str());
                return false;
            }
            if (bus->getIsTri()) {
                io_manager.write("  tri [%d:%d]",
                    bus->getRangeLeft(), bus->getRangeRight());
            } else {
                io_manager.write("  wire [%d:%d]",
                    bus->getRangeLeft(), bus->getRangeRight());
            }
            io_manager.write(" %s;\n", id2vl(bus->getName()).c_str());
        }
    }
    // net
    std::vector<Net*> assign_nets;
    ObjectId nets = cell->getNets();
    if ( nets > 0 ) {
        Net *net = nullptr;
        ArrayObject<ObjectId> *net_vector
                                   = cell->addr< ArrayObject<ObjectId> >(nets);
        if (!net_vector) {
            message->issueMsg("DBIO", 48, kError, "net",
                    nets, cell->getName().c_str());
            return false;
        }
        for (int i = 0; i < net_vector->getSize(); ++i) {
            net = cell->addr<Net>((*net_vector)[i]);
            if (!net) {
                message->issueMsg("DBIO", 49, kError, "net",
                        (*net_vector)[i], cell->getName().c_str());
                return false;
            }
            if (!(net->getIsBusNet()) && !(net->getIsOfBus())
                    && !(net->getIsFromTerm())) {
                if (NetType::kNetTypeTri == net->getType()) {
                    io_manager.write("  tri %s;\n",
                        id2vl(net->getName()).c_str());
                } else {
                    io_manager.write("  wire %s;\n",
                        id2vl(net->getName()).c_str());
                }
            }
            AssignType assign_type = net->getAssignType();
            if ((assign_type == kAssignTypeNet) ||
                (assign_type == kAssignTypeInt) ||
                (assign_type == kAssignTypeReal)) {
                assign_nets.push_back(net);
            }

       }
    }
    // output assign statement
    for (auto net : assign_nets) {
        AssignType assign_type = net->getAssignType();
        switch (assign_type) {
            case kAssignTypeNet: {
                ObjectId assign_net_id = net->getAssignNet();
                if (assign_net_id > 0) {
                    Net *assign_net = cell->addr<Net>(assign_net_id);
                    if (!assign_net) {
                        message->issueMsg("DBIO", 49, kError, "net",
                                assign_net_id, cell->getName().c_str());
                        return false;
                    }
                    io_manager.write("  assign %s = %s;\n",
                        id2vl(net->getName()).c_str(),
                        id2vl(assign_net->getName()).c_str());
                }
                }
                break;
            case kAssignTypeInt: {
                int int_constant = net->getAssignInt();
                if (0 == int_constant) {
                    io_manager.write("  supply0 %s;\n",
                        id2vl(net->getName()).c_str());
                } else if (1 == int_constant) {
                    io_manager.write("  supply1 %s;\n",
                        id2vl(net->getName()).c_str());
                } else {
                    io_manager.write("  assign %s = %d;\n",
                        id2vl(net->getName()).c_str(), int_constant);
                }
                }
                break;
            case kAssignTypeReal: {
                double real_constant = net->getAssignReal();
                io_manager.write("  assign %s = %g;\n",
                    id2vl(net->getName()).c_str(), real_constant);
                }
                break;
            default:
                break;
        }
    }
 
    // instance
    ObjectId insts = cell->getInstances();
    if ( insts > 0 ) {
        Inst *inst = nullptr;
        ArrayObject<ObjectId> *inst_vector
                                  = cell->addr< ArrayObject<ObjectId> >(insts);
        if (!inst_vector) {
            message->issueMsg("DBIO", 48, kError, "instance",
                    insts, cell->getName().c_str());
            return false;
        }
        for (int i = 0; i < inst_vector->getSize(); ++i) {
            inst = cell->addr<Inst>((*inst_vector)[i]);
            if (!inst) {
                message->issueMsg("DBIO", 48, kError, "instance",
                        (*inst_vector)[i], cell->getName().c_str());
                return false;
            }
            Cell *master = inst->getMaster();
            if (!master) {
                message->issueMsg("DBIO", 50, kError,
                        inst->getName().c_str());
                return false;
            }

            io_manager.write("  %s %s (",
                id2vl(master->getName()).c_str(), id2vl(inst->getName()).c_str());

            ObjectId pins = inst->getPins();
            ArrayObject<ObjectId> *pins_vector =
                    cell->addr<ArrayObject<ObjectId>>(pins);
            if (!pins_vector) {
                message->issueMsg("DBIO", 51, kError, "pins vector",
                        pins, inst->getName().c_str());
                //return false;
            }
            first = true;
            for (int j = 0; pins_vector && j < pins_vector->getSize(); j++) {
                ObjectId pin_id = (*pins_vector)[j];
                Pin *pin = cell->addr<Pin>(pin_id);
                if (!pin) {
                    message->issueMsg("DBIO", 51, kError, "pin",
                            pin_id, inst->getName().c_str());
                    return false;
                }
                if (first) {
                    first = false;
                    io_manager.write(" .");
                } else {
                    io_manager.write(", .");
                }
                io_manager.write("%s(", id2vl(pin->getName()).c_str());

                if (false == pin->getIsConnectNets()) {
                    Net *net = pin->getNet();
                    if (net) {
                        io_manager.write("%s", id2vl(net->getName()).c_str());
                    }
                } else {
                    io_manager.write("{ ");
                    ArrayObject<ObjectId> *net_array = pin->getNetArray();
                    ArrayObject<ObjectId>::iterator iter;
                    Net *net = nullptr;
                    bool first_net = true;
                    for (iter = net_array ->begin(); iter != net_array->end();
                            ++iter) {
                        net = cell->addr<Net>(*iter);
                        if (net) {
                            if (!first_net) {
                                io_manager.write(", ");
                            }
                            io_manager.write("%s", id2vl(net->getName()).c_str());
                            first_net = false;
                        }
                    }
                    io_manager.write(" }");
                }
                io_manager.write(")");
            }
            io_manager.write(" );\n");
        }
    }

    io_manager.write("endmodule\n\n");
    return true;
}

/// @brief writeModules Write modules
///
/// @param io_manager
///
/// @return 
static bool writeModules(IOManager &io_manager) {
    Cell *top_cell = getTopCell();
    Cell *cell = nullptr;
    ArrayObject<ObjectId> *vct = nullptr;
    ediAssert(top_cell != nullptr);
    ObjectId cells = top_cell->getCells();

    if (cells != 0) {
        vct = top_cell->addr< ArrayObject<ObjectId> >(cells);
    }
    if (vct) {
        for (int i = 0; i < vct->getSize(); ++i) {
            cell = top_cell->addr<Cell>((*vct)[i]);
            if (!cell) {
                message->issueMsg("DBIO", 52, kError, i);
                continue;
            }
            if (cell->getCellType() == CellType::kHierCell) {
                writeModule(io_manager, cell);
            }
        }
    }
    // Top cell is written at the end of Verilog file
    writeModule(io_manager, top_cell);
    return true;
}

int cmdWriteVerilog(Command* cmd) {
    std::string file_name;
    if (cmd == nullptr) return 0;
    if (cmd->isOptionSet("file")) {
        bool res = cmd->getOptionValue("file", file_name);
        return writeVerilog(file_name);
    }
    return TCL_ERROR;
}

/// @brief writeVerilog Write verilog file
///
/// @param argc
/// @param argv
///
/// @return 
int writeVerilog(std::string& file_name) {

    if (file_name.empty()) {
        message->issueMsg("DBIO", 53, kError);
        return 1;
    }
    Cell *top_cell = getTopCell();
    IOManager io_manager;
    if (!io_manager.open(file_name.c_str(), "wb")) {
        message->issueMsg("DBIO", 54, kError,
                file_name.c_str(), strerror(errno));
        return 1;
    }
    message->info("\nWriting Verilog\n");
    fflush(stdout);

    if (!writeHeader(io_manager)) {
        message->issueMsg("DBIO", 55, kError, "header");
        return 1;
    }

    if (!writeModules(io_manager)) {
        message->issueMsg("DBIO", 55, kError, "module");
        return 1;
    }
    return 0;
}

}  // namespace db
}  // namespace open_edi
