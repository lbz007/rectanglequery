/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  ---
 *
 *  The Verilog frontend.
 *
 *  This frontend is using the AST frontend library (see frontends/ast/).
 *  Thus this frontend does not generate RTLIL code directly but creates an
 *  AST directly from the Verilog parse tree and then passes this AST to
 *  the AST frontend library.
 *
 */

#include "verilog_frontend.h"
#include "preproc.h"
#include "libs/sha1/sha1.h"
#include <stdarg.h>
#include <queue>

#include "db/io/read_verilog.h"
#include "util/monitor.h"

YOSYS_NAMESPACE_BEGIN
using namespace VERILOG_FRONTEND;
using namespace open_edi;

// use the Verilog bison/flex parser to generate an AST and use AST::process() to convert it to RTLIL

static std::vector<std::string> verilog_defaults;
static std::list<std::vector<std::string>> verilog_defaults_stack;

static void error_on_dpi_function(AST::AstNode *node)
{
	if (node->type == AST::AST_DPI_FUNCTION)
		log_error("Found DPI function %s.\n", node->str.c_str());
	for (auto child : node->children)
		error_on_dpi_function(child);
}

static void add_package_types(dict<std::string, AST::AstNode *> &user_types, std::vector<AST::AstNode *> &package_list)
{
	// prime the parser's user type lookup table with the package qualified names
	// of typedefed names in the packages seen so far.
	for (const auto &pkg : package_list) {
		log_assert(pkg->type==AST::AST_PACKAGE);
		for (const auto &node: pkg->children) {
			if (node->type == AST::AST_TYPEDEF) {
				std::string s = pkg->str + "::" + node->str.substr(1);
				user_types[s] = node;
			}
		}
	}
	user_type_stack.clear();
	user_type_stack.push_back(new UserTypeMap());
}

struct VerilogFrontend : public Frontend {
	VerilogFrontend() : Frontend("verilog", "read modules from Verilog file") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    read_verilog [filename]\n");
		log("\n");
		log("Load modules from a Verilog file to the current design. A large subset of\n");
		log("Verilog-2005 is supported.\n");
		log("\n");
	}
        int getAstNodeNum(AST::AstNode *root) {
            int total_num_ = 0;
            std::queue<Yosys::AST::AstNode*> ast_nodes;
            ast_nodes.push(root);

            struct Yosys::AST::AstNode *node = nullptr;
            while (!ast_nodes.empty()) {
                node = ast_nodes.front();
                ast_nodes.pop();
                total_num_++;
                for (auto child : node->children) {
                    ast_nodes.push(child);
                }
            }
            return total_num_;
        }

        void execute(std::istream *&f, std::string filename, std::vector<std::string> args, RTLIL::Design *design) override
	{
            AST::current_filename = filename;
            AST::set_line_num = &frontend_verilog_yyset_lineno;
            AST::get_line_num = &frontend_verilog_yyget_lineno;

            open_edi::util::IOManager io_manager;
            if (false == io_manager.open(filename.c_str(), "r")) {
                printf("Open file %s fail\n", filename.c_str());
                return;
            }

            lexin = &io_manager;
            current_ast = new AST::AstNode(AST::AST_DESIGN);
            frontend_verilog_yyrestart(NULL);
            frontend_verilog_yyparse();
            frontend_verilog_yylex_destroy();
	}
} VerilogFrontend;

struct VerilogDefaults : public Pass {
	VerilogDefaults() : Pass("verilog_defaults", "set default options for read_verilog") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    verilog_defaults -add [options]\n");
		log("\n");
		log("Add the specified options to the list of default options to read_verilog.\n");
		log("\n");
		log("\n");
		log("    verilog_defaults -clear\n");
		log("\n");
		log("Clear the list of Verilog default options.\n");
		log("\n");
		log("\n");
		log("    verilog_defaults -push\n");
		log("    verilog_defaults -pop\n");
		log("\n");
		log("Push or pop the list of default options to a stack. Note that -push does\n");
		log("not imply -clear.\n");
		log("\n");
	}
	void execute(std::vector<std::string> args, RTLIL::Design*) override
	{
		if (args.size() < 2)
			cmd_error(args, 1, "Missing argument.");

		if (args[1] == "-add") {
			verilog_defaults.insert(verilog_defaults.end(), args.begin()+2, args.end());
			return;
		}

		if (args.size() != 2)
			cmd_error(args, 2, "Extra argument.");

		if (args[1] == "-clear") {
			verilog_defaults.clear();
			return;
		}

		if (args[1] == "-push") {
			verilog_defaults_stack.push_back(verilog_defaults);
			return;
		}

		if (args[1] == "-pop") {
			if (verilog_defaults_stack.empty()) {
				verilog_defaults.clear();
			} else {
				verilog_defaults.swap(verilog_defaults_stack.back());
				verilog_defaults_stack.pop_back();
			}
			return;
		}
	}
} VerilogDefaults;

struct VerilogDefines : public Pass {
	VerilogDefines() : Pass("verilog_defines", "define and undefine verilog defines") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    verilog_defines [options]\n");
		log("\n");
		log("Define and undefine verilog preprocessor macros.\n");
		log("\n");
		log("    -Dname[=definition]\n");
		log("        define the preprocessor symbol 'name' and set its optional value\n");
		log("        'definition'\n");
		log("\n");
		log("    -Uname[=definition]\n");
		log("        undefine the preprocessor symbol 'name'\n");
		log("\n");
		log("    -reset\n");
		log("        clear list of defined preprocessor symbols\n");
		log("\n");
		log("    -list\n");
		log("        list currently defined preprocessor symbols\n");
		log("\n");
	}
	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++) {
			std::string arg = args[argidx];
			if (arg == "-D" && argidx+1 < args.size()) {
				std::string name = args[++argidx], value;
				size_t equal = name.find('=');
				if (equal != std::string::npos) {
					value = name.substr(equal+1);
					name = name.substr(0, equal);
				}
				design->verilog_defines->add(name, value);
				continue;
			}
			if (arg.compare(0, 2, "-D") == 0) {
				size_t equal = arg.find('=', 2);
				std::string name = arg.substr(2, equal-2);
				std::string value;
				if (equal != std::string::npos)
					value = arg.substr(equal+1);
				design->verilog_defines->add(name, value);
				continue;
			}
			if (arg == "-U" && argidx+1 < args.size()) {
				std::string name = args[++argidx];
				design->verilog_defines->erase(name);
				continue;
			}
			if (arg.compare(0, 2, "-U") == 0) {
				std::string name = arg.substr(2);
				design->verilog_defines->erase(name);
				continue;
			}
			if (arg == "-reset") {
				design->verilog_defines->clear();
				continue;
			}
			if (arg == "-list") {
				design->verilog_defines->log();
				continue;
			}
			break;
		}

		if (args.size() != argidx)
			cmd_error(args, argidx, "Extra argument.");
	}
} VerilogDefines;

YOSYS_NAMESPACE_END

// the yyerror function used by bison to report parser errors
void frontend_verilog_yyerror(char const *fmt, ...)
{
	va_list ap;
	char buffer[1024];
	char *p = buffer;
	va_start(ap, fmt);
	p += vsnprintf(p, buffer + sizeof(buffer) - p, fmt, ap);
	va_end(ap);
	p += snprintf(p, buffer + sizeof(buffer) - p, "\n");
	YOSYS_NAMESPACE_PREFIX log_file_error(YOSYS_NAMESPACE_PREFIX AST::current_filename, frontend_verilog_yyget_lineno(),
					      "%s", buffer);
	//exit(1);
}
