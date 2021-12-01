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
 *  ---
 *
 *  This is the actual bison parser for Verilog code. The AST ist created directly
 *  from the bison reduce functions here. Note that this code uses a few global
 *  variables to hold the state of the AST generator and therefore this parser is
 *  not reentrant.
 *
 */

%{
#include <list>
#include <stack>
#include <string.h>
#include "frontends/verilog/verilog_frontend.h"
#include "frontends/verilog/verilog_callback.h"
#include "frontends/verilog/verilog.tab.hpp"
#include "kernel/log.h"
//#include "db/core/db.h"

#define YYLEX_PARAM &yylval, &yylloc

USING_YOSYS_NAMESPACE
using namespace AST;
using namespace VERILOG_FRONTEND;
using namespace open_edi;

YOSYS_NAMESPACE_BEGIN
namespace VERILOG_FRONTEND {
	int port_counter;
	dict<std::string, int> port_stubs;
	dict<IdString, AstNode*> *attr_list, default_attr_list;
	std::stack<dict<IdString, AstNode*> *> attr_list_stack;
	dict<IdString, AstNode*> *albuf;
	std::vector<UserTypeMap*> user_type_stack;
	dict<std::string, AstNode*> pkg_user_types;
	std::vector<AstNode*> ast_stack;
	struct AstNode *astbuf1, *astbuf2, *astbuf3;
	struct AstNode *current_function_or_task;
	struct AstNode *current_ast, *current_ast_mod;
	int current_function_or_task_port_id;
	std::vector<char> case_type_stack;
	bool do_not_require_port_stubs;
	bool default_nettype_wire;
	bool sv_mode, formal_mode, lib_mode, specify_mode;
	bool noassert_mode, noassume_mode, norestrict_mode;
	bool assume_asserts_mode, assert_assumes_mode;
	bool current_wire_rand, current_wire_const;
	bool current_modport_input, current_modport_output;
	//std::istream *lexin;
	util::IOManager *lexin;
}
YOSYS_NAMESPACE_END

#define SET_RULE_LOC(LHS, BEGIN, END) \
    do { (LHS).first_line = (BEGIN).first_line; \
    (LHS).first_column = (BEGIN).first_column; \
    (LHS).last_line = (END).last_line; \
    (LHS).last_column = (END).last_column; } while(0)

// Macro to describe how we handle a callback.
// If the function was set then call it.
// If the function returns non zero then there was an error
// so call the error routine and exit.
#define CALLBACK(func, arg) \
    do {  \
        if (func) { \
            if ((*func)(arg)) { \
            } else { \
                frontend_verilog_yyerror("Callback function failed.\n"); \
                exit(1); \
            } \
        } \
    } while(0)

int frontend_verilog_yylex(YYSTYPE *yylval_param, YYLTYPE *yyloc_param);

struct specify_target {
	char polarity_op;
	AstNode *dst, *dat;
};

struct specify_triple {
	AstNode *t_min, *t_avg, *t_max;
};

struct specify_rise_fall {
	specify_triple rise;
	specify_triple fall;
};

static void addTypedefNode(std::string *name, AstNode *node)
{
        return;
	log_assert(node);
	auto *tnode = new AstNode(AST_TYPEDEF, node);
	tnode->str = *name;
	auto user_types = user_type_stack.back();
	(*user_types)[*name] = tnode;
	if (current_ast_mod && current_ast_mod->type == AST_PACKAGE) {
		// typedef inside a package so we need the qualified name
		auto qname = current_ast_mod->str + "::" + (*name).substr(1);
		pkg_user_types[qname] = tnode;
	}
	delete name;
	ast_stack.back()->children.push_back(tnode);
}

static void enterTypeScope()
{
	auto user_types = new UserTypeMap();
	user_type_stack.push_back(user_types);
}

static void exitTypeScope()
{
	user_type_stack.pop_back();
}

static bool isInLocalScope(const std::string *name)
{
	// tests if a name was declared in the current block scope
	auto user_types = user_type_stack.back();
	return (user_types->count(*name) > 0);
}

static AstNode *getTypeDefinitionNode(std::string type_name)
{
	// return the definition nodes from the typedef statement
	auto user_types = user_type_stack.back();
	log_assert(user_types->count(type_name) > 0);
	auto typedef_node = (*user_types)[type_name];
	log_assert(typedef_node->type == AST_TYPEDEF);
	return typedef_node->children[0];
}

static AstNode *copyTypeDefinition(std::string type_name)
{
	// return a copy of the template from a typedef definition
	auto typedef_node = getTypeDefinitionNode(type_name);
	return typedef_node->clone();
}

static AstNode *makeRange(int msb = 31, int lsb = 0, bool isSigned = true)
{
	auto range = new AstNode(AST_RANGE);
	range->children.push_back(AstNode::mkconst_int(msb, true));
	range->children.push_back(AstNode::mkconst_int(lsb, true));
	range->is_signed = isSigned;
	return range;
}

static void addRange(AstNode *parent, int msb = 31, int lsb = 0, bool isSigned = true)
{
	auto range = makeRange(msb, lsb, isSigned);
	parent->children.push_back(range);
}

static AstNode *checkRange(AstNode *type_node, AstNode *range_node)
{
	if (type_node->range_left >= 0 && type_node->range_right >= 0) {
		// type already restricts the range
		if (range_node) {
			frontend_verilog_yyerror("integer/genvar types cannot have packed dimensions.");
		}
		else {
			range_node = makeRange(type_node->range_left, type_node->range_right, false);
		}
	}
	if (range_node && range_node->children.size() != 2) {
		frontend_verilog_yyerror("wire/reg/logic packed dimension must be of the form: [<expr>:<expr>], [<expr>+:<expr>], or [<expr>-:<expr>]");
	}
	return range_node;
}

static void rewriteAsMemoryNode(AstNode *node, AstNode *rangeNode)
{
	node->type = AST_MEMORY;
	if (rangeNode->type == AST_RANGE && rangeNode->children.size() == 1) {
		// SV array size [n], rewrite as [n-1:0]
		rangeNode->children[0] = new AstNode(AST_SUB, rangeNode->children[0], AstNode::mkconst_int(1, true));
		rangeNode->children.push_back(AstNode::mkconst_int(0, false));
	}
	node->children.push_back(rangeNode);
}

%}

%define api.prefix {frontend_verilog_yy}
%define api.pure

/* The union is defined in the header, so we need to provide all the
 * includes it requires
 */
%code requires {
#include <map>
#include <string>
#include "frontends/verilog/verilog_frontend.h"
}

%union {
	std::string *string;
	struct YOSYS_NAMESPACE_PREFIX AST::AstNode *ast;
	YOSYS_NAMESPACE_PREFIX dict<YOSYS_NAMESPACE_PREFIX RTLIL::IdString, YOSYS_NAMESPACE_PREFIX AST::AstNode*> *al;
	struct specify_target *specify_target_ptr;
	struct specify_triple *specify_triple_ptr;
	struct specify_rise_fall *specify_rise_fall_ptr;
	bool boolean;
	char ch;
}

