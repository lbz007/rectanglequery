/* @file  verilog_callback.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2021 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_PARSER_VERILOG_FRONTENDS_VERILOG_VERILOG_CALLBACK_H_
#define SRC_PARSER_VERILOG_FRONTENDS_VERILOG_VERILOG_CALLBACK_H_
#include "frontends/ast/ast.h"
namespace open_edi {
namespace verilog_parser {
typedef bool (*VerilogStringCbkFunc)(std::string &str);
typedef bool (*VerilogNodeCbkFunc)(Yosys::AST::AstNode *node);

class VerilogCallback {
  public:
    VerilogCallback() {}
    ~VerilogCallback() {}
    void setModuleCbk(VerilogStringCbkFunc module_cbk) {
        module_cbk_ = module_cbk;
    }
    VerilogStringCbkFunc getModuleCbk() { return module_cbk_; }
    
    void setPortCbk(VerilogNodeCbkFunc port_cbk) {
        port_cbk_ = port_cbk;
    }
    VerilogNodeCbkFunc getPortCbk() { return port_cbk_; }
    
    void setWireCbk(VerilogNodeCbkFunc wire_cbk) {
        wire_cbk_ = wire_cbk;
    }
    VerilogNodeCbkFunc getWireCbk() { return wire_cbk_; }
    
    void setInstCbk(VerilogNodeCbkFunc inst_cbk) {
        inst_cbk_ = inst_cbk;
    }
    VerilogNodeCbkFunc getInstCbk() { return inst_cbk_; }
    
    void setAssignCbk(VerilogNodeCbkFunc assign_cbk) {
        assign_cbk_ = assign_cbk;
    }
    VerilogNodeCbkFunc getAssignCbk() { return assign_cbk_; }
  private:
    VerilogStringCbkFunc module_cbk_;
    VerilogNodeCbkFunc   port_cbk_;
    VerilogNodeCbkFunc   wire_cbk_;
    VerilogNodeCbkFunc   inst_cbk_;
    VerilogNodeCbkFunc   assign_cbk_;
};

extern VerilogCallback kVerilogCallback;

}  // namespace verilog_parser
}  // namespace open_edi
#endif  // SRC_PARSER_VERILOG_FRONTENDS_VERILOG_VERILOG_CALLBACK_H_
