
/* @file  command_manager.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <mutex>
#include <stack>

#include "command_manager.h"
#include "db/util/box.h"

namespace open_edi {
namespace infra {
//  using namespace opengeo;

std::mutex infra_mutex_;

CommandManager* CommandManager::instance_ = nullptr;

CommandManager* CommandManager::getCommandManager() {
    if (instance_ == nullptr) {
        if (infra_mutex_.try_lock()) {
            instance_ = new CommandManager();
            infra_mutex_.unlock();
        }
    }
    return instance_;
}

void CommandManager::printHelp(Command* cmd) {
    message->info(cmd->getDescription().c_str());
    for (int i = 0; i < cmd->getOptionNum(); i++) {
        Option *opt = cmd->getOption(i);
        if (opt == nullptr) {
            message->issueMsg("INFRA", kGetOptionFail, kError);
        }
        if (opt->isPublic() == kPrivate) {
            continue;
        }
        std::string head = opt->getName();
        switch (opt->getType())
        {
        case OptionDataType::kNone:
            break;
        case OptionDataType::kBoolNoValue:
        case OptionDataType::kBoolHasValue:
        {
            head += " <bool>";
            break;
        }
        case OptionDataType::kInt:
        {
            head += " <int>";
            break;
        }
        case OptionDataType::kDouble:
        {
            head += " <double>";
            break;
        }
        case OptionDataType::kString:
        {
            head += " <string>";
            break;
        }
        case OptionDataType::kEnum:
        {
            head += " <enum>";
            break;
        }
        case OptionDataType::kPoint:
        {
            head += " <point>";
            break;
        }
        case OptionDataType::kRect:
        {
            head += " <rect>";
            break;
        }
        case OptionDataType::kIntList:
        {
            head += " <int list>";
            break;
        }
        case OptionDataType::kDoubleList:
        {
            head += " <double list>";
            break;
        }
        case OptionDataType::kStringList:
        {
            head += " <string list>";
            break;
        }
        default:
        {
            break;
        }
        }
        int max_size = 30;
        if (head.size() > max_size) {
            head += "\n";
            message->info(head.c_str());
            std::string body;
            for (int i = 0; i < max_size; i++) {
                body += " ";
            }
            body += opt->getDescription().c_str();
            //body += "\n";
            message->info(body.c_str());
        } else {
            int blank_size = max_size - head.size();
            for (int i = 0; i < blank_size; i++) {
                head += " ";
            }
            head += opt->getDescription().c_str();
            //head += "\n";
            message->info(head.c_str());
        }
    }
}

int CommandManager::executeCommand(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[]) {
    //std::cout << "in callback \n" << std::endl;
    Command* cmd = getCommandManager()->getCommandByName(Tcl_GetStringFromObj(objv[0], 0));
    if (objc == 1){
        if (cmd != nullptr) {
            cmd->preParse();
        }
        //return cmd;
    }
    if (objc == 2) {
        if (!strcmp(Tcl_GetStringFromObj(objv[1], 0), "--help") || !strcmp(Tcl_GetStringFromObj(objv[1], 0), "-help")) {
            printHelp(cmd);
            return TCL_OK;
        }
    }

    if (cmd != nullptr) {
        int res = cmd->objParser(objc, objv);
        if (res != 0) {
            message->info("mesage parse error\n");
            //return cmd;   // whether return nulllptr
            return -1;
        }
    } else {
        message->info("command not recoginized \n");
        return -1;
    }

    pthread_t process_bar_thread = 0;
    if (0 == process_bar_thread && cmd->useProcessBar()) {
        pthread_create(&process_bar_thread, NULL, processBar, NULL);
    }

    int res = (*(cmd->getRegisterCallback()))(cmd);

    if (0 != process_bar_thread && cmd->useProcessBar()) {
        pthread_cancel(process_bar_thread);
    }
    return res;
}

int CommandManager::RegisterCommand(Command* v) {
    std::map<std::string, Command*>::iterator iter = commands_.find(v->getName());
    if (iter == commands_.end()) {
        commands_[v->getName()] = v;
        return 0;
    } else {
        return -1;
    }
}


Command* CommandManager::getCommandByName(const char* name) {
    std::map<std::string, Command*>::iterator iter = commands_.find(name);
    if (iter != commands_.end())
        return iter->second;
    else
        return nullptr;
}

Command* CommandManager::createCommandByName(const char* name) {
    Command* command = new Command();
    command->setName(name);
    RegisterCommand(command);
    return command;
}

Command* CommandManager::parseCommand(int argc, const char *argv[]) {
    Command* cmd = getCommandManager()->getCommandByName(argv[0]);
    if (argc == 1){
        return cmd;
    }
    if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-help")) {
        printHelp(cmd);
        return nullptr;
    }
    if (cmd != nullptr) {
        int res = cmd->parser(argc, argv);
        if (res != 0) {
            message->info("mesage parse error\n");
            return cmd;   // whether return nulllptr
        }
    } else {
        message->info("command not recoginized \n");
    }
    return cmd;
}

Command* CommandManager::createCommand(Tcl_Interp *itp, commandCallback cb, const char* cmd_name, const char* description) {
    Command* command = new Command();
    command->setName(cmd_name);
    command->setDescription(description);
    RegisterCommand(command);
    Tcl_CreateCommand(itp, cmd_name, cb, NULL, NULL);
    return command;
}

Command* CommandManager::createObjCommand(Tcl_Interp *itp, registeredCallback cb, const char* cmd_name, const char* description, bool add_processbar) {
    Command* command = new Command();
    command->setName(cmd_name);
    command->setDescription(description);
    command->setUseProcessBar(add_processbar);
    command->setTclInterp(itp);
    RegisterCommand(command);
    command->setRegisterCallback(cb);
    Tcl_CreateObjCommand(itp, cmd_name, executeCommand, NULL, NULL);

    return command;
}

Command* CommandManager::createCommand(Tcl_Interp *itp, commandCallback cb, const char* cmd_name, const char* description, Option& opt_head) {
    Command* command = new Command();
    std::string name_cmd = cmd_name;
    command->setName(cmd_name);
    command->setDescription(description);
    RegisterCommand(command);
    Option* opt_ptr = &opt_head;
    std::stack<Option*> opt_stack;  // to reverse the sequence in the command
    while (opt_ptr) {
        opt_stack.push(opt_ptr);
        opt_ptr = opt_ptr->getNext();
    }
    while (!opt_stack.empty()) {
        command->addOption(opt_stack.top());
        //  message->info("add opt %s in create command \n", opt_stack.top()->getName().c_str());
        opt_stack.pop();
    }
    Tcl_CreateCommand(itp, cmd_name, cb, NULL, NULL);

    return command;
}

Command* CommandManager::createObjCommand(Tcl_Interp *itp, registeredCallback cb, const char* cmd_name, const char* description, Option& opt_head, bool add_processbar) {
    Command* command = new Command();
    std::string name_cmd = cmd_name;
    command->setName(cmd_name);
    command->setDescription(description);
    command->setUseProcessBar(add_processbar);
    command->setTclInterp(itp);
    RegisterCommand(command);
    Option* opt_ptr = &opt_head;
    std::stack<Option*> opt_stack;  // to reverse the sequence in the command
    while (opt_ptr) {
        opt_stack.push(opt_ptr);
        opt_ptr = opt_ptr->getNext();
    }
    while (!opt_stack.empty()) {
        command->addOption(opt_stack.top());
        //  message->info("add opt %s in create command \n", opt_stack.top()->getName().c_str());
        opt_stack.pop();
    }
    command->setRegisterCallback(cb);
    Tcl_CreateObjCommand(itp, cmd_name, executeCommand, NULL, NULL);
    return command;
}

Command* CommandManager::createCommand(Tcl_Interp *itp, commandCallback cb, const char* cmd_name, const char* description, Option& opt_head, OptionGroup& group_head) {
    Command* command = new Command();
    std::string name_cmd = cmd_name;
    command->setName(cmd_name);
    command->setDescription(description);
    RegisterCommand(command);
    Option* opt_ptr = &opt_head;
    std::stack<Option*> opt_stack;  // to reverse the sequence in the command
    while (opt_ptr) {
        opt_stack.push(opt_ptr);
        opt_ptr = opt_ptr->getNext();
    }
    while (!opt_stack.empty()) {
        command->addOption(opt_stack.top());
        //  message->info("add opt %s in create command \n", opt_stack.top()->getName().c_str());
        opt_stack.pop();
    }

    OptionGroup* group_ptr = &group_head;
    while (group_ptr) {
        Option* opt1 = command->getOption(group_ptr->getOpt1Name().c_str());
        if (opt1 == nullptr)
            message->issueMsg("INFRA", kGroupGetOptionFail, kError, group_ptr->getOpt1Name().c_str());
        group_ptr->setOpt1Ptr(opt1);
        Option* opt2 = command->getOption(group_ptr->getOpt2Name().c_str());
        if (opt2 == nullptr)
            message->issueMsg("INFRA", kGroupGetOptionFail, kError, group_ptr->getOpt2Name().c_str());
        group_ptr->setOpt2Ptr(opt2);

        command->addGroup(group_ptr);
        group_ptr = group_ptr->getNext();
    }

    Tcl_CreateCommand(itp, cmd_name, cb, NULL, NULL);
    return command;
}

Command* CommandManager::createObjCommand(Tcl_Interp *itp, registeredCallback cb, const char* cmd_name, const char* description,
                                             Option& opt_head, OptionGroup& group_head, bool add_processbar) {
    Command* command = new Command();
    std::string name_cmd = cmd_name;
    command->setName(cmd_name);
    command->setDescription(description);
    command->setUseProcessBar(add_processbar);
    command->setTclInterp(itp);
    RegisterCommand(command);
    Option* opt_ptr = &opt_head;
    std::stack<Option*> opt_stack;  // to reverse the sequence in the command
    while (opt_ptr) {
        opt_stack.push(opt_ptr);
        opt_ptr = opt_ptr->getNext();
    }
    while (!opt_stack.empty()) {
        command->addOption(opt_stack.top());
        //  message->info("add opt %s in create command \n", opt_stack.top()->getName().c_str());
        opt_stack.pop();
    }

    OptionGroup* group_ptr = &group_head;
    while (group_ptr) {
        Option* opt1 = command->getOption(group_ptr->getOpt1Name().c_str());
        if (opt1 == nullptr)
            message->issueMsg("INFRA", kGroupGetOptionFail, kError, group_ptr->getOpt1Name().c_str());
        group_ptr->setOpt1Ptr(opt1);
        Option* opt2 = command->getOption(group_ptr->getOpt2Name().c_str());
        if (opt2 == nullptr)
            message->issueMsg("INFRA", kGroupGetOptionFail, kError, group_ptr->getOpt2Name().c_str());
        group_ptr->setOpt2Ptr(opt2);

        command->addGroup(group_ptr);
        group_ptr = group_ptr->getNext();
    }
    command->setRegisterCallback(cb);
    Tcl_CreateObjCommand(itp, cmd_name, executeCommand, NULL, NULL);
    return command;
}

Option& CommandManager::createOption(const char* name, OptionDataType type, bool is_required, const char* description) {
    return *(new Option(name, type, is_required, description));
}

Option& CommandManager::createOption(const char* name, OptionDataType type, bool is_required, std::vector<std::string>* v, const char* description) {
    return *(new Option(name, type, is_required, v, description));
}

Option& CommandManager::createOption(const char* name, OptionDataType type, bool is_required, const char* v, const char* description) {
    return *(new Option(name, type, is_required, v, description));
}

Option& CommandManager::createOption(const char* name, OptionDataType type, bool is_required, bool v, const char* description) {
    return *(new Option(name, type, is_required, v, description));
}

Option& CommandManager::createOption(const char* name, OptionDataType type, bool is_required, int v, const char* description, int min, int max) {
    return *(new Option(name, type, is_required, v, description, min, max));
}

Option& CommandManager::createOption(const char* name, OptionDataType type, bool is_required, double v, const char* description, double min, double max){
    return *(new Option(name, type, is_required, v, description, min, max));
}

OptionGroup& CommandManager::createOptionGroup(const char* name1, const char* name2, OptionRelation r) {
    return *(new OptionGroup(name1, name2, r));
}


}  //  namespace infra
}  //  namespace open_edi