%token <string> TOK_STRING TOK_ID TOK_CONSTVAL TOK_REALVAL TOK_PRIMITIVE
%token <string> TOK_SVA_LABEL TOK_SPECIFY_OPER TOK_MSG_TASKS
%token <string> TOK_BASE TOK_BASED_CONSTVAL TOK_UNBASED_UNSIZED_CONSTVAL
%token <string> TOK_USER_TYPE TOK_PKG_USER_TYPE
%token TOK_ASSERT TOK_ASSUME TOK_RESTRICT TOK_COVER TOK_FINAL
%token ATTR_BEGIN ATTR_END DEFATTR_BEGIN DEFATTR_END
%token TOK_MODULE TOK_ENDMODULE TOK_PARAMETER TOK_LOCALPARAM TOK_DEFPARAM
%token TOK_PACKAGE TOK_ENDPACKAGE TOK_PACKAGESEP
%token TOK_INTERFACE TOK_ENDINTERFACE TOK_MODPORT TOK_VAR TOK_WILDCARD_CONNECT
%token TOK_INPUT TOK_OUTPUT TOK_INOUT TOK_WIRE TOK_TRI TOK_WAND TOK_WOR TOK_REG TOK_LOGIC
%token TOK_INTEGER TOK_SIGNED TOK_ASSIGN TOK_PLUS_ASSIGN TOK_ALWAYS TOK_INITIAL
%token TOK_ALWAYS_FF TOK_ALWAYS_COMB TOK_ALWAYS_LATCH
%token TOK_BEGIN TOK_END TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_REPEAT
%token TOK_DPI_FUNCTION TOK_POSEDGE TOK_NEGEDGE TOK_OR TOK_AUTOMATIC
%token TOK_CASE TOK_CASEX TOK_CASEZ TOK_ENDCASE TOK_DEFAULT
%token TOK_FUNCTION TOK_ENDFUNCTION TOK_TASK TOK_ENDTASK TOK_SPECIFY
%token TOK_IGNORED_SPECIFY TOK_ENDSPECIFY TOK_SPECPARAM TOK_SPECIFY_AND TOK_IGNORED_SPECIFY_AND
%token TOK_GENERATE TOK_ENDGENERATE TOK_GENVAR TOK_REAL
%token TOK_SYNOPSYS_FULL_CASE TOK_SYNOPSYS_PARALLEL_CASE
%token TOK_SUPPLY0 TOK_SUPPLY1 TOK_TO_SIGNED TOK_TO_UNSIGNED
%token TOK_POS_INDEXED TOK_NEG_INDEXED TOK_PROPERTY TOK_ENUM TOK_TYPEDEF
%token TOK_RAND TOK_CONST TOK_CHECKER TOK_ENDCHECKER TOK_EVENTUALLY
%token TOK_INCREMENT TOK_DECREMENT TOK_UNIQUE TOK_PRIORITY
%token TOK_STRUCT TOK_PACKED TOK_UNSIGNED TOK_INT TOK_BYTE TOK_SHORTINT TOK_UNION
%token TOK_OR_ASSIGN TOK_XOR_ASSIGN TOK_AND_ASSIGN TOK_SUB_ASSIGN

%type <ast> range range_or_multirange  non_opt_range non_opt_multirange range_or_signed_int
%type <ast> wire_type expr basic_expr concat_list rvalue lvalue lvalue_concat_list
%type <string> opt_label opt_sva_label tok_prim_wrapper hierarchical_id hierarchical_type_id integral_number
%type <string> type_name
%type <ast> opt_enum_init enum_type struct_type non_wire_data_type
%type <boolean> opt_signed opt_property unique_case_attr always_comb_or_latch always_or_always_ff
%type <al> attr case_attr
%type <ast> struct_union

%type <specify_target_ptr> specify_target
%type <specify_triple_ptr> specify_triple specify_opt_triple
%type <specify_rise_fall_ptr> specify_rise_fall
%type <ast> specify_if specify_condition
%type <ch> specify_edge

// operator precedence from low to high
%left OP_LOR
%left OP_LAND
%left '|' OP_NOR
%left '^' OP_XNOR
%left '&' OP_NAND
%left OP_EQ OP_NE OP_EQX OP_NEX
%left '<' OP_LE OP_GE '>'
%left OP_SHL OP_SHR OP_SSHL OP_SSHR
%left '+' '-'
%left '*' '/' '%'
%left OP_POW
%precedence OP_CAST
%precedence UNARY_OPS

%define parse.error verbose
%define parse.lac full

%precedence FAKE_THEN
%precedence TOK_ELSE

%debug
%locations

%%

input: {
	ast_stack.clear();
	ast_stack.push_back(current_ast);
} design {
	ast_stack.pop_back();
	//log_assert(GetSize(ast_stack) == 0);
	for (auto &it : default_attr_list)
		delete it.second;
	default_attr_list.clear();
};

design:
	module design |
	defattr design |
	task_func_decl design |
	param_decl design |
	localparam_decl design |
	typedef_decl design |
	package design |
	interface design |
	%empty;

attr:
	{
	} attr_opt {
	};

attr_opt: %empty;

defattr:
	DEFATTR_BEGIN {
	} opt_attr_list {
	} DEFATTR_END;

opt_attr_list:
	attr_list | %empty;

attr_list:
	attr_assign |
	attr_list ',' attr_assign;

attr_assign:
	hierarchical_id {
	} |
	hierarchical_id '=' expr {
	};

hierarchical_id:
	TOK_ID {
		$$ = $1;
	} |
	hierarchical_id TOK_PACKAGESEP TOK_ID {
		if ($3->compare(0, 1, "\\") == 0)
			*$1 += "::" + $3->substr(1);
		else
			*$1 += "::" + *$3;
		delete $3;
		$$ = $1;
	} |
	hierarchical_id '.' TOK_ID {
		if ($3->compare(0, 1, "\\") == 0)
			*$1 += "." + $3->substr(1);
		else
			*$1 += "." + *$3;
		delete $3;
		$$ = $1;
	};

hierarchical_type_id:
	TOK_USER_TYPE
	| TOK_PKG_USER_TYPE				// package qualified type name
	| '(' TOK_USER_TYPE ')'	{ $$ = $2; }		// non-standard grammar
	;

module:
	attr TOK_MODULE {
	} TOK_ID {
		do_not_require_port_stubs = false;
		port_stubs.clear();
		port_counter = 0;
                CALLBACK(verilog_parser::kVerilogCallback.getModuleCbk(), *$4);
		delete $4;
	} module_para_opt module_args_opt ';' module_body TOK_ENDMODULE opt_label {
		if (port_stubs.size() != 0)
			frontend_verilog_yyerror("Missing details for module port `%s'.",
					port_stubs.begin()->first.c_str());
		//ast_stack.pop_back();
		//log_assert(ast_stack.size() == 1);
		current_ast_mod = NULL;
		exitTypeScope();
	};

module_para_opt:
	'#' '(' { astbuf1 = nullptr; } module_para_list { if (astbuf1) delete astbuf1; } ')' | %empty;

module_para_list:
	single_module_para | module_para_list ',' single_module_para;

single_module_para:
	%empty |
	attr TOK_PARAMETER {
		if (astbuf1) delete astbuf1;
		astbuf1 = new AstNode(AST_PARAMETER);
		astbuf1->children.push_back(AstNode::mkconst_int(0, true));
	} param_type single_param_decl |
	attr TOK_LOCALPARAM {
		if (astbuf1) delete astbuf1;
		astbuf1 = new AstNode(AST_LOCALPARAM);
		astbuf1->children.push_back(AstNode::mkconst_int(0, true));
	} param_type single_param_decl |
	single_param_decl;

module_args_opt:
	'(' ')' | %empty | '(' module_args optional_comma ')';

module_args:
	module_arg | module_args ',' module_arg;

optional_comma:
	',' | %empty;

module_arg_opt_assignment:
	'=' expr { } |
	%empty;

module_arg:
	TOK_ID {
                if (port_stubs.count(*$1) != 0)
                        frontend_verilog_yyerror("Duplicate module port `%s'.", $1->c_str());
                port_stubs[*$1] = ++port_counter;
		delete $1;
	} module_arg_opt_assignment |
	TOK_ID {
	} TOK_ID {  /* SV interfaces */
	} module_arg_opt_assignment |
	attr wire_type range TOK_ID {
		AstNode *node = $2;
		node->str = *$4;
		node->port_id = ++port_counter;
		if ($3 != NULL)
			node->children.push_back($3);
		if (!node->is_input && !node->is_output)
			frontend_verilog_yyerror("Module port `%s' is neither input nor output.", $4->c_str());
		if (node->is_reg && node->is_input && !node->is_output && !sv_mode)
			frontend_verilog_yyerror("Input port `%s' is declared as register.", $4->c_str());
                delete node;
		delete $4;
	} module_arg_opt_assignment |
	'.' '.' '.' {
		do_not_require_port_stubs = true;
	};

