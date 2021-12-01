/* @file  command.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_INFRA_COMMAND_H_
#define SRC_INFRA_COMMAND_H_
#include <string>
#include <vector>
#include <map>
#include "option.h"
#include "util/util.h"
#include "db/util/box.h"
#include <tcl.h>

namespace open_edi {
namespace infra {
class Command;

typedef int(*registeredCallback)(Command* cmd);

//using namespace open_edi::util;

class Command {
 public:
    Command() {
    }
    ~Command() {}
    void setName(const char * v) { name_ = v;}
    std::string getName() const { return name_;}
    void setIsPublic(CommandAccessType v) { is_public_ = v;}
    CommandAccessType getIsPublic() const { return is_public_;}
    void setDescription(const char * v) { description_ = v;}
    std::string getDescription() const { return description_;}
    void addOption(Option* v);
    int getOptionNum() const { return options_.size();}
    Option* getOption(int i) { return options_.at(i);}
    Option* getOption(const char * name);
    void addGroup(OptionGroup* v);
    int getGroupNum() const { return opt_groups_.size();}
    OptionGroup* getGroup(int i) { return opt_groups_.at(i);}
    Option* createOption(const char * name, OptionDataType type);
    bool isOptionSet(const char * name);
    bool getOptionValue(const char * name, bool& value);
    bool getOptionValue(const char * name, int& value);
    bool getOptionValue(const char * name, double& value);
    bool getOptionValue(const char * name, std::string& value);
    bool getOptionValue(const char * name, Point** value);
    bool getOptionValue(const char * name, Point& value);
    bool getOptionValue(const char * name, db::Box** value);
    bool getOptionValue(const char * name, db::Box& value);
    bool getOptionValue(const char * name, std::vector<std::string>** value);
    bool getOptionValue(const char * name, std::vector<std::string>& value);
    bool getOptionValue(const char * name, std::vector<std::pair<std::string, int>>& value);
    bool getOptionValue(const char * name, std::vector<int>** value); 
    bool getOptionValue(const char * name, std::vector<int>& value);
    bool getOptionValue(const char * name, std::vector<double>** value); 
    bool getOptionValue(const char * name, std::vector<double>& value);

    int preParse();
    int postParse();
    int setData();
    int parser(int argc, const char** argv);
    int objParser(int objc, Tcl_Obj* CONST objv[]);
    int parseValue(Option* opt, const char* argv, int argc);
    int parseNum(std::string& data, int& value);
    
    const std::map<std::string, Option*>* getOptionsByOrder() const {return &options_by_order_;}
    void setCallback(std::function<bool(Command*)> v) { cb_ = v;}
    Option* getNextPositionalOption();
    void setRegisterCallback(registeredCallback v) { register_cb_ = v;}
    registeredCallback getRegisterCallback() const { return register_cb_;}
    Tcl_Interp* getTclInterp() const { return current_interp_;}
    void setTclInterp(Tcl_Interp* itp) { current_interp_ = itp;}
    bool useProcessBar() const { return use_process_bar_;}
    void setUseProcessBar(bool value) { use_process_bar_ = value;}
    std::string getCurrentCmd() const { return current_cmd_args_;}

 private:
    bool isValidInt(std::string opt);
    bool isValidDouble(std::string opt);
    std::string name_;
    CommandAccessType is_public_;
    std::string description_;
    std::vector<Option*> options_;
    std::map<std::string, int> option_names_;  // use to map< option name, option vetor index>
    std::map<std::string, Option*> options_by_order_;
    std::vector<OptionGroup*> opt_groups_;
    bool use_process_bar_;
    Tcl_Interp *current_interp_;
    std::string current_cmd_args_;

    int current_num_;   // internal use for parse data
    std::function<bool(Command*)> cb_;
    registeredCallback register_cb_;
};

}  // namespace infra 
}  // namespace open_edi
#endif  // SRC_INFRA_COMMAND_H_
