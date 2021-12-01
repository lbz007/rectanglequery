/* @file  read_verilog.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include <string.h>
#include <stdio.h>
#include <queue>
#include <vector>
#include <string>
#include <unordered_map>

#include "db/io/read_verilog.h"
#include "db/core/db.h"
#include "parser/verilog/kernel/register.h"
#include "parser/verilog/kernel/log.h"
#include "parser/verilog/frontends/verilog/verilog_callback.h"
#include "infra/command_manager.h"
#include "util/message.h"

#include <gperftools/profiler.h>

namespace open_edi {

namespace db {
using namespace open_edi::infra;
using IdArray = ArrayObject<ObjectId>;
static Yosys::RTLIL::Design *yosys_design = NULL;
static bool kFirstRunReadVerilog = false;
static std::unordered_map<Inst*, std::string> kInstMasterMap;
static std::unordered_map<Pin*, std::string> kPinNameMap;
static std::unordered_map<char, bool> kValidCharMap;
static std::vector<Cell*> kModuleVector;
static Cell *kCurrentModule = nullptr;

/// @brief id2db Convert identity from Verilog format to DB format
///
/// @param txt
///
/// @return 
static std::string id2db(std::string txt) {
    if (txt.size() > 1 && txt[0] == '\\') {
        txt = txt.substr(1);
    }
    for (size_t i = 0; i < txt.size(); i++) {
        // A simple identifier shall be any sequence of letters, digits,
        // dollar signs ($), and underscore characters (_).
        if (kValidCharMap[txt[i]]) {
            continue;
        }
        // has special character, escape it.
        txt.insert(i, "\\");
        i++;
    }
    return txt;
}

/// @brief getIntConstant Get an integer value from AST node
///
/// @param current_node
/// @param constant
///
/// @return 
static bool getIntConstant(struct Yosys::AST::AstNode *current_node,
                           int &constant) {
    int bound = 0;
    if (current_node->type == Yosys::AST::AstNodeType::AST_NEG &&
        current_node->children[0]->type == 
                                     Yosys::AST::AstNodeType::AST_CONSTANT) {
        bound = -1 * current_node->children[0]->integer;
    } else if (current_node->type == Yosys::AST::AstNodeType::AST_CONSTANT) {
        bound = current_node->integer;
    } else {
        return false;
    }
    constant = bound;

    return true;
}

/// @brief getRealConstant Get an double value from AST node
///
/// @param current_node
/// @param constant
///
/// @return 
static bool getRealConstant(struct Yosys::AST::AstNode *current_node,
                            double &constant) {
    double bound = 0.0;
    if (current_node->type == Yosys::AST::AstNodeType::AST_NEG && 
        current_node->children[0]->type ==
                                      Yosys::AST::AstNodeType::AST_REALVALUE) {
        bound = -1 * current_node->children[0]->realvalue;
    } else if (current_node->type == Yosys::AST::AstNodeType::AST_REALVALUE) {
        bound = current_node->realvalue;
    } else {
        return false;
    }
    constant = bound;

    return true;
}

/// @brief getIdWithRange Get identity with range
///
/// @param ast_node
///
/// @return 
static std::string getIdWithRange(struct Yosys::AST::AstNode *ast_node) {
    std::string name;
    int int_constant = 0;
    if (ast_node->type == Yosys::AST::AstNodeType::AST_IDENTIFIER) {
        name = name + id2db(ast_node->str);
        for (auto child : ast_node->children) {
            if (child->type == Yosys::AST::AstNodeType::AST_RANGE) {
                for (auto child_child : child->children) {
                    bool result = getIntConstant(child_child, int_constant);
                    if (result) {
                        name = name + "[" + std::to_string(int_constant) + "]";
                    }
                    break;
                }
                break;
            }
        }
    }
    return name;
}

 

/// @brief findMasterForInst Find masters for all instances
static void findMasterForInst() {
    Cell *top_cell = getTopCell();
    for (auto it : kInstMasterMap) {
        Inst *inst = it.first;
        std::string cell_name = it.second;
        Cell *cell = top_cell->getCell(cell_name);
        if (!cell) {
            message->issueMsg("DBIO", 34, kWarn, cell_name.c_str());
            Object::destroyObject<Inst>(inst);
            continue;
        }
        kInstMasterMap.erase(inst);

        inst->setMaster(cell->getId());

        ObjectId pins = inst->getPins();
        IdArray *pins_vector = Object::addr<IdArray>(pins);
        IdArray *pgpin_vector = nullptr;
        for (int i = 0; i < pins_vector->getSize(); i++) {
            ObjectId pin_id = (*pins_vector)[i];
            Pin *pin = Object::addr<Pin>(pin_id);
            const char *pin_name = kPinNameMap[pin].c_str();
            Term *term = cell->getTerm(pin_name);
            if (!term) {
                message->issueMsg("DBIO", 35, kError, cell_name.c_str(), pin->getName().c_str());
                break;
            }
            pin->setTerm(term);
            Net *net = pin->getNet();
            net->addPin(pin);
            kPinNameMap.erase(pin);
        }
    }
}

#if 0
static void setTopCell(Cell *new_top_cell) {
    Cell *top_cell = getTopCell();
    std::string cell_name = new_top_cell->getName();
    message->info("Set top design to %s\n", cell_name.c_str());
    top_cell->setName(cell_name);
    // copy term to top cell
    // copy net to top cell
    // copy inst to top cell
}
/// @brief setTopCell Find and set the top cell
static void findAndSetTopCell() {
    std::unordered_set<Cell*> master_set;
    std::vector<ObjectId> module_id_vector;
    std::vector<Cell*> candidate_vector;
    Root *root = getRoot();
    Cell *top_cell = getTopCell();
    Cell *new_cell = nullptr;
    Cell *master_cell = nullptr;
    Inst *inst = nullptr;

    for (auto cell : kModuleVector) {
        ArrayObject<ObjectId> *inst_array = cell->getInstanceArray();
        for (int j = 0; j < inst_array->getSize(); ++j) {
            inst = cell->getInstance((*inst_array)[j]);
            master_cell = inst->getMaster();
            if (master_cell && master_cell->isHierCell()) {
                master_set.insert(master_cell);
            }
        }
    }
    for (auto cell : kModuleVector) {
        if (master_set.find(cell) == master_set.end()) {
            candidate_vector.push_back(cell);
        }
    }
    if (candidate_vector.size() > 1) {
        message->issueMsg(kWarn, "There are %d root modules in Verilog file.\n"
                         "User should specify which module is top design.\n",
                                                   candidate_vector.size());
        message->info("Root module names:\n");
        for (auto cell : candidate_vector) {
            message->info(" %s", cell->getName().c_str());
        }
        message->info("\n");
    } else if (candidate_vector.size() == 1) {
        setTopCell(candidate_vector[0]);
    } else {
        message->issueMsg(kError,
                                "There is not root module in Verilog file.\n");
    }
}
#endif

/// @brief moduleCbk Read in Verilog Module name
///
/// @param str
///
/// @return 
static bool moduleCbk(std::string &str) {
    std::string module_name = id2db(str);
    Cell *top_cell = getTopCell();
    if (!module_name.compare(top_cell->getName())) {
        kCurrentModule = top_cell;
    } else {
        kCurrentModule = top_cell->createCell(module_name, true);
        if (!kCurrentModule) {
            message->issueMsg("DBIO", 14, kError, "design", module_name.c_str());
        }
    }
    kModuleVector.push_back(kCurrentModule);
    return true;
}

/// @brief wireCbk Read in Verilog wires of a module
///
/// @param current_node
///
/// @return 
static bool wireCbk(struct Yosys::AST::AstNode *current_node) {
    Term *term = nullptr;
    Net  *net = nullptr;
    Bus  *bus = nullptr;
    int32_t range_left = INT_MAX, range_right = INT_MIN;
    std::string wire_name;
    wire_name = id2db(current_node->str);
    bool is_term = false;
    bool is_tri = current_node->is_tri;

    if (current_node->port_id > 0) {
        // is port
        is_term = true;
        term = kCurrentModule->createTerm(wire_name);
        if (!term) {
            message->issueMsg("DBIO", 36, kError, "term",
                    wire_name.c_str(), kCurrentModule->getName().c_str());
            return false;
        }
        term->setIsWriteVerilog(true);
        term->setIsWriteDef(true);
        if (current_node->is_input && current_node->is_output) {
            term->setDirection(SignalDirection::kInout);
        } else if (current_node->is_input) {
            term->setDirection(SignalDirection::kInput);
        } else if (current_node->is_output) {
            term->setDirection(SignalDirection::kOutput);
        }
    }
    for (auto child : current_node->children) {
        if (child->type == Yosys::AST::AstNodeType::AST_RANGE) {
            if (child->children.size() < 2) {
                message->issueMsg("DBIO", 37, kError, wire_name.c_str());
                return false;
            }
            getIntConstant(child->children[0], range_left);
            getIntConstant(child->children[1], range_right);

            if (term) {
                term->setHasRange(true);
                term->setIsWriteDef(false);
                if (range_left < range_right) {
                    term->setRangeLow(range_left);
                    term->setRangeHigh(range_right);
                } else {
                    term->setRangeLow(range_right);
                    term->setRangeHigh(range_left);
                }
            } else {
                bus = kCurrentModule->createBus(wire_name);
                if (!bus) {
                    message->issueMsg("DBIO", 36, kError, "bus",
                            wire_name.c_str(), kCurrentModule->getName().c_str());
                    return false;
                }
                bus->setRangeLeft(range_left);
                bus->setRangeRight(range_right);
                if (is_tri) {
                    bus->setIsTri(true);
                }
            }
        }
    }
    // create net
    std::vector<std::string> net_names;
    std::vector<std::string> bus_net_names;
    if (range_left != INT_MAX || range_right != INT_MIN) {
        std::string net_name = wire_name;
        net = kCurrentModule->getNet(net_name);
        if (!net) {
            net = kCurrentModule->createNet(net_name);
            if (!net) {
                message->issueMsg("DBIO", 36, kError, "net",
                        net_name.c_str(), kCurrentModule->getName().c_str());
                return false;
            }
        }
        net->setIsBusNet(true);
        if (is_term) {
            net->setIsFromTerm(true);
        }
        if (is_tri) {
            net->setType(NetType::kNetTypeTri);
        }

        int range_low = range_right;
        int range_high = range_left;

        if (range_left < range_right) {
            range_low = range_left;
            range_high = range_right;
        }
        for (int32_t i = range_low; i <= range_high; ++i) {
            net_name = wire_name + "[" + std::to_string(i) + "]";
            bus_net_names.push_back(net_name);
        }
    } else {
        net_names.push_back(wire_name);
    }
    for (std::string net_name : net_names) {
        net = kCurrentModule->getNet(net_name);
        if (!net) {
            net = kCurrentModule->createNet(net_name);
            if (!net) {
                message->issueMsg("DBIO", 36, kError, "net",
                        net_name.c_str(), kCurrentModule->getName().c_str());
                return false;
            }
        }
        if (is_tri) {
            net->setType(NetType::kNetTypeTri);
        }
        if (is_term) {
            // a term should has pin and net with the same name
            Pin *pin = kCurrentModule->createIOPin(net_name);
            if (!pin) {
                message->issueMsg("DBIO", 36, kError, "pin",
                        net_name.c_str(), kCurrentModule->getName().c_str());
                return false;
            }
            pin->setTerm(term);
            pin->setNet(net);
            net->addPin(pin);
            net->setIsFromTerm(true);
        }
    }
    for (std::string net_name : bus_net_names) {
        net = kCurrentModule->getNet(net_name);
        if (!net) {
            net = kCurrentModule->createNet(net_name);
            if (!net) {
                message->issueMsg("DBIO", 36, kError, "net",
                        net_name.c_str(), kCurrentModule->getName().c_str());
                return false;
            }
        }
        net->setIsOfBus(true);
        if (is_term) {
            // a term should has pin and net with the same name
            term = kCurrentModule->createTerm(net_name);
            if (!term) {
                message->issueMsg("DBIO", 36, kError, "term",
                        wire_name.c_str(), kCurrentModule->getName().c_str());
                return false;
            }
            term->setIsWriteVerilog(false);
            term->setIsWriteDef(true);
            Pin *pin = kCurrentModule->createIOPin(net_name);
            if (!pin) {
                message->issueMsg("DBIO", 36, kError, "pin",
                        net_name.c_str(), kCurrentModule->getName().c_str());
                return false;
            }
            pin->setTerm(term);
            pin->setNet(net);
            net->addPin(pin);
            net->setIsFromTerm(true);
        }
    }
    return true;
}

/// @brief instCbk Read in Verilog inst in a module
///
/// @param current_node
///
/// @return 
static bool instCbk(struct Yosys::AST::AstNode *current_node) {
    std::string inst_name = id2db(current_node->str);
    std::string cell_name = "";
    Cell *cell = nullptr;
    Inst *inst = kCurrentModule->createInstance(inst_name);
    Pin  *pin = nullptr;
    if (!inst) {
        message->issueMsg("DBIO", 36, kError, "instance",
                inst_name.c_str(), kCurrentModule->getName().c_str());
        return false;
    }
    inst->setOwner(kCurrentModule->getId());

    for (auto child : current_node->children) {
        if (child->type == Yosys::AST::AstNodeType::AST_CELLTYPE) {
            cell_name = id2db(child->str);
            cell = getTopCell()->getCell(cell_name);
            if (cell) {
                inst->setMaster(cell->getId());
            } else {
                kInstMasterMap[inst] = cell_name;
            }
        } else if (child->type == Yosys::AST::AstNodeType::AST_ARGUMENT) {
            std::string pin_name = id2db(child->str);
            pin = nullptr;
            if (cell) {
                pin = inst->createInstancePin(pin_name);
            } else {
                pin = inst->createInstancePinWithoutMaster(pin_name);
            }
            if (!pin) {
                message->issueMsg("DBIO", 38, kError,
                        pin_name.c_str(), inst_name.c_str(),
                        kCurrentModule->getName().c_str());
                break;
            }
            if (!cell) {
                kPinNameMap[pin] = pin_name;
            }
            for (auto child_child : child->children) {
                if (child_child->type ==
                                    Yosys::AST::AstNodeType::AST_IDENTIFIER) {
                    std::string net_name = getIdWithRange(child_child);
                    Net *net = kCurrentModule->getNet(net_name);
                    if (!net) {
                        message->issueMsg("DBIO", 39, kError,
                                net_name.c_str(), kCurrentModule->getName().c_str());
                        break;
                    }
                    pin->setNet(net);
                    // Adds net=>pin connection when master is available.
                    if (cell) {
                        net->addPin(pin);
                    }
                    break;
                } else if (child_child->type ==
                                    Yosys::AST::AstNodeType::AST_CONCAT) {
                    std::string net_name;
                    for (auto child_child_child : child_child->children) {
                        net_name = getIdWithRange(child_child_child);
                        Net *net = kCurrentModule->getNet(net_name);
                        if (!net) {
                            message->issueMsg("DBIO", 39, kError,
                                    net_name.c_str(), kCurrentModule->getName().c_str());
                            break;
                        }
                        pin->addNet(net);
                        // Adds net=>pin connection when master is available.
                        if (cell) {
                            net->addPin(pin);
                        }
                    }
                }
            }
        }
    }
    return true;
}

/// @brief assignCbk Read in Verilog assign statement
///
/// @param current_node
///
/// @return 
static bool assignCbk(struct Yosys::AST::AstNode *current_node) {
    bool is_int = false;
    bool is_real = false;
    int  int_constant = 0;
    double real_constant = 0.0;
    std::string left_net_name = "";
    std::string right_net_name = "";
    Net *left_net = nullptr;
    Net *right_net = nullptr;

    if (current_node->children.size() == 2) {
        left_net_name = getIdWithRange(current_node->children[0]);
        if (current_node->children[1]->type
                == Yosys::AST::AstNodeType::AST_IDENTIFIER) {
            right_net_name = getIdWithRange(current_node->children[1]);
        } else {
            is_int = getIntConstant(current_node->children[1], int_constant);
            if (!is_int) {
                is_real = getRealConstant(current_node->children[1],
                                                              real_constant);
                if (!is_real) {
                    return false;
                }
            }
        }
        left_net = kCurrentModule->getNet(left_net_name);
        if (!left_net) {
            message->issueMsg("DBIO", 40, kWarn,
                    left_net_name.c_str(),
                    kCurrentModule->getName().c_str());
            return false;
        }
        if (!right_net_name.empty()) {
            right_net = kCurrentModule->getNet(right_net_name);
            if (!right_net) {
                message->issueMsg("DBIO", 40, kWarn,
                        right_net_name.c_str(),
                        kCurrentModule->getName().c_str());
                return false;
            }
            left_net->addAssignNet(right_net->getId());
        } else if (is_int) {
            left_net->addAssignConstant(int_constant);
        } else if (is_real) {
            left_net->addAssignConstant(real_constant);
        } else {
            return false;
        }
    }

    return true;
}

/// @brief initValidCharMap Initialize unordered_map for for Verilog charactors 
static void initValidCharMap() {
    for (char c = 'A'; c <= 'Z'; ++c) {
        kValidCharMap[c] = true;
    }
    for (char c = '0'; c <= '9'; ++c) {
        kValidCharMap[c] = true;
    }
    kValidCharMap['_'] = true;
    for (char c = 'a'; c <= 'z'; ++c) {
        kValidCharMap[c] = true;
    }
    kValidCharMap['$'] = true;
}

int cmdReadVerilog(Command* cmd) {
    std::vector<std::string> args;
    std::string top_name;
    bool is_top_set = false;

    if (cmd == nullptr) return TCL_ERROR;
    args.push_back(cmd->getName());

    bool res = false;
    if (cmd->isOptionSet(toString(ReadVerilogOption::kFileName))) {
        res = cmd->getOptionValue(
            toString(ReadVerilogOption::kFileName), args);
    }
    if (res && cmd->isOptionSet(toString(ReadVerilogOption::kTop))) {
        is_top_set = true;
        res = cmd->getOptionValue(
            toString(ReadVerilogOption::kTop), top_name);
    }
    return readVerilog(args, top_name, is_top_set);
}

/// @brief readVerilog Main entrance of read_verilog command
///
/// @param argc
/// @param argv
///
/// @return 
int readVerilog(std::vector<std::string>& args, std::string& top_name, bool is_top_set) {
    //ProfilerStart("read_verilog.perf");
    if (is_top_set) {
        Cell *top_cell = getTopCell();
        top_cell->setName(top_name);
    } else {
        message->issueMsg("DBIO", 41, kError);
        return TCL_ERROR;
    }

    if (!kFirstRunReadVerilog) {
        kFirstRunReadVerilog = true;
        Yosys::log_files.push_back(stdout);
        Yosys::Pass::init_register();
        initValidCharMap();

        verilog_parser::kVerilogCallback.setModuleCbk(moduleCbk);
        verilog_parser::kVerilogCallback.setWireCbk(wireCbk);
        verilog_parser::kVerilogCallback.setInstCbk(instCbk);
        verilog_parser::kVerilogCallback.setAssignCbk(assignCbk);
    }

    if (NULL == yosys_design) {
        yosys_design = new Yosys::RTLIL::Design;
    }

    auto state = Yosys::pass_register[args[0]]->pre_execute();
    Yosys::pass_register[args[0]]->execute(args, yosys_design);
    Yosys::pass_register[args[0]]->post_execute(state);

    findMasterForInst();
    //findAndSetTopCell(); // TODO(haoqs) : find and set top
    //ProfilerStop();
    return TCL_OK;
}

}  // namespace db
}  // namespace open_edi