package:
	attr TOK_PACKAGE {
	} TOK_ID {
	} ';' package_body TOK_ENDPACKAGE opt_label {
	};

package_body:
	package_body package_body_stmt | %empty;

package_body_stmt:
	typedef_decl | localparam_decl | param_decl;

interface:
	TOK_INTERFACE {
	} TOK_ID {
	} module_para_opt module_args_opt ';' interface_body TOK_ENDINTERFACE {
	};

interface_body:
	interface_body interface_body_stmt | %empty;

interface_body_stmt:
	param_decl | localparam_decl | typedef_decl | defparam_decl | wire_decl | always_stmt | assign_stmt |
	modport_stmt;

non_opt_delay:
	'#' TOK_ID { delete $2; } |
	'#' TOK_CONSTVAL { delete $2; } |
	'#' TOK_REALVAL { delete $2; } |
	'#' '(' expr ')' { delete $3; } |
	'#' '(' expr ':' expr ':' expr ')' { delete $3; delete $5; delete $7; };

delay:
	non_opt_delay | %empty;

wire_type:
	{
		astbuf3 = new AstNode(AST_WIRE);
		current_wire_rand = false;
		current_wire_const = false;
	} wire_type_token_list {
		$$ = astbuf3;
	};

wire_type_token_list:
	wire_type_token |
	wire_type_token_list wire_type_token |
	wire_type_token_io |
	hierarchical_type_id {
	};

wire_type_token_io:
	TOK_INPUT {
		astbuf3->is_input = true;
	} |
	TOK_OUTPUT {
		astbuf3->is_output = true;
	} |
	TOK_INOUT {
		astbuf3->is_input = true;
		astbuf3->is_output = true;
	};

wire_type_token:
	TOK_WIRE {
	} |
        TOK_TRI {
		astbuf3->is_tri = true;
        } |
	TOK_WOR {
		astbuf3->is_wor = true;
	} |
	TOK_WAND {
		astbuf3->is_wand = true;
	} |
	TOK_REG {
		astbuf3->is_reg = true;
	} |
	TOK_LOGIC {
		astbuf3->is_logic = true;
	} |
	TOK_VAR {
		astbuf3->is_logic = true;
	} |
	TOK_INTEGER {
		astbuf3->is_reg = true;
		astbuf3->range_left = 31;
		astbuf3->range_right = 0;
		astbuf3->is_signed = true;
	} |
	TOK_GENVAR {
		astbuf3->type = AST_GENVAR;
		astbuf3->is_reg = true;
		astbuf3->is_signed = true;
		astbuf3->range_left = 31;
		astbuf3->range_right = 0;
	} |
	TOK_SIGNED {
		astbuf3->is_signed = true;
	} |
	TOK_RAND {
		current_wire_rand = true;
	} |
	TOK_CONST {
		current_wire_const = true;
	};

non_opt_range:
	'[' expr ':' expr ']' {
		$$ = new AstNode(AST_RANGE);
		$$->children.push_back($2);
		$$->children.push_back($4);
	} |
	'[' expr TOK_POS_INDEXED expr ']' {
		$$ = new AstNode(AST_RANGE);
		AstNode *expr = new AstNode(AST_SELFSZ, $2);
		$$->children.push_back(new AstNode(AST_SUB, new AstNode(AST_ADD, expr->clone(), $4), AstNode::mkconst_int(1, true)));
		$$->children.push_back(new AstNode(AST_ADD, expr, AstNode::mkconst_int(0, true)));
	} |
	'[' expr TOK_NEG_INDEXED expr ']' {
		$$ = new AstNode(AST_RANGE);
		AstNode *expr = new AstNode(AST_SELFSZ, $2);
		$$->children.push_back(new AstNode(AST_ADD, expr, AstNode::mkconst_int(0, true)));
		$$->children.push_back(new AstNode(AST_SUB, new AstNode(AST_ADD, expr->clone(), AstNode::mkconst_int(1, true)), $4));
	} |
	'[' expr ']' {
		$$ = new AstNode(AST_RANGE);
		$$->children.push_back($2);
	};

non_opt_multirange:
	non_opt_range non_opt_range {
		$$ = new AstNode(AST_MULTIRANGE, $1, $2);
	} |
	non_opt_multirange non_opt_range {
		$$ = $1;
		$$->children.push_back($2);
	};

range:
	non_opt_range {
		$$ = $1;
	} |
	%empty {
		$$ = NULL;
	};

range_or_multirange:
	range { $$ = $1; } |
	non_opt_multirange { $$ = $1; };

range_or_signed_int:
	  range 		{ $$ = $1; }
	| TOK_INTEGER		{ $$ = makeRange(); }
	;

module_body:
	module_body module_body_stmt |
	/* the following line makes the generate..endgenrate keywords optional */
	module_body gen_stmt |
	module_body ';' |
	%empty;

module_body_stmt:
	task_func_decl | specify_block | param_decl | localparam_decl | typedef_decl | defparam_decl | specparam_declaration | wire_decl | assign_stmt | cell_stmt |
	enum_decl | struct_decl |
	always_stmt | TOK_GENERATE module_gen_body TOK_ENDGENERATE | defattr | assert_property | checker_decl | ignored_specify_block;

checker_decl:
	TOK_CHECKER TOK_ID ';' {
	} module_body TOK_ENDCHECKER {
	};

task_func_decl:
	attr TOK_DPI_FUNCTION TOK_ID TOK_ID {
	} opt_dpi_function_args ';' {
	} |
	attr TOK_DPI_FUNCTION TOK_ID '=' TOK_ID TOK_ID {
	} opt_dpi_function_args ';' {
	} |
	attr TOK_DPI_FUNCTION TOK_ID ':' TOK_ID '=' TOK_ID TOK_ID {
	} opt_dpi_function_args ';' {
	} |
	attr TOK_TASK opt_automatic TOK_ID {
	} task_func_args_opt ';' task_func_body TOK_ENDTASK {
	} |
	attr TOK_FUNCTION opt_automatic opt_signed range_or_signed_int TOK_ID {
	} task_func_args_opt ';' task_func_body TOK_ENDFUNCTION {
	};

dpi_function_arg:
	TOK_ID TOK_ID {
	} |
	TOK_ID {
	};

opt_dpi_function_args:
	'(' dpi_function_args ')' |
	%empty;

dpi_function_args:
	dpi_function_args ',' dpi_function_arg |
	dpi_function_args ',' |
	dpi_function_arg |
	%empty;

opt_automatic:
	TOK_AUTOMATIC |
	%empty;

opt_signed:
	TOK_SIGNED {
		$$ = true;
	} |
	%empty {
		$$ = false;
	};

task_func_args_opt:
	'(' ')' | %empty | '(' {
	} task_func_args optional_comma {
	} ')';

task_func_args:
	task_func_port | task_func_args ',' task_func_port;

task_func_port:
	attr wire_type range {
	} wire_name |
	{
	} wire_name;

task_func_body:
	task_func_body behavioral_stmt |
	%empty;

/*************************** specify parser ***************************/

specify_block:
	TOK_SPECIFY specify_item_list TOK_ENDSPECIFY;

specify_item_list:
	specify_item specify_item_list |
	%empty;

specify_item:
	specify_if '(' specify_edge expr TOK_SPECIFY_OPER specify_target ')' '=' specify_rise_fall ';' {
	} |
	TOK_ID '(' specify_edge expr specify_condition ',' specify_edge expr specify_condition ',' specify_triple specify_opt_triple ')' ';' {
	};

specify_opt_triple:
	',' specify_triple {
	} |
	%empty {
	};

specify_if:
	TOK_IF '(' expr ')' {
	} |
	%empty {
	};

specify_condition:
	TOK_SPECIFY_AND expr {
	} |
	%empty {
	};

specify_target:
	expr {
	} |
	'(' expr ':' expr ')'{
	} |
	'(' expr TOK_NEG_INDEXED expr ')'{
	} |
	'(' expr TOK_POS_INDEXED expr ')'{
	};

