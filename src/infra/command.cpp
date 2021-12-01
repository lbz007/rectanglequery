/* @file  command.cpp
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
#include <algorithm>
#include <utility>
#include <ctype.h>
#include "command.h"
#include "db/util/box.h"

namespace open_edi {
namespace infra {

void Command::addOption(Option* v) {
    options_.push_back(v);
    if (option_names_.find(v->getName()) != option_names_.end())
        message->info("Error: option name already exsit\n");
    option_names_.insert(std::make_pair<std::string, int>(v->getName(), options_.size()-1));
    options_by_order_[v->getName()] = v;
}

void Command::addGroup(OptionGroup* v) {
    opt_groups_.push_back(v);
}

Option* Command::getNextPositionalOption() {
    std::vector<Option*>::iterator iter = options_.begin();
    while (iter != options_.end()) {
        std::string cur_name = (*iter)->getName();
        // message->info("in position get name %s\n", cur_name.c_str());
        if (cur_name[0] != '-') {
            // message->info("get one positional option %s \n", cur_name.c_str());
            if ((*iter)->tempIsSet() == false) {
                return *iter;
            }
        }
        iter++;
    }
    return nullptr;
}

Option* Command::getOption(const char * name) {
    if (option_names_.find(name) != option_names_.end())
        return getOption(option_names_.find(name)->second);
    else
        return nullptr;
}

Option* Command::createOption(const char * name, OptionDataType type) {
    Option *ob = new Option();
    ob->setName(name);
    ob->setType(type);
    switch (ob->getType()) {
    case OptionDataType::kNone:
        break;
    case OptionDataType::kBoolNoValue:
    case OptionDataType::kBoolHasValue:
    {
        bool *a = new bool();
        ob->setData(a);
        break;
    }
    case OptionDataType::kInt:
    {
        int *a = new int();
        ob->setData(a);
        break;
    }
    case OptionDataType::kDouble:
    {
        double *a = new double();
        ob->setData(a);
        break;
    }
    case OptionDataType::kString:
    {
        std::vector<std::pair<std::string, int>> *a = new std::vector<std::pair<std::string, int>>();
        ob->setData(a);
        break;
    }
    case OptionDataType::kIntList:
    {
        std::vector<int> *a = new std::vector<int>();
        ob->setData(a);
        break;
    }
    case OptionDataType::kDoubleList:
    {
        std::vector<double> *a = new std::vector<double>();
        ob->setData(a);
        break;
    }
    case OptionDataType::kStringList:
    {
        std::vector<std::string> *a = new std::vector<std::string>();
        ob->setData(a);
        break;
    }
    default:
    {
        break;
    }
    }
    addOption(ob);
    return ob;
}

bool Command::isOptionSet(const char * name) {
    std::map<std::string, int>::iterator iter = option_names_.find(name);
    if (iter != option_names_.end())
        return getOption(iter->second)->isSet();
    else
        return false;
}

bool Command::getOptionValue(const char * name, bool& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kBoolHasValue && 
             op->getType() != OptionDataType::kBoolNoValue) {
                 printf("data type %d \n", op->getType());
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }  
        value = *((bool *)op->getData());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, int& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kInt && op->getType() != OptionDataType::kEnum) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
        // message->info("will get data\n");
        value = *((int *)op->getData());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, double& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kDouble) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
        value = *((double *)op->getData());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, std::string& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kString) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
           
        value = ((std::vector<std::pair<std::string, int>>*)op->getData())->at(0).first;
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, Point** value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kPoint) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
        (*value) = new Point();
        (*value)->setX(((Point *)op->getData())->getX());
        (*value)->setY(((Point *)op->getData())->getY());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, Point& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kPoint) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
        value.setX(((Point *)op->getData())->getX());
        value.setY(((Point *)op->getData())->getY());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, db::Box** value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kRect) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
        (*value) = new db::Box;
        (*value)->setLLX(((db::Box *)op->getData())->getLLX());
        (*value)->setLLY(((db::Box *)op->getData())->getLLY());
        (*value)->setURX(((db::Box *)op->getData())->getURX());
        (*value)->setURY(((db::Box *)op->getData())->getURY());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, db::Box& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kRect) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
        value.setLLX(((db::Box *)op->getData())->getLLX());
        value.setLLY(((db::Box *)op->getData())->getLLY());
        value.setURX(((db::Box *)op->getData())->getURX());
        value.setURY(((db::Box *)op->getData())->getURY());
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, std::vector<std::string>** value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kStringList) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	std::vector<std::string>* op_data = (std::vector<std::string>* )op->getData();
	for (int i = 0; i < op_data->size(); i++)
            (*value)->push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, std::vector<std::string>& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kStringList) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	    std::vector<std::string>* op_data = (std::vector<std::string>* )op->getData();
	    for (int i = 0; i < op_data->size(); i++)
            value.push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, std::vector<std::pair<std::string, int>>& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kString) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	    std::vector<std::pair<std::string, int>>* op_data = (std::vector<std::pair<std::string, int>>* )op->getData();
	    for (int i = 0; i < op_data->size(); i++)
            value.push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}


bool Command::getOptionValue(const char * name, std::vector<int>** value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kIntList) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	std::vector<int>* op_data = (std::vector<int>* )op->getData();
	for (int i = 0; i < op_data->size(); i++)
            (*value)->push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, std::vector<int>& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kIntList) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	std::vector<int>* op_data = (std::vector<int>* )op->getData();
	for (int i = 0; i < op_data->size(); i++)
            value.push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}


bool Command::getOptionValue(const char * name, std::vector<double>** value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kDoubleList) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	std::vector<double>* op_data = (std::vector<double>* )op->getData();
	for (int i = 0; i < op_data->size(); i++)
            (*value)->push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

bool Command::getOptionValue(const char * name, std::vector<double>& value) {
    if (option_names_.find(name) != option_names_.end()) {
        Option* op = getOption(option_names_.find(name)->second);
        if ( op->getType() != OptionDataType::kDoubleList) {
            message->issueMsg("INFRA", kOptionDataTypeError, kError, name);
            return false;
        }
	std::vector<double>* op_data = (std::vector<double>* )op->getData();
	for (int i = 0; i < op_data->size(); i++)
            value.push_back(op_data->at(i));
        return true;
    } else {
        message->issueMsg("INFRA", kInvalidOption, kError, name);
        return false;
    }
}

int Command::preParse() {
    for (int i = 0; i <getOptionNum(); i++) {
        Option *opt = getOption(i);
        if (opt == nullptr) {
            // fprintf(stderr, "ERROR: invalid option type %s \n", *argv);
            message->issueMsg("INFRA", kGetOptionFail, kError);
            return kGetOptionFail;
        }
        if (opt->getType() == kIntList) {
            std::vector<int> *data = (std::vector<int> *)opt->getTempData();
            data->clear();
        }
        if (opt->getType() == kStringList ) {
            std::vector<std::string> *data = (std::vector<std::string> *)opt->getTempData();
            data->clear();
        }
        if (opt->getType() == kString) {
            std::vector<std::pair<std::string, int>> *data = 
                (std::vector<std::pair<std::string, int>> *)opt->getTempData();
            data->clear();
        }
        
        if (opt->getType() == kDoubleList) {
            std::vector<double> *data = (std::vector<double> *)opt->getTempData();
            data->clear();
        }
        opt->setTempIsSet(false);
        opt->setIsSet(false);
    }
    return kSuccess;
}

int Command::postParse() {
    for (int i = 0; i <getOptionNum(); i++) {
        Option *opt = getOption(i);
        if (opt == nullptr) {
            message->issueMsg("INFRA", kGetOptionFail, kError);
            return kGetOptionFail;
        }
        if (opt->isRequired() && !opt->tempIsSet()) {
            message->issueMsg("INFRA", kGetOptionFail, kError);
            return kGetOptionFail;
        }
        if (opt->tempIsSet()) {
            int result = opt->checkRule();
            if (result != kSuccess)
                return result;
        }
    }

    if (getGroupNum() > 0) {
        for (int i = 0; i < getGroupNum(); i++) {
            OptionGroup *group = getGroup(i);
            if (group == nullptr) {
                message->info("command do not get group\n");
            }
            int result = group->checkRule();
            if (result != 0)
                return result;
        }
    }
    return kSuccess;
}

int Command::parser(int argc, const char** argv) {
    current_cmd_args_.clear();
    for(int i = 0; i < argc; i++) {
        current_cmd_args_ += argv[i];
        current_cmd_args_ += " ";
    }
    argc--;
    argv++;
    int total_num = argc;
    preParse();
    if (argc == 0)
        return kSuccess;

    Option *current_option = nullptr;
    bool current_opt_finish = true;
    while (argc > 0) {
        // message->info("will process argv %s \n", *argv);
        if (argv[0][0] == '-') {
            Option *opt = getOption(*argv);
            if (opt == nullptr) {
                message->issueMsg("INFRA", kInvalidOption, kError, *argv);
                message->info("can not find option %s \n", *argv);
                return kInvalidOption;
            }
            if (current_option) {
                OptionDataType type = current_option->getType();
                if (current_opt_finish == false) {
                    message->issueMsg("INFRA", kOptionIncorrectPara, kError, current_option->getName().c_str());
                    return kOptionIncorrectPara;
                }
            }

            current_option = opt;
            current_opt_finish = false;
            if (current_option->getType() == kBoolNoValue) {
                bool *data = (bool *)current_option->getTempData();
                *data = true;
                current_option->setTempIsSet(true);
                current_opt_finish = true;
            }
            // message->info("option name %s, data type %d \n", current_option->getName().c_str(), current_option->getType());
        } else {
            // message->info("in handle value\n");
            if (current_opt_finish == true) {
                Option *opt = getNextPositionalOption();  // add for positional option
                if (opt == nullptr) {
                    message->issueMsg("INFRA", kOptionOneValue, kError, *argv);
                    return kOptionOneValue;
                }
                current_option = opt;
                current_opt_finish = false;
            }

            int result = parseValue(current_option, *argv, total_num - argc);
            if (result != kSuccess)
                return result;
            current_opt_finish = true;
            current_option->setTempIsSet(true);
        }

        argv++;
        argc--;
        if (argc == 0) {  // when finish , check and set the last arg
            if (current_option) {
                OptionDataType type = current_option->getType();
                if (current_opt_finish == false && type != kIntList && type != kDoubleList && type != kStringList) {
                    message->issueMsg("INFRA", kOptionIncorrectPara, kError, current_option->getName().c_str());
                    return kOptionIncorrectPara;
                }
                current_option->setTempIsSet(true);
            }
        }
    }

    int res = postParse();
    if (res == kSuccess) {
        if (cb_) {
            bool res = cb_(this);
            if (res == false)
                return kUserLogicError;
        }
        setData();
    }

    return kSuccess;
}

int Command::objParser(int objc, Tcl_Obj* CONST objv[]) {
    current_cmd_args_.clear();
    for(int i = 0; i < objc; i++) {
        const char* argv = Tcl_GetStringFromObj(objv[i], 0);
        current_cmd_args_ += argv;
        current_cmd_args_ += " ";
    }
    objc--;
    objv++;
    int total_num = objc;
    preParse();

    Option *current_option = nullptr;
    bool current_opt_finish = true;
    while (objc > 0) {
        // message->info("will process argv %s \n", *argv);
        const char* argv = Tcl_GetStringFromObj(objv[0], 0);
        if (argv[0] == '-') {
            Option *opt = getOption(argv);
            if (opt == nullptr) {
                message->issueMsg("INFRA", kInvalidOption, kError, argv);
                message->info("can not find option %s \n", argv);
                return kInvalidOption;
            }
            if (current_option) {
                OptionDataType type = current_option->getType();
                if (current_opt_finish == false) {
                    message->issueMsg("INFRA", kOptionIncorrectPara, kError, current_option->getName().c_str());
                    return kOptionIncorrectPara;
                }
            }

            current_option = opt;
            current_opt_finish = false;
            if (current_option->getType() == kBoolNoValue) {
                bool *data = (bool *)current_option->getTempData();
                *data = true;
                current_option->setTempIsSet(true);
                current_opt_finish = true;
            }
            // message->info("option name %s, data type %d \n", current_option->getName().c_str(), current_option->getType());
        } else {
            // message->info("in handle value\n");
            if (current_opt_finish == true) {
                Option *opt = getNextPositionalOption();  // add for positional option
                if (opt == nullptr) {
                    message->issueMsg("INFRA", kOptionOneValue, kError, argv);
                    return kOptionOneValue;
                }
                current_option = opt;
                current_opt_finish = false;
            }

            int result = parseValue(current_option, argv, total_num - objc);
            if (result != kSuccess)
                return result;
            current_opt_finish = true;
            current_option->setTempIsSet(true);
        }

        objv++;
        objc--;
        if (objc == 0) {  // when finish , check and set the last arg
            if (current_option) {
                OptionDataType type = current_option->getType();
                if (current_opt_finish == false && type != kIntList && type != kDoubleList && type != kStringList) {
                    message->issueMsg("INFRA", kOptionIncorrectPara, kError, current_option->getName().c_str());
                    return kOptionIncorrectPara;
                }
                current_option->setTempIsSet(true);
            }
        }
    }

    int res = postParse();
    if (res == kSuccess) {
        if (cb_) {
            bool res = cb_(this);
            if (res == false)
                return kUserLogicError;
        }
        setData();
    }

    return kSuccess;
}

int Command::parseValue(Option* opt, const char* argv, int argc) {
    switch (opt->getType()) {
    case OptionDataType::kNone:
        message->issueMsg("INFRA", kNoValueOption, kError, opt->getName().c_str());
        return kSuccess;
    case OptionDataType::kBoolNoValue:
    {
        break;
    }
    case OptionDataType::kBoolHasValue:
    {
        std::string str = argv;//Tcl_GetStringFromObj(objv, 0);
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        if (str == std::string("true") || !strcmp(argv, "1")) {
            // message->info("option get bool true\n");
            bool *data = (bool *)opt->getTempData();
            *data = true;
        } else if (str == std::string("false") || !strcmp(argv, "0")) {
            // message->info("option get bool false\n");
            bool *data = (bool *)opt->getTempData();
            *data = false;
        } else {
            message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
            return kDataNotSupport;
        }
        break;
    }
    case OptionDataType::kInt:
    {
        int *data = (int *)opt->getTempData();
        std::string sdata = argv;
        if (!isValidInt(sdata)) {
            message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
            return kDataNotSupport;
        }
        int tmp = atoi(argv);
        *data = tmp;
        break;
    }
    case OptionDataType::kDouble:
    {
        double *data = (double *)opt->getTempData();
        std::string sdata = argv;
        if (!isValidDouble(sdata)) {
            message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
            return kDataNotSupport;
        }
        double tmp = atof(argv);
        *data = tmp;
        break;
    }
    case OptionDataType::kString:
    {
        //message->info("in handle string\n");
        std::vector<std::pair<std::string, int>> *data = (std::vector<std::pair<std::string, int>> *)opt->getTempData();
        //*data = *argv;
        std::string incoming_string = argv;
        std::size_t pos = incoming_string.find_first_not_of(" ");
        incoming_string = incoming_string.substr(pos);
        pos = incoming_string.find_last_not_of(" ");
        incoming_string = incoming_string.substr(0, pos + 1);
        data->push_back(std::make_pair(incoming_string, argc));
        // message->info("option get string \n");
        break;
    }
    case OptionDataType::kEnum:
    {
        int *data = (int *)opt->getTempData();
        std::string incoming_string = argv;
        std::vector<std::string>* enums = opt->getEnums();
        if (enums == nullptr) {
            message->info("enumc get null ptr \n");
            return kDataNotSupport;
        }
        // message->info("enum has size %d\n", enums->size());
        std::vector<std::string>::iterator iter = find(enums->begin(), enums->end(), incoming_string);
        if (iter != enums->end()) {
            for (int i = 0; i < enums->size(); i++) {
                if (!strcmp(argv, enums->at(i).c_str())) {
                    // message->info("option get enum %s    %d \n", enums->at(i).c_str(), i);
                    *data = i;
                    break;
                }
            }
        } else {
            message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
            return kDataNotSupport;
        }
        break;
    }
    case OptionDataType::kPoint:
    {
        Point* data = (Point*) opt->getTempData();
        int value;
        std::string incoming_string = argv;
        std::size_t pos = incoming_string.find_first_not_of(" ");
        incoming_string = incoming_string.substr(pos);
        pos = incoming_string.find_last_not_of(" ");
        incoming_string = incoming_string.substr(0, pos + 1);
        pos = incoming_string.find(" ");
        if (pos != std::string::npos) { // input by {}
            if (pos == 0) {
                pos = incoming_string.find_first_not_of(" ");
                incoming_string = incoming_string.substr(pos);
            }
            std::string sub_string = incoming_string.substr(0, pos);
            int res = parseNum(sub_string, value);
            if (res != kSuccess) {
                message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
                return kDataNotSupport;
            }
            data->setX(value);

            incoming_string = incoming_string.substr(pos + 1);
            res = parseNum(incoming_string, value);
            if (res != kSuccess) {
                message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
                return kDataNotSupport;
            }
            data->setY(value);
            break;
        }
        message->issueMsg("INFRA", kOptionIncorrectPara, kError, opt->getName().c_str());
        return kOptionIncorrectPara;
    }
    case OptionDataType::kRect:
    {
        db::Box* data = (db::Box*) opt->getTempData();
        int value;
        std::string incoming_string = argv;
        std::size_t pos = incoming_string.find_first_not_of(" ");
        incoming_string = incoming_string.substr(pos);
        pos = incoming_string.find_last_not_of(" ");
        incoming_string = incoming_string.substr(0, pos + 1);
        pos = incoming_string.find(" ");
        current_num_ = 0;
        if (pos != std::string::npos) { // input by {}
            while (pos != std::string::npos) {
                if (pos == 0) {
                    pos = incoming_string.find_first_not_of(" ");
                    incoming_string = incoming_string.substr(pos);
                }
                std::string sub_string = incoming_string.substr(0, pos);
                int res = parseNum(sub_string, value);
                if (res != kSuccess) {
                    message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
                    return kDataNotSupport;
                }
                switch (current_num_) {
                case 0:
                    data->setLLX(value);
                    current_num_ = 1;
                    break;
                case 1:
                    data->setLLY(value);
                    current_num_ = 2;
                    break;
                case 2: {
                    data->setURX(value);
                    incoming_string = incoming_string.substr(pos + 1);
                    int res = parseNum(incoming_string, value);
                    if (res != kSuccess) {
                        message->issueMsg("INFRA", kDataNotSupport, kError, argv, opt->getName().c_str());
                        return kDataNotSupport;
                    }
                    data->setURY(value);
                    current_num_ = 4;
                    break;
                }
                default:
                    message->issueMsg("INFRA", kOptionOneValue, kError, argv);
                    return kOptionOneValue;
                }
                if (current_num_ < 4) {
                    incoming_string = incoming_string.substr(pos + 1);
                    pos = incoming_string.find(" ");
                } else {
                    break;
                }
            }
            if (current_num_ != 4) {
                message->issueMsg("INFRA", kOptionIncorrectPara, kError, opt->getName().c_str());
                return kOptionIncorrectPara;
            }
        } else {
            message->issueMsg("INFRA", kOptionOneValue, kError, argv);
            return kOptionOneValue;
        }
        break;
    }
    case OptionDataType::kIntList:
    {
        std::vector<int> *data = (std::vector<int> *)opt->getTempData();
        int value;
        std::string incoming_string = argv;
        std::size_t pos = incoming_string.find_first_not_of(" ");
        incoming_string = incoming_string.substr(pos);
        pos = incoming_string.find_last_not_of(" ");
        incoming_string = incoming_string.substr(0, pos + 1);
        pos = incoming_string.find(" ");
        while (pos != std::string::npos) {
            if (pos == 0) {
                pos = incoming_string.find_first_not_of(" ");
                incoming_string = incoming_string.substr(pos);
            }
            if (incoming_string == "") {
                break;
            }
            std::string sub_string = incoming_string.substr(0, pos);
            if (!isValidInt(sub_string)) {
                message->issueMsg("INFRA", kDataNotSupport, kError, sub_string.c_str(), opt->getName().c_str());
                return kDataNotSupport;
            }
            int value = atoi(sub_string.c_str());
            data->push_back(value);
            incoming_string = incoming_string.substr(pos + 1);
            pos = incoming_string.find(" ");
        }
        if (incoming_string == "") {
            break;
        }
        if (!isValidInt(incoming_string)) {
            message->issueMsg("INFRA", kDataNotSupport, kError, incoming_string.c_str(), opt->getName().c_str());
            return kDataNotSupport;
        }
        value = atoi(incoming_string.c_str());
        data->push_back(value);
        break;
    }
    case OptionDataType::kDoubleList:
    {
        std::vector<double> *data = (std::vector<double> *)opt->getTempData();
        double value;
        std::string incoming_string = argv;
        std::size_t pos = incoming_string.find_first_not_of(" ");
        incoming_string = incoming_string.substr(pos);
        pos = incoming_string.find_last_not_of(" ");
        incoming_string = incoming_string.substr(0, pos + 1);
        pos = incoming_string.find(" ");
        while (pos != std::string::npos) {
            if (pos == 0) {
                pos = incoming_string.find_first_not_of(" ");
                incoming_string = incoming_string.substr(pos);
            }
            if (incoming_string == "") {
                break;
            }
            std::string sub_string = incoming_string.substr(0, pos);
            if (!isValidDouble(sub_string)) {
                message->issueMsg("INFRA", kDataNotSupport, kError, sub_string.c_str(), opt->getName().c_str());
                return kDataNotSupport;
            }
            double value = atof(sub_string.c_str());
            data->push_back(value);
            incoming_string = incoming_string.substr(pos + 1);
            pos = incoming_string.find(" ");
        }
        if (incoming_string == "") {
            break;
        }
        if (!isValidDouble(incoming_string)) {
            message->issueMsg("INFRA", kDataNotSupport, kError, incoming_string.c_str(), opt->getName().c_str());
            return kDataNotSupport;
        }
        value = atof(incoming_string.c_str());
        data->push_back(value);
        break;
    }
    case OptionDataType::kStringList:
    {
        std::vector<std::string> *data = (std::vector<std::string> *)opt->getTempData();
        std::string incoming_string = argv;
        std::size_t pos = incoming_string.find_first_not_of(" ");
        incoming_string = incoming_string.substr(pos);
        pos = incoming_string.find_last_not_of(" ");
        incoming_string = incoming_string.substr(0, pos + 1);
        pos = incoming_string.find(" ");
        while (pos != std::string::npos) {
            if (pos == 0) {
                pos = incoming_string.find_first_not_of(" ");
                incoming_string = incoming_string.substr(pos);
            }
            std::string sub_string = incoming_string.substr(0, pos);
            data->push_back(sub_string);
            incoming_string = incoming_string.substr(pos + 1);
            pos = incoming_string.find(" ");
        }
        data->push_back(incoming_string);
        break;
    }
    default:
    {
        break;
    }
    }
    return kSuccess;
}

int Command::parseNum(std::string& data, int& value) {
    if (!strcmp(data.c_str(), "0")) {
        value = 0;
        return kSuccess;
    }
    int tmp = atoi(data.c_str());
    if (tmp != 0) {
        value = tmp;
        return kSuccess;
    }
    std::string digit;
    if (data[0] == '{' || data[0] == '(') {
        digit = data.substr(1);
    } else if (data[data.size() -1] == '}' || data[data.size() -1] == ')') {
        digit = data.substr(0, data.size() - 1);
    } else {
        // message->issueMsg("INFRA", 5, kError, data.c_str(), opt->getName().c_str());
        return kDataNotSupport;
    }

    tmp = atoi(digit.c_str());
    if (tmp != 0) {
        value = tmp;
        return kSuccess;
    } else {
        // message->issueMsg("INFRA", 5, kError, data.c_str(), opt->getName().c_str());
        return kDataNotSupport;
    }

    return kSuccess;
}

int Command::setData() {
    for (int i = 0; i < getOptionNum(); i++) {
        Option *opt = getOption(i);
        if (opt == nullptr) {
            message->issueMsg("INFRA", kGetOptionFail, kError);
        }
        if (opt->tempIsSet()) {
            switch (opt->getType()) {
            case OptionDataType::kNone:
                break;
            case OptionDataType::kBoolNoValue:
            case OptionDataType::kBoolHasValue:
            {
                bool *data = (bool *)opt->getTempData();
                *((bool *)opt->getData()) = *data;
                break;
            }
            case OptionDataType::kInt:
            {
                int *data = (int *)opt->getTempData();
                //message->info("temp int data %d \n", *data);
                *((int *)opt->getData()) = *data;
                break;
            }
            case OptionDataType::kDouble:
            {
                double *data = (double *)opt->getTempData();
                *((double *)opt->getData()) = *data;
                break;
            }
            case OptionDataType::kString:
            {
                // message->info("in handle string\n");
                //std::string *data = (std::string *)opt->getTempData();
                //*((std::string *)opt->getData()) = *data;
                std::vector<std::pair<std::string, int>> *temp_datas = (std::vector<std::pair<std::string, int>> *)opt->getTempData();
                std::vector<std::pair<std::string, int>> *data = (std::vector<std::pair<std::string, int>> *)opt->getData();
                data->clear();
                data->insert(data->begin(), temp_datas->begin(), temp_datas->end());
                break;
            }
            case OptionDataType::kEnum:
            {
                int *data = (int *)opt->getTempData();
                *((int *)opt->getData()) = *data;
                break;
            }
            case OptionDataType::kPoint:
            {
                Point *data = (Point *)opt->getTempData();
                // message->info("temp point data1 %d, data2 %d\n", data->getX(), data->getY());
                (*((Point *)opt->getData())).setX((*data).getX());
                (*((Point *)opt->getData())).setY((*data).getY());
                break;
            }
            case OptionDataType::kRect:
            {
                db::Box *data = (db::Box *)opt->getTempData();
                (*((db::Box *)opt->getData())).setLLX((*data).getLLX());
                (*((db::Box *)opt->getData())).setLLY((*data).getLLY());
                (*((db::Box *)opt->getData())).setURX((*data).getURX());
                (*((db::Box *)opt->getData())).setURY((*data).getURY());
                break;
            }
            case OptionDataType::kIntList:
            {
                std::vector<int> *temp_datas = (std::vector<int> *)opt->getTempData();
                std::vector<int> *data = (std::vector<int> *)opt->getData();
                data->clear();
                data->insert(data->begin(), temp_datas->begin(), temp_datas->end());
                break;
            }
            case OptionDataType::kDoubleList:
            {
                std::vector<double> *temp_datas = (std::vector<double> *)opt->getTempData();
                std::vector<double> *data = (std::vector<double> *)opt->getData();
                data->clear();
                data->insert(data->begin(), temp_datas->begin(), temp_datas->end());
                break;
            }
            case OptionDataType::kStringList:
            {
                std::vector<std::string> *temp_datas = (std::vector<std::string> *)opt->getTempData();
                std::vector<std::string> *data = (std::vector<std::string> *)opt->getData();
                data->clear();
                data->insert(data->begin(), temp_datas->begin(), temp_datas->end());
                break;
            }
            default:
            {
                break;
            }
            }
            opt->setTempIsSet(false);
            opt->setIsSet(true);
            if (opt->isSet()) {
                opt->setIsUserSet(true);
            }
            // opt->setIsUserSet(false);
        }
    }
    return kSuccess;
}

bool Command::isValidInt(std::string opt) {
  int len = static_cast<int>(opt.length());
  if (len == 0) {
    return false;
  }
  for (int i = 0; i < len; ++i) {
    if (!isdigit(opt[i])) {
      return false;
    }
  }
  return true;
}

bool Command::isValidDouble(std::string opt) {
  int len = static_cast<int>(opt.length());
  if (len == 0) {
    return false;
  }
  for (int i = 0; i < len; ++i) {
    if (!isdigit(opt[i]) && (opt[i] != '.') && (opt[i] != 'e') && (opt[i] != 'E') && (opt[i] != '+') && (opt[i] != '-')) {
      return false;
    }
  }
  return true;
}

}  // namespace infra
}  // namespace open_edi