specify_edge:
	TOK_POSEDGE { $$ = 'p'; } |
	TOK_NEGEDGE { $$ = 'n'; } |
	%empty { $$ = 0; };

specify_rise_fall:
	specify_triple {
	} |
	'(' specify_triple ',' specify_triple ')' {
	} |
	'(' specify_triple ',' specify_triple ',' specify_triple ')' {
	} |
	'(' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ')' {
	} |
	'(' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ',' specify_triple ')' {
	}

specify_triple:
	expr {
	} |
	expr ':' expr ':' expr {
	};

/******************** ignored specify parser **************************/

ignored_specify_block:
	TOK_IGNORED_SPECIFY ignored_specify_item_opt TOK_ENDSPECIFY |
	TOK_IGNORED_SPECIFY TOK_ENDSPECIFY ;

ignored_specify_item_opt:
	ignored_specify_item_opt ignored_specify_item |
	ignored_specify_item ;

ignored_specify_item:
	specparam_declaration
	// | pulsestyle_declaration
	// | showcancelled_declaration
	| path_declaration
	| system_timing_declaration
	;

specparam_declaration:
	TOK_SPECPARAM list_of_specparam_assignments ';' |
	TOK_SPECPARAM specparam_range list_of_specparam_assignments ';' ;

// IEEE 1364-2005 calls this sinmply 'range' but the current 'range' rule allows empty match
// and the 'non_opt_range' rule allows index ranges not allowed by 1364-2005
// exxxxtending this for SV specparam would change this anyhow
specparam_range:
	'[' ignspec_constant_expression ':' ignspec_constant_expression ']' ;

list_of_specparam_assignments:
	specparam_assignment | list_of_specparam_assignments ',' specparam_assignment;

specparam_assignment:
	ignspec_id '=' ignspec_expr ;

ignspec_opt_cond:
	TOK_IF '(' ignspec_expr ')' | %empty;

path_declaration :
	simple_path_declaration ';'
	// | edge_sensitive_path_declaration
	// | state_dependent_path_declaration
	;

simple_path_declaration :
	ignspec_opt_cond parallel_path_description '=' path_delay_value |
	ignspec_opt_cond full_path_description '=' path_delay_value
	;

path_delay_value :
	'(' ignspec_expr list_of_path_delay_extra_expressions ')'
	|     ignspec_expr
	|     ignspec_expr list_of_path_delay_extra_expressions
	;

list_of_path_delay_extra_expressions :
	',' ignspec_expr
	| ',' ignspec_expr list_of_path_delay_extra_expressions
	;

specify_edge_identifier :
	TOK_POSEDGE | TOK_NEGEDGE ;

parallel_path_description :
	'(' specify_input_terminal_descriptor opt_polarity_operator '=' '>' specify_output_terminal_descriptor ')' |
	'(' specify_edge_identifier specify_input_terminal_descriptor '=' '>' '(' specify_output_terminal_descriptor opt_polarity_operator ':' ignspec_expr ')' ')' |
	'(' specify_edge_identifier specify_input_terminal_descriptor '=' '>' '(' specify_output_terminal_descriptor TOK_POS_INDEXED ignspec_expr ')' ')' ;

full_path_description :
	'(' list_of_path_inputs '*' '>' list_of_path_outputs ')' |
	'(' specify_edge_identifier list_of_path_inputs '*' '>' '(' list_of_path_outputs opt_polarity_operator ':' ignspec_expr ')' ')' |
	'(' specify_edge_identifier list_of_path_inputs '*' '>' '(' list_of_path_outputs TOK_POS_INDEXED ignspec_expr ')' ')' ;

// This was broken into 2 rules to solve shift/reduce conflicts
list_of_path_inputs :
	specify_input_terminal_descriptor                  opt_polarity_operator  |
	specify_input_terminal_descriptor more_path_inputs opt_polarity_operator ;

more_path_inputs :
    ',' specify_input_terminal_descriptor |
    more_path_inputs ',' specify_input_terminal_descriptor ;

list_of_path_outputs :
	specify_output_terminal_descriptor |
	list_of_path_outputs ',' specify_output_terminal_descriptor ;

opt_polarity_operator :
	'+' | '-' | %empty;

// Good enough for the time being
specify_input_terminal_descriptor :
	ignspec_id ;

// Good enough for the time being
specify_output_terminal_descriptor :
	ignspec_id ;

system_timing_declaration :
	ignspec_id '(' system_timing_args ')' ';' ;

system_timing_arg :
	TOK_POSEDGE ignspec_id |
	TOK_NEGEDGE ignspec_id |
	ignspec_expr ;

system_timing_args :
	system_timing_arg |
	system_timing_args TOK_IGNORED_SPECIFY_AND system_timing_arg |
	system_timing_args ',' system_timing_arg ;

// for the time being this is OK, but we may write our own expr here.
// as I'm not sure it is legal to use a full expr here (probably not)
// On the other hand, other rules requiring constant expressions also use 'expr'
// (such as param assignment), so we may leave this as-is, perhaps adding runtime checks for constant-ness
ignspec_constant_expression:
	expr { };

ignspec_expr:
	expr { } |
	expr ':' expr ':' expr {
	};

ignspec_id:
	TOK_ID { }
	range_or_multirange { };

/**********************************************************************/

param_signed:
	TOK_SIGNED {
		astbuf1->is_signed = true;
	} | TOK_UNSIGNED {
		astbuf1->is_signed = false;
	} | %empty;

param_integer:
	TOK_INTEGER {
		astbuf1->children.push_back(new AstNode(AST_RANGE));
		astbuf1->children.back()->children.push_back(AstNode::mkconst_int(31, true));
		astbuf1->children.back()->children.push_back(AstNode::mkconst_int(0, true));
		astbuf1->is_signed = true;
	};

param_real:
	TOK_REAL {
		astbuf1->children.push_back(new AstNode(AST_REALVALUE));
	};

param_range:
	range {
		if ($1 != NULL) {
			astbuf1->children.push_back($1);
		}
	};

param_integer_type: param_integer param_signed;
param_range_type: type_vec param_signed param_range;
param_implicit_type: param_signed param_range;

param_type:
	param_integer_type | param_real | param_range_type | param_implicit_type |
	hierarchical_type_id {
		astbuf1->is_custom_type = true;
		astbuf1->children.push_back(new AstNode(AST_WIRETYPE));
		astbuf1->children.back()->str = *$1;
	};

param_decl:
	attr TOK_PARAMETER {
		astbuf1 = new AstNode(AST_PARAMETER);
		astbuf1->children.push_back(AstNode::mkconst_int(0, true));
	} param_type param_decl_list ';' {
		delete astbuf1;
	};

localparam_decl:
	attr TOK_LOCALPARAM {
		astbuf1 = new AstNode(AST_LOCALPARAM);
		astbuf1->children.push_back(AstNode::mkconst_int(0, true));
	} param_type param_decl_list ';' {
		delete astbuf1;
	};

param_decl_list:
	single_param_decl | param_decl_list ',' single_param_decl;

single_param_decl:
	TOK_ID '=' expr {
		AstNode *node;
		if (astbuf1 == nullptr) {
			if (!sv_mode)
				frontend_verilog_yyerror("In pure Verilog (not SystemVerilog), parameter/localparam with an initializer must use the parameter/localparam keyword");
			node = new AstNode(AST_PARAMETER);
			node->children.push_back(AstNode::mkconst_int(0, true));
		} else {
			node = astbuf1->clone();
		}
		node->str = *$1;
		delete node->children[0];
		node->children[0] = $3;
		//ast_stack.back()->children.push_back(node);
		delete $1;
	};

defparam_decl:
	TOK_DEFPARAM defparam_decl_list ';';

defparam_decl_list:
	single_defparam_decl | defparam_decl_list ',' single_defparam_decl;

single_defparam_decl:
	range rvalue '=' expr {
		AstNode *node = new AstNode(AST_DEFPARAM);
		node->children.push_back($2);
		node->children.push_back($4);
		if ($1 != NULL)
			node->children.push_back($1);
		//ast_stack.back()->children.push_back(node);
	};

/////////
// enum
/////////

enum_type: TOK_ENUM {
	 } enum_base_type '{' enum_name_list '}' {	// create template for the enum vars
         }
	 ;

enum_base_type: type_atom type_signing
	| type_vec type_signing range	{ }
	| %empty			{ }
	;

type_atom: TOK_INTEGER		{ }		// 4-state signed
	|  TOK_INT		{ }		// 2-state signed
	|  TOK_SHORTINT		{ }	// 2-state signed
	|  TOK_BYTE		{ }	// 2-state signed
	;

type_vec: TOK_REG		{ }		// unsigned
	| TOK_LOGIC		{ }		// unsigned
	;

type_signing:
	  TOK_SIGNED		{ }
	| TOK_UNSIGNED		{ }
	| %empty
	;

enum_name_list: enum_name_decl
	| enum_name_list ',' enum_name_decl
	;

enum_name_decl:
	TOK_ID opt_enum_init {
	}
	;

opt_enum_init:
	'=' basic_expr		{ }	// TODO: restrict this
	| %empty		{ }
	;

enum_var_list:
	enum_var
	| enum_var_list ',' enum_var
	;

enum_var: TOK_ID {
	}
	;

enum_decl: enum_type enum_var_list ';'		{ }
	;

//////////////////
// struct or union
//////////////////

struct_decl: struct_type struct_var_list ';' 	{ }
	;

struct_type: struct_union { astbuf2 = $1; } struct_body { }
	;

struct_union:
	  TOK_STRUCT		{ }
	| TOK_UNION		{ }
	;

struct_body: opt_packed '{' struct_member_list '}'
	;

opt_packed:
	TOK_PACKED opt_signed_struct |
	%empty { frontend_verilog_yyerror("Only PACKED supported at this time"); };

opt_signed_struct:
	  TOK_SIGNED		{ }
	| TOK_UNSIGNED		{ }
	| %empty // default is unsigned
	;

struct_member_list: struct_member
	| struct_member_list struct_member
	;

struct_member: struct_member_type member_name_list ';'		{ }
	;

member_name_list:
	  member_name
	| member_name_list ',' member_name
	;

member_name: TOK_ID {
		} range { }
	;

struct_member_type: { } member_type_token
	;

member_type_token:
	  member_type 
	| hierarchical_type_id {
		}
	| struct_union {
		} struct_body  {
		}
	;

member_type: type_atom type_signing
	| type_vec type_signing range_or_multirange	{ }
	;

struct_var_list: struct_var
	| struct_var_list ',' struct_var
	;

struct_var: TOK_ID	{
			}
	;

/////////
// wire
/////////

wire_decl:
	attr wire_type range {
		//albuf = $1;
		astbuf1 = $2;
		astbuf2 = checkRange(astbuf1, $3);
	} delay wire_name_list {
		delete astbuf1;
		if (astbuf2 != NULL)
			delete astbuf2;
	} ';' |
	attr TOK_SUPPLY0 TOK_ID {
                AstNode *node = new AstNode(AST_WIRE);
                node->str = *$3;
                CALLBACK(verilog_parser::kVerilogCallback.getWireCbk(), node);
                delete node;
                node = new AstNode(AST_ASSIGN, new AstNode(AST_IDENTIFIER), AstNode::mkconst_int(0, false, 1));
                node->children[0]->str = *$3;
                CALLBACK(verilog_parser::kVerilogCallback.getAssignCbk(), node);
                delete node;
		delete $3;
	} opt_supply_wires ';' |
	attr TOK_SUPPLY1 TOK_ID {
                AstNode *node = new AstNode(AST_WIRE);
                node->str = *$3;
                CALLBACK(verilog_parser::kVerilogCallback.getWireCbk(), node);
                delete node;
                node = new AstNode(AST_ASSIGN, new AstNode(AST_IDENTIFIER), AstNode::mkconst_int(1, false, 1));
                node->children[0]->str = *$3;
                CALLBACK(verilog_parser::kVerilogCallback.getAssignCbk(), node);
                delete node;
		delete $3;
	} opt_supply_wires ';';

opt_supply_wires:
	%empty |
	opt_supply_wires ',' TOK_ID {
		AstNode *wire_node = ast_stack.back()->children.at(GetSize(ast_stack.back()->children)-2)->clone();
		AstNode *assign_node = ast_stack.back()->children.at(GetSize(ast_stack.back()->children)-1)->clone();
		wire_node->str = *$3;
		assign_node->children[0]->str = *$3;
		ast_stack.back()->children.push_back(wire_node);
		ast_stack.back()->children.push_back(assign_node);
		delete $3;
	};

wire_name_list:
	wire_name_and_opt_assign | wire_name_list ',' wire_name_and_opt_assign;

wire_name_and_opt_assign:
	wire_name { } |
	wire_name '=' expr {
		AstNode *wire = new AstNode(AST_IDENTIFIER);
		wire->str = ast_stack.back()->children.back()->str;
		if (astbuf1->is_input) {
		}
		else if (astbuf1->is_reg || astbuf1->is_logic){
			AstNode *assign = new AstNode(AST_ASSIGN_LE, wire, $3);
                        CALLBACK(verilog_parser::kVerilogCallback.getAssignCbk(), assign);
                        delete assign;
			//AstNode *block = new AstNode(AST_BLOCK, assign);
			//AstNode *init = new AstNode(AST_INITIAL, block);

			//ast_stack.back()->children.push_back(init);
		}
		else {
			AstNode *assign = new AstNode(AST_ASSIGN, wire, $3);
                        CALLBACK(verilog_parser::kVerilogCallback.getAssignCbk(), assign);
                        delete assign;
			//ast_stack.back()->children.push_back(assign);
		}

	};

wire_name:
	TOK_ID range_or_multirange {
		if (astbuf1 == nullptr)
			frontend_verilog_yyerror("Internal error - should not happen - no AST_WIRE node.");
		AstNode *node = astbuf1->clone();
		node->str = *$1;
		if (astbuf2 != NULL)
			node->children.push_back(astbuf2->clone());
		if ($2 != NULL) {
			if (node->is_input || node->is_output)
				frontend_verilog_yyerror("input/output/inout ports cannot have unpacked dimensions.");
			if (!astbuf2 && !node->is_custom_type) {
				addRange(node, 0, 0, false);
			}
			rewriteAsMemoryNode(node, $2);
		}
                if (do_not_require_port_stubs && (node->is_input || node->is_output) && port_stubs.count(*$1) == 0) {
                        port_stubs[*$1] = ++port_counter;
                }
                if (port_stubs.count(*$1) != 0) {
                        if (!node->is_input && !node->is_output)
                                frontend_verilog_yyerror("Module port `%s' is neither input nor output.", $1->c_str());
                        if (node->is_reg && node->is_input && !node->is_output && !sv_mode)
                                frontend_verilog_yyerror("Input port `%s' is declared as register.", $1->c_str());
                        node->port_id = port_stubs[*$1];
                        port_stubs.erase(*$1);
                } else {
                        if (node->is_input || node->is_output)
                                frontend_verilog_yyerror("Module port `%s' is not declared in module header.", $1->c_str());
                }
                CALLBACK(verilog_parser::kVerilogCallback.getWireCbk(), node);
                delete node;
		delete $1;
	};

assign_stmt:
	TOK_ASSIGN delay assign_expr_list ';';

assign_expr_list:
	assign_expr | assign_expr_list ',' assign_expr;

assign_expr:
	lvalue '=' expr {
		AstNode *node = new AstNode(AST_ASSIGN, $1, $3);
                CALLBACK(verilog_parser::kVerilogCallback.getAssignCbk(), node);
                delete node;
	};

type_name: TOK_ID		// first time seen
	 | TOK_USER_TYPE	{ if (isInLocalScope($1)) frontend_verilog_yyerror("Duplicate declaration of TYPEDEF '%s'", $1->c_str()+1); }
	 ;

typedef_decl:
	TOK_TYPEDEF wire_type range type_name range_or_multirange ';' {
		astbuf1 = $2;
		astbuf2 = checkRange(astbuf1, $3);
		if (astbuf2)
			astbuf1->children.push_back(astbuf2);

		if ($5 != NULL) {
			if (!astbuf2) {
				addRange(astbuf1, 0, 0, false);
			}
			rewriteAsMemoryNode(astbuf1, $5);
		}
		addTypedefNode($4, astbuf1); }
	| TOK_TYPEDEF non_wire_data_type type_name ';'   { addTypedefNode($3, $2); }
	;

non_wire_data_type:
	  enum_type
	| struct_type
	;

cell_stmt:
	attr TOK_ID {
		astbuf1 = new AstNode(AST_CELL);
		astbuf1->children.push_back(new AstNode(AST_CELLTYPE));
		astbuf1->children[0]->str = *$2;
		delete $2;
	} cell_parameter_list_opt cell_list ';' {
                CALLBACK(verilog_parser::kVerilogCallback.getInstCbk(), astbuf1);
		delete astbuf1;
	} |
	attr tok_prim_wrapper delay {
		astbuf1 = new AstNode(AST_PRIMITIVE);
		astbuf1->str = *$2;
		delete $2;
	} prim_list ';' {
		delete astbuf1;
	};

tok_prim_wrapper:
	TOK_PRIMITIVE {
		$$ = $1;
	} |
	TOK_OR {
		$$ = new std::string("or");
	};

cell_list:
	single_cell |
	cell_list ',' single_cell;

single_cell:
	TOK_ID {
		if (astbuf1->type != AST_PRIMITIVE)
			astbuf1->str = *$1;
		delete $1;
		//ast_stack.back()->children.push_back(astbuf2);
	} '(' cell_port_list ')' {
	} |
	TOK_ID non_opt_range {
		if (astbuf1->type != AST_PRIMITIVE)
			astbuf1->str = *$1;
		delete $1;
		//ast_stack.back()->children.push_back(new AstNode(AST_CELLARRAY, $2, astbuf2));
	} '(' cell_port_list ')'{
	};

prim_list:
	single_prim |
	prim_list ',' single_prim;

single_prim:
	single_cell |
	/* no name */ {
	} '(' cell_port_list ')' {
	}

cell_parameter_list_opt:
	'#' '(' cell_parameter_list ')' | %empty;

cell_parameter_list:
	cell_parameter | cell_parameter_list ',' cell_parameter;

cell_parameter:
	%empty |
	expr {
		AstNode *node = new AstNode(AST_PARASET);
		astbuf1->children.push_back(node);
		node->children.push_back($1);
	} |
	'.' TOK_ID '(' expr ')' {
		AstNode *node = new AstNode(AST_PARASET);
		node->str = *$2;
		astbuf1->children.push_back(node);
		node->children.push_back($4);
		delete $2;
	};

cell_port_list:
	cell_port_list_rules {
		// remove empty args from end of list
		while (!astbuf1->children.empty()) {
			AstNode *node = astbuf1->children.back();
			if (node->type != AST_ARGUMENT) break;
			if (!node->children.empty()) break;
			if (!node->str.empty()) break;
			astbuf1->children.pop_back();
			delete node;
		}

		// check port types
		bool has_positional_args = false;
		bool has_named_args = false;
		for (auto node : astbuf1->children) {
			if (node->type != AST_ARGUMENT) continue;
			if (node->str.empty())
				has_positional_args = true;
			else
				has_named_args = true;
		}

		if (has_positional_args && has_named_args)
			frontend_verilog_yyerror("Mix of positional and named cell ports.");
	};

cell_port_list_rules:
	cell_port | cell_port_list_rules ',' cell_port;

cell_port:
	attr {
		AstNode *node = new AstNode(AST_ARGUMENT);
		astbuf1->children.push_back(node);
	} |
	attr expr {
		AstNode *node = new AstNode(AST_ARGUMENT);
		astbuf1->children.push_back(node);
		node->children.push_back($2);
	} |
	attr '.' TOK_ID '(' expr ')' {
		AstNode *node = new AstNode(AST_ARGUMENT);
		node->str = *$3;
		astbuf1->children.push_back(node);
		node->children.push_back($5);
		delete $3;
	} |
	attr '.' TOK_ID '(' ')' {
		AstNode *node = new AstNode(AST_ARGUMENT);
		node->str = *$3;
		astbuf1->children.push_back(node);
		delete $3;
	} |
	attr '.' TOK_ID {
		AstNode *node = new AstNode(AST_ARGUMENT);
		node->str = *$3;
		astbuf1->children.push_back(node);
		node->children.push_back(new AstNode(AST_IDENTIFIER));
		node->children.back()->str = *$3;
		delete $3;
	} |
	attr TOK_WILDCARD_CONNECT {
		if (!sv_mode)
			frontend_verilog_yyerror("Wildcard port connections are only supported in SystemVerilog mode.");
	};

always_comb_or_latch:
	TOK_ALWAYS_COMB {
		$$ = false;
	} |
	TOK_ALWAYS_LATCH {
		$$ = true;
	};

always_or_always_ff:
	TOK_ALWAYS {
		$$ = false;
	} |
	TOK_ALWAYS_FF {
		$$ = true;
	};

always_stmt:
	attr always_or_always_ff {
	} always_cond {
	} behavioral_stmt {
	} |
	attr always_comb_or_latch {
	} behavioral_stmt {
	} |
	attr TOK_INITIAL {
	} behavioral_stmt {
	};

always_cond:
	'@' '(' always_events ')' |
	'@' '(' '*' ')' |
	'@' ATTR_BEGIN ')' |
	'@' '(' ATTR_END |
	'@' '*' |
	%empty;

always_events:
	always_event |
	always_events TOK_OR always_event |
	always_events ',' always_event;

always_event:
	TOK_POSEDGE expr {
	} |
	TOK_NEGEDGE expr {
	} |
	expr {
	};

opt_label:
	':' TOK_ID {
		$$ = $2;
	} |
	%empty {
		$$ = NULL;
	};

opt_sva_label:
	TOK_SVA_LABEL ':' {
		$$ = $1;
	} |
	%empty {
		$$ = NULL;
	};

opt_property:
	TOK_PROPERTY {
		$$ = true;
	} |
	TOK_FINAL {
		$$ = false;
	} |
	%empty {
		$$ = false;
	};

modport_stmt:
    TOK_MODPORT TOK_ID {
    }  modport_args_opt {
    } ';'

modport_args_opt:
    '(' ')' | '(' modport_args optional_comma ')';

modport_args:
    modport_arg | modport_args ',' modport_arg;

modport_arg:
    modport_type_token modport_member |
    modport_member

modport_member:
    TOK_ID {
    }

modport_type_token:
    TOK_INPUT {} | TOK_OUTPUT {}

assert:
	opt_sva_label TOK_ASSERT opt_property '(' expr ')' ';' {
	} |
	opt_sva_label TOK_ASSUME opt_property '(' expr ')' ';' {
	} |
	opt_sva_label TOK_ASSERT opt_property '(' TOK_EVENTUALLY expr ')' ';' {
	} |
	opt_sva_label TOK_ASSUME opt_property '(' TOK_EVENTUALLY expr ')' ';' {
	} |
	opt_sva_label TOK_COVER opt_property '(' expr ')' ';' {
	} |
	opt_sva_label TOK_COVER opt_property '(' ')' ';' {
	} |
	opt_sva_label TOK_COVER ';' {
	} |
	opt_sva_label TOK_RESTRICT opt_property '(' expr ')' ';' {
	} |
	opt_sva_label TOK_RESTRICT opt_property '(' TOK_EVENTUALLY expr ')' ';' {
	};

assert_property:
	opt_sva_label TOK_ASSERT TOK_PROPERTY '(' expr ')' ';' {
	} |
	opt_sva_label TOK_ASSUME TOK_PROPERTY '(' expr ')' ';' {
	} |
	opt_sva_label TOK_ASSERT TOK_PROPERTY '(' TOK_EVENTUALLY expr ')' ';' {
	} |
	opt_sva_label TOK_ASSUME TOK_PROPERTY '(' TOK_EVENTUALLY expr ')' ';' {
	} |
	opt_sva_label TOK_COVER TOK_PROPERTY '(' expr ')' ';' {
	} |
	opt_sva_label TOK_RESTRICT TOK_PROPERTY '(' expr ')' ';' {
	} |
	opt_sva_label TOK_RESTRICT TOK_PROPERTY '(' TOK_EVENTUALLY expr ')' ';' {
	};

simple_behavioral_stmt:
	attr lvalue '=' delay expr {
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, $5);
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_INCREMENT {
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, new AstNode(AST_ADD, $2->clone(), AstNode::mkconst_int(1, true)));
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_DECREMENT {
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, new AstNode(AST_SUB, $2->clone(), AstNode::mkconst_int(1, true)));
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue OP_LE delay expr {
		AstNode *node = new AstNode(AST_ASSIGN_LE, $2, $5);
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_XOR_ASSIGN delay expr {
		AstNode *xor_node = new AstNode(AST_BIT_XOR, $2->clone(), $5);
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, xor_node);
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_OR_ASSIGN delay expr {
		AstNode *or_node = new AstNode(AST_BIT_OR, $2->clone(), $5);
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, or_node);
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_PLUS_ASSIGN delay expr {
		AstNode *add_node = new AstNode(AST_ADD, $2->clone(), $5);
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, add_node);
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_SUB_ASSIGN delay expr {
		AstNode *sub_node = new AstNode(AST_SUB, $2->clone(), $5);
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, sub_node);
		ast_stack.back()->children.push_back(node);
	} |
	attr lvalue TOK_AND_ASSIGN delay expr {
		AstNode *and_node = new AstNode(AST_BIT_AND, $2->clone(), $5);
		AstNode *node = new AstNode(AST_ASSIGN_EQ, $2, and_node);
		ast_stack.back()->children.push_back(node);
	};

// this production creates the obligatory if-else shift/reduce conflict
behavioral_stmt:
	defattr | assert | wire_decl | param_decl | localparam_decl | typedef_decl |
	non_opt_delay behavioral_stmt |
	simple_behavioral_stmt ';' |
	attr ';' {
	} |
	attr hierarchical_id {
	} opt_arg_list ';'{
	} |
	attr TOK_MSG_TASKS {
	} opt_arg_list ';'{
	} |
	attr TOK_BEGIN {
	} opt_label {
	} behavioral_stmt_list TOK_END opt_label {
	} |
	attr TOK_FOR '(' {
	} simple_behavioral_stmt ';' expr {
	} ';' simple_behavioral_stmt ')' {
	} behavioral_stmt {
	} |
	attr TOK_WHILE '(' expr ')' {
	} behavioral_stmt {
	} |
	attr TOK_REPEAT '(' expr ')' {
	} behavioral_stmt {
	} |
	attr TOK_IF '(' expr ')' {
	} behavioral_stmt {
	} optional_else {
	} |
	case_attr case_type '(' expr ')' {
	} opt_synopsys_attr case_body TOK_ENDCASE {
	};

unique_case_attr:
	%empty {
	} |
	TOK_PRIORITY case_attr {
	} |
	TOK_UNIQUE case_attr {
	};

case_attr:
	attr unique_case_attr {
	};

case_type:
	TOK_CASE {
	} |
	TOK_CASEX {
	} |
	TOK_CASEZ {
	};

opt_synopsys_attr:
	opt_synopsys_attr TOK_SYNOPSYS_FULL_CASE {
	} |
	opt_synopsys_attr TOK_SYNOPSYS_PARALLEL_CASE {
	} |
	%empty;

behavioral_stmt_list:
	behavioral_stmt_list behavioral_stmt |
	%empty;

optional_else:
	TOK_ELSE {
	} behavioral_stmt {
	} |
	%empty %prec FAKE_THEN;

case_body:
	case_body case_item |
	%empty;

case_item:
	{
	} case_select {
	} behavioral_stmt {
	};

gen_case_body:
	gen_case_body gen_case_item |
	%empty;

gen_case_item:
	{
	} case_select {
	} gen_stmt_block {
	};

case_select:
	case_expr_list ':' |
	TOK_DEFAULT;

case_expr_list:
	TOK_DEFAULT {
	} |
	TOK_SVA_LABEL {
	} |
	expr {
	} |
	case_expr_list ',' expr {
	};

rvalue:
	hierarchical_id '[' expr ']' '.' rvalue {
		$$ = new AstNode(AST_PREFIX, $3, $6);
		$$->str = *$1;
		delete $1;
	} |
	hierarchical_id range {
		$$ = new AstNode(AST_IDENTIFIER, $2);
		$$->str = *$1;
		delete $1;
		if ($2 == nullptr && ($$->str == "\\$initstate" ||
				$$->str == "\\$anyconst" || $$->str == "\\$anyseq" ||
				$$->str == "\\$allconst" || $$->str == "\\$allseq"))
			$$->type = AST_FCALL;
	} |
	hierarchical_id non_opt_multirange {
		$$ = new AstNode(AST_IDENTIFIER, $2);
		$$->str = *$1;
		delete $1;
	};

lvalue:
	rvalue {
		$$ = $1;
	} |
	'{' lvalue_concat_list '}' {
		$$ = $2;
	};

lvalue_concat_list:
	expr {
		$$ = new AstNode(AST_CONCAT);
		$$->children.push_back($1);
	} |
	expr ',' lvalue_concat_list {
		$$ = $3;
		$$->children.push_back($1);
	};

opt_arg_list:
	'(' arg_list optional_comma ')' |
	%empty;

arg_list:
	arg_list2 |
	%empty;

arg_list2:
	single_arg |
	arg_list ',' single_arg;

single_arg:
	expr {
		ast_stack.back()->children.push_back($1);
	};

module_gen_body:
	module_gen_body gen_stmt_or_module_body_stmt |
	%empty;

gen_stmt_or_module_body_stmt:
	gen_stmt | module_body_stmt |
	attr ';' {
	};

// this production creates the obligatory if-else shift/reduce conflict
gen_stmt:
	TOK_FOR '(' {
	} simple_behavioral_stmt ';' expr {
	} ';' simple_behavioral_stmt ')' gen_stmt_block {
	} |
	TOK_IF '(' expr ')' {
	} gen_stmt_block {
	} opt_gen_else {
	} |
	case_type '(' expr ')' {
	} gen_case_body TOK_ENDCASE {
	} |
	TOK_BEGIN {
	} opt_label {
	} module_gen_body TOK_END opt_label {
	} |
	TOK_MSG_TASKS {
	} opt_arg_list ';'{
	};

gen_stmt_block:
	{
	} gen_stmt_or_module_body_stmt {
	};

opt_gen_else:
	TOK_ELSE gen_stmt_block | %empty %prec FAKE_THEN;

expr:
	basic_expr {
		$$ = $1;
	} |
	basic_expr '?' attr expr ':' expr {
		$$ = new AstNode(AST_TERNARY);
		$$->children.push_back($1);
		$$->children.push_back($4);
		$$->children.push_back($6);
	};

basic_expr:
	rvalue {
		$$ = $1;
	} |
	'(' expr ')' integral_number {
		if ($4->compare(0, 1, "'") != 0)
			frontend_verilog_yyerror("Cast operation must be applied on sized constants e.g. (<expr>)<constval> , while %s is not a sized constant.", $4->c_str());
		AstNode *bits = $2;
		AstNode *val = const2ast(*$4, case_type_stack.size() == 0 ? 0 : case_type_stack.back(), !lib_mode);
		if (val == NULL)
			log_error("Value conversion failed: `%s'\n", $4->c_str());
		$$ = new AstNode(AST_TO_BITS, bits, val);
		delete $4;
	} |
	hierarchical_id integral_number {
		if ($2->compare(0, 1, "'") != 0)
			frontend_verilog_yyerror("Cast operation must be applied on sized constants, e.g. <ID>\'d0, while %s is not a sized constant.", $2->c_str());
		AstNode *bits = new AstNode(AST_IDENTIFIER);
		bits->str = *$1;
		AstNode *val = const2ast(*$2, case_type_stack.size() == 0 ? 0 : case_type_stack.back(), !lib_mode);
		if (val == NULL)
			log_error("Value conversion failed: `%s'\n", $2->c_str());
		$$ = new AstNode(AST_TO_BITS, bits, val);
		delete $1;
		delete $2;
	} |
	integral_number {
		$$ = const2ast(*$1, case_type_stack.size() == 0 ? 0 : case_type_stack.back(), !lib_mode);
		if ($$ == NULL)
			log_error("Value conversion failed: `%s'\n", $1->c_str());
		delete $1;
	} |
	TOK_REALVAL {
		$$ = new AstNode(AST_REALVALUE);
		char *p = (char*)malloc(GetSize(*$1) + 1), *q;
		for (int i = 0, j = 0; j < GetSize(*$1); j++)
			if ((*$1)[j] != '_')
				p[i++] = (*$1)[j], p[i] = 0;
		$$->realvalue = strtod(p, &q);
		log_assert(*q == 0);
		delete $1;
		free(p);
	} |
	TOK_STRING {
		$$ = AstNode::mkconst_str(*$1);
		delete $1;
	} |
	hierarchical_id attr {
		AstNode *node = new AstNode(AST_FCALL);
		node->str = *$1;
		delete $1;
		ast_stack.push_back(node);
	} '(' arg_list optional_comma ')' {
		$$ = ast_stack.back();
		ast_stack.pop_back();
	} |
	TOK_TO_SIGNED attr '(' expr ')' {
		$$ = new AstNode(AST_TO_SIGNED, $4);
	} |
	TOK_TO_UNSIGNED attr '(' expr ')' {
		$$ = new AstNode(AST_TO_UNSIGNED, $4);
	} |
	'(' expr ')' {
		$$ = $2;
	} |
	'(' expr ':' expr ':' expr ')' {
		delete $2;
		$$ = $4;
		delete $6;
	} |
	'{' concat_list '}' {
		$$ = $2;
	} |
	'{' expr '{' concat_list '}' '}' {
		$$ = new AstNode(AST_REPLICATE, $2, $4);
	} |
	'~' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_BIT_NOT, $3);
	} |
	basic_expr '&' attr basic_expr {
		$$ = new AstNode(AST_BIT_AND, $1, $4);
	} |
	basic_expr OP_NAND attr basic_expr {
		$$ = new AstNode(AST_BIT_NOT, new AstNode(AST_BIT_AND, $1, $4));
	} |
	basic_expr '|' attr basic_expr {
		$$ = new AstNode(AST_BIT_OR, $1, $4);
	} |
	basic_expr OP_NOR attr basic_expr {
		$$ = new AstNode(AST_BIT_NOT, new AstNode(AST_BIT_OR, $1, $4));
	} |
	basic_expr '^' attr basic_expr {
		$$ = new AstNode(AST_BIT_XOR, $1, $4);
	} |
	basic_expr OP_XNOR attr basic_expr {
		$$ = new AstNode(AST_BIT_XNOR, $1, $4);
	} |
	'&' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_REDUCE_AND, $3);
	} |
	OP_NAND attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_REDUCE_AND, $3);
		$$ = new AstNode(AST_LOGIC_NOT, $$);
	} |
	'|' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_REDUCE_OR, $3);
	} |
	OP_NOR attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_REDUCE_OR, $3);
		$$ = new AstNode(AST_LOGIC_NOT, $$);
	} |
	'^' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_REDUCE_XOR, $3);
	} |
	OP_XNOR attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_REDUCE_XNOR, $3);
	} |
	basic_expr OP_SHL attr basic_expr {
		$$ = new AstNode(AST_SHIFT_LEFT, $1, new AstNode(AST_TO_UNSIGNED, $4));
	} |
	basic_expr OP_SHR attr basic_expr {
		$$ = new AstNode(AST_SHIFT_RIGHT, $1, new AstNode(AST_TO_UNSIGNED, $4));
	} |
	basic_expr OP_SSHL attr basic_expr {
		$$ = new AstNode(AST_SHIFT_SLEFT, $1, new AstNode(AST_TO_UNSIGNED, $4));
	} |
	basic_expr OP_SSHR attr basic_expr {
		$$ = new AstNode(AST_SHIFT_SRIGHT, $1, new AstNode(AST_TO_UNSIGNED, $4));
	} |
	basic_expr '<' attr basic_expr {
		$$ = new AstNode(AST_LT, $1, $4);
	} |
	basic_expr OP_LE attr basic_expr {
		$$ = new AstNode(AST_LE, $1, $4);
	} |
	basic_expr OP_EQ attr basic_expr {
		$$ = new AstNode(AST_EQ, $1, $4);
	} |
	basic_expr OP_NE attr basic_expr {
		$$ = new AstNode(AST_NE, $1, $4);
	} |
	basic_expr OP_EQX attr basic_expr {
		$$ = new AstNode(AST_EQX, $1, $4);
	} |
	basic_expr OP_NEX attr basic_expr {
		$$ = new AstNode(AST_NEX, $1, $4);
	} |
	basic_expr OP_GE attr basic_expr {
		$$ = new AstNode(AST_GE, $1, $4);
	} |
	basic_expr '>' attr basic_expr {
		$$ = new AstNode(AST_GT, $1, $4);
	} |
	basic_expr '+' attr basic_expr {
		$$ = new AstNode(AST_ADD, $1, $4);
	} |
	basic_expr '-' attr basic_expr {
		$$ = new AstNode(AST_SUB, $1, $4);
	} |
	basic_expr '*' attr basic_expr {
		$$ = new AstNode(AST_MUL, $1, $4);
	} |
	basic_expr '/' attr basic_expr {
		$$ = new AstNode(AST_DIV, $1, $4);
	} |
	basic_expr '%' attr basic_expr {
		$$ = new AstNode(AST_MOD, $1, $4);
	} |
	basic_expr OP_POW attr basic_expr {
		$$ = new AstNode(AST_POW, $1, $4);
	} |
	'+' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_POS, $3);
	} |
	'-' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_NEG, $3);
	} |
	basic_expr OP_LAND attr basic_expr {
		$$ = new AstNode(AST_LOGIC_AND, $1, $4);
	} |
	basic_expr OP_LOR attr basic_expr {
		$$ = new AstNode(AST_LOGIC_OR, $1, $4);
	} |
	'!' attr basic_expr %prec UNARY_OPS {
		$$ = new AstNode(AST_LOGIC_NOT, $3);
	} |
	TOK_SIGNED OP_CAST '(' expr ')' {
		if (!sv_mode)
			frontend_verilog_yyerror("Static cast is only supported in SystemVerilog mode.");
		$$ = new AstNode(AST_TO_SIGNED, $4);
	} |
	TOK_UNSIGNED OP_CAST '(' expr ')' {
		if (!sv_mode)
			frontend_verilog_yyerror("Static cast is only supported in SystemVerilog mode.");
		$$ = new AstNode(AST_TO_UNSIGNED, $4);
	} |
	basic_expr OP_CAST '(' expr ')' {
		if (!sv_mode)
			frontend_verilog_yyerror("Static cast is only supported in SystemVerilog mode.");
		$$ = new AstNode(AST_CAST_SIZE, $1, $4);
	};

concat_list:
	expr {
		$$ = new AstNode(AST_CONCAT, $1);
	} |
	expr ',' concat_list {
		$$ = $3;
		$$->children.push_back($1);
	};

integral_number:
	TOK_CONSTVAL { $$ = $1; } |
	TOK_UNBASED_UNSIZED_CONSTVAL { $$ = $1; } |
	TOK_BASE TOK_BASED_CONSTVAL {
		$1->append(*$2);
		$$ = $1;
		delete $2;
	} |
	TOK_CONSTVAL TOK_BASE TOK_BASED_CONSTVAL {
		$1->append(*$2).append(*$3);
		$$ = $1;
		delete $2;
		delete $3;
	};
