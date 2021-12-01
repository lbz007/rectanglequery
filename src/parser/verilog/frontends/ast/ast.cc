/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2018  Ruben Undheim <ruben.undheim@gmail.com>
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
 *  This is the AST frontend library.
 *
 *  The AST frontend library is not a frontend on it's own but provides a
 *  generic abstract syntax tree (AST) abstraction for HDL code and can be
 *  used by HDL frontends. See "ast.h" for an overview of the API and the
 *  Verilog frontend for an usage example.
 *
 */

#include "kernel/yosys.h"
#include "libs/sha1/sha1.h"
#include "ast.h"

YOSYS_NAMESPACE_BEGIN

using namespace AST;
using namespace AST_INTERNAL;

// instantiate global variables (public API)
namespace AST {
	std::string current_filename;
	void (*set_line_num)(int) = NULL;
	int (*get_line_num)() = NULL;
}

// instantiate global variables (private API)
namespace AST_INTERNAL {
	bool flag_dump_ast1, flag_dump_ast2, flag_no_dump_ptr, flag_dump_vlog1, flag_dump_vlog2, flag_dump_rtlil, flag_nolatches, flag_nomeminit;
	bool flag_nomem2reg, flag_mem2reg, flag_noblackbox, flag_lib, flag_nowb, flag_noopt, flag_icells, flag_pwires, flag_autowire;
	AstNode *current_ast, *current_ast_mod;
	std::map<std::string, AstNode*> current_scope;
	const dict<RTLIL::SigBit, RTLIL::SigBit> *genRTLIL_subst_ptr = NULL;
	RTLIL::SigSpec ignoreThisSignalsInInitial;
	AstNode *current_always, *current_top_block, *current_block, *current_block_child;
	AstModule *current_module;
	bool current_always_clocked;
}

// convert node types to string
std::string AST::type2str(AstNodeType type)
{
	switch (type)
	{
#define X(_item) case _item: return #_item;
	X(AST_NONE)
	X(AST_DESIGN)
	X(AST_MODULE)
	X(AST_TASK)
	X(AST_FUNCTION)
	X(AST_DPI_FUNCTION)
	X(AST_WIRE)
	X(AST_MEMORY)
	X(AST_AUTOWIRE)
	X(AST_PARAMETER)
	X(AST_LOCALPARAM)
	X(AST_DEFPARAM)
	X(AST_PARASET)
	X(AST_ARGUMENT)
	X(AST_RANGE)
	X(AST_MULTIRANGE)
	X(AST_CONSTANT)
	X(AST_REALVALUE)
	X(AST_CELLTYPE)
	X(AST_IDENTIFIER)
	X(AST_PREFIX)
	X(AST_ASSERT)
	X(AST_ASSUME)
	X(AST_LIVE)
	X(AST_FAIR)
	X(AST_COVER)
	X(AST_ENUM)
	X(AST_ENUM_ITEM)
	X(AST_FCALL)
	X(AST_TO_BITS)
	X(AST_TO_SIGNED)
	X(AST_TO_UNSIGNED)
	X(AST_SELFSZ)
	X(AST_CAST_SIZE)
	X(AST_CONCAT)
	X(AST_REPLICATE)
	X(AST_BIT_NOT)
	X(AST_BIT_AND)
	X(AST_BIT_OR)
	X(AST_BIT_XOR)
	X(AST_BIT_XNOR)
	X(AST_REDUCE_AND)
	X(AST_REDUCE_OR)
	X(AST_REDUCE_XOR)
	X(AST_REDUCE_XNOR)
	X(AST_REDUCE_BOOL)
	X(AST_SHIFT_LEFT)
	X(AST_SHIFT_RIGHT)
	X(AST_SHIFT_SLEFT)
	X(AST_SHIFT_SRIGHT)
	X(AST_SHIFTX)
	X(AST_SHIFT)
	X(AST_LT)
	X(AST_LE)
	X(AST_EQ)
	X(AST_NE)
	X(AST_EQX)
	X(AST_NEX)
	X(AST_GE)
	X(AST_GT)
	X(AST_ADD)
	X(AST_SUB)
	X(AST_MUL)
	X(AST_DIV)
	X(AST_MOD)
	X(AST_POW)
	X(AST_POS)
	X(AST_NEG)
	X(AST_LOGIC_AND)
	X(AST_LOGIC_OR)
	X(AST_LOGIC_NOT)
	X(AST_TERNARY)
	X(AST_MEMRD)
	X(AST_MEMWR)
	X(AST_MEMINIT)
	X(AST_TCALL)
	X(AST_ASSIGN)
	X(AST_CELL)
	X(AST_PRIMITIVE)
	X(AST_CELLARRAY)
	X(AST_ALWAYS)
	X(AST_INITIAL)
	X(AST_BLOCK)
	X(AST_ASSIGN_EQ)
	X(AST_ASSIGN_LE)
	X(AST_CASE)
	X(AST_COND)
	X(AST_CONDX)
	X(AST_CONDZ)
	X(AST_DEFAULT)
	X(AST_FOR)
	X(AST_WHILE)
	X(AST_REPEAT)
	X(AST_GENVAR)
	X(AST_GENFOR)
	X(AST_GENIF)
	X(AST_GENCASE)
	X(AST_GENBLOCK)
	X(AST_TECALL)
	X(AST_POSEDGE)
	X(AST_NEGEDGE)
	X(AST_EDGE)
	X(AST_INTERFACE)
	X(AST_INTERFACEPORT)
	X(AST_INTERFACEPORTTYPE)
	X(AST_MODPORT)
	X(AST_MODPORTMEMBER)
	X(AST_PACKAGE)
	X(AST_WIRETYPE)
	X(AST_TYPEDEF)
	X(AST_STRUCT)
	X(AST_UNION)
	X(AST_STRUCT_ITEM)
#undef X
	default:
		log_abort();
	}
        return "";
}

// create new node (AstNode constructor)
// (the optional child arguments make it easier to create AST trees)
AstNode::AstNode(AstNodeType type, AstNode *child1, AstNode *child2, AstNode *child3)
{
	static unsigned int hashidx_count = 123456789;
	hashidx_count = mkhash_xorshift(hashidx_count);
	hashidx_ = hashidx_count;

	this->type = type;
	is_input = false;
	is_output = false;
	is_reg = false;
	is_logic = false;
	is_signed = false;
	is_string = false;
	is_enum = false;
	is_wand = false;
	is_wor = false;
	is_tri = false;
	is_unsized = false;
	was_checked = false;
	range_valid = false;
	range_swapped = false;
	is_custom_type = false;
	port_id = 0;
	range_left = -1;
	range_right = 0;
	integer = 0;
	realvalue = 0;

	if (child1)
		children.push_back(child1);
	if (child2)
		children.push_back(child2);
	if (child3)
		children.push_back(child3);
}

// create a (deep recursive) copy of a node
AstNode *AstNode::clone() const
{
	AstNode *that = new AstNode;
	*that = *this;
	for (auto &it : that->children)
		it = it->clone();
	return that;
}

// create a (deep recursive) copy of a node use 'other' as target root node
void AstNode::cloneInto(AstNode *other) const
{
	AstNode *tmp = clone();
	other->delete_children();
	*other = *tmp;
	tmp->children.clear();
	delete tmp;
}

// delete all children in this node
void AstNode::delete_children()
{
	for (auto &it : children)
		delete it;
	children.clear();
}

// AstNode destructor
AstNode::~AstNode()
{
	delete_children();
}

// create a nice text representation of the node
// (traverse tree by recursion, use 'other' pointer for diffing two AST trees)
void AstNode::dumpAst(FILE *f, std::string indent) const
{
	if (f == NULL) {
		for (auto f : log_files)
			dumpAst(f, indent);
		return;
	}

	std::string type_name = type2str(type);
	fprintf(f, "%s%s", indent.c_str(), type_name.c_str());

	if (!flag_no_dump_ptr) {
                fprintf(f, " [%p]", this);
	}

	if (!str.empty())
		fprintf(f, " str='%s'", str.c_str());
	if (!bits.empty()) {
		fprintf(f, " bits='");
		for (size_t i = bits.size(); i > 0; i--)
			fprintf(f, "%c", bits[i-1] == State::S0 ? '0' :
					bits[i-1] == State::S1 ? '1' :
					bits[i-1] == RTLIL::Sx ? 'x' :
					bits[i-1] == RTLIL::Sz ? 'z' : '?');
		fprintf(f, "'(%d)", GetSize(bits));
	}
	if (is_input)
		fprintf(f, " input");
	if (is_output)
		fprintf(f, " output");
	if (is_logic)
		fprintf(f, " logic");
	if (is_reg) // this is an AST dump, not Verilog - if we see "logic reg" that's fine.
		fprintf(f, " reg");
	if (is_signed)
		fprintf(f, " signed");
	if (port_id > 0)
		fprintf(f, " port=%d", port_id);
	if (range_valid || range_left != -1 || range_right != 0)
		fprintf(f, " %srange=[%d:%d]%s", range_swapped ? "swapped_" : "", range_left, range_right, range_valid ? "" : "!");
	if (integer != 0)
		fprintf(f, " int=%u", (int)integer);
	if (realvalue != 0)
		fprintf(f, " real=%e", realvalue);
	if (is_enum) {
		fprintf(f, " type=enum");
	}
	fprintf(f, "\n");

	for (size_t i = 0; i < children.size(); i++)
		children[i]->dumpAst(f, indent + "  ");

	fflush(f);
}

// helper function for AstNode::dumpVlog()
static std::string id2vl(std::string txt)
{
	if (txt.size() > 1 && txt[0] == '\\')
		txt = txt.substr(1);
	for (size_t i = 0; i < txt.size(); i++) {
		if ('A' <= txt[i] && txt[i] <= 'Z') continue;
		if ('a' <= txt[i] && txt[i] <= 'z') continue;
		if ('0' <= txt[i] && txt[i] <= '9') continue;
		if (txt[i] == '_') continue;
		txt = "\\" + txt + " ";
		break;
	}
	return txt;
}

// dump AST node as Verilog pseudo-code
void AstNode::dumpVlog(FILE *f, std::string indent) const
{
	bool first = true;
	std::string txt;
	std::vector<AstNode*> rem_children1, rem_children2;

	if (f == NULL) {
		for (auto f : log_files)
			dumpVlog(f, indent);
		return;
	}

	switch (type)
	{
	case AST_MODULE:
		fprintf(f, "%s" "module %s(", indent.c_str(), id2vl(str).c_str());
		for (auto child : children)
			if (child->type == AST_WIRE && (child->is_input || child->is_output)) {
				fprintf(f, "%s%s", first ? "" : ", ", id2vl(child->str).c_str());
				first = false;
			}
		fprintf(f, ");\n");

		for (auto child : children)
			if (child->type == AST_PARAMETER || child->type == AST_LOCALPARAM || child->type == AST_DEFPARAM)
				child->dumpVlog(f, indent + "  ");
			else
				rem_children1.push_back(child);

		for (auto child : rem_children1)
			if (child->type == AST_WIRE || child->type == AST_AUTOWIRE || child->type == AST_MEMORY)
				child->dumpVlog(f, indent + "  ");
			else
				rem_children2.push_back(child);
		rem_children1.clear();

		for (auto child : rem_children2)
			if (child->type == AST_TASK || child->type == AST_FUNCTION)
				child->dumpVlog(f, indent + "  ");
			else
				rem_children1.push_back(child);
		rem_children2.clear();

		for (auto child : rem_children1)
			child->dumpVlog(f, indent + "  ");
		rem_children1.clear();

		fprintf(f, "%s" "endmodule\n", indent.c_str());
		break;

	case AST_WIRE:
		if (is_input && is_output)
			fprintf(f, "%s" "inout", indent.c_str());
		else if (is_input)
			fprintf(f, "%s" "input", indent.c_str());
		else if (is_output)
			fprintf(f, "%s" "output", indent.c_str());
		else if (!is_reg)
			fprintf(f, "%s" "wire", indent.c_str());
		if (is_reg)
			fprintf(f, "%s" "reg", (is_input || is_output) ? " " : indent.c_str());
		if (is_signed)
			fprintf(f, " signed");
		for (auto child : children) {
			fprintf(f, " ");
			child->dumpVlog(f, "");
		}
		fprintf(f, " %s", id2vl(str).c_str());
		fprintf(f, ";\n");
		break;

	case AST_MEMORY:
		fprintf(f, "%s" "memory", indent.c_str());
		if (is_signed)
			fprintf(f, " signed");
		for (auto child : children) {
			fprintf(f, " ");
			child->dumpVlog(f, "");
			if (first)
				fprintf(f, " %s", id2vl(str).c_str());
			first = false;
		}
		fprintf(f, ";\n");
		break;

	case AST_RANGE:
		if (range_valid) {
			if (range_swapped)
				fprintf(f, "[%d:%d]", range_right, range_left);
			else
				fprintf(f, "[%d:%d]", range_left, range_right);
		} else {
			for (auto child : children) {
				fprintf(f, "%c", first ? '[' : ':');
				child->dumpVlog(f, "");
				first = false;
			}
			fprintf(f, "]");
		}
		break;

	case AST_ALWAYS:
		fprintf(f, "%s" "always @", indent.c_str());
		for (auto child : children) {
			if (child->type != AST_POSEDGE && child->type != AST_NEGEDGE && child->type != AST_EDGE)
				continue;
			fprintf(f, first ? "(" : ", ");
			child->dumpVlog(f, "");
			first = false;
		}
		fprintf(f, first ? "*\n" : ")\n");
		for (auto child : children) {
			if (child->type != AST_POSEDGE && child->type != AST_NEGEDGE && child->type != AST_EDGE)
				child->dumpVlog(f, indent + "  ");
		}
		break;

	case AST_INITIAL:
		fprintf(f, "%s" "initial\n", indent.c_str());
		for (auto child : children) {
			if (child->type != AST_POSEDGE && child->type != AST_NEGEDGE && child->type != AST_EDGE)
				child->dumpVlog(f, indent + "  ");
		}
		break;

	case AST_POSEDGE:
	case AST_NEGEDGE:
	case AST_EDGE:
		if (type == AST_POSEDGE)
			fprintf(f, "posedge ");
		if (type == AST_NEGEDGE)
			fprintf(f, "negedge ");
		for (auto child : children)
			child->dumpVlog(f, "");
		break;

	case AST_IDENTIFIER:
		fprintf(f, "%s", id2vl(str).c_str());
		for (auto child : children)
			child->dumpVlog(f, "");
		break;

	case AST_CONSTANT:
		if (!str.empty())
			fprintf(f, "\"%s\"", str.c_str());
		else if (bits.size() == 32)
			fprintf(f, "%d", RTLIL::Const(bits).as_int());
		else
			fprintf(f, "%d'b %s", GetSize(bits), RTLIL::Const(bits).as_string().c_str());
		break;

	case AST_REALVALUE:
		fprintf(f, "%e", realvalue);
		break;

	case AST_BLOCK:
		if (children.size() == 1) {
			children[0]->dumpVlog(f, indent);
		} else {
			fprintf(f, "%s" "begin\n", indent.c_str());
			for (auto child : children)
				child->dumpVlog(f, indent + "  ");
			fprintf(f, "%s" "end\n", indent.c_str());
		}
		break;

	case AST_CASE:
		if (!children.empty() && children[0]->type == AST_CONDX)
			fprintf(f, "%s" "casex (", indent.c_str());
		else if (!children.empty() && children[0]->type == AST_CONDZ)
			fprintf(f, "%s" "casez (", indent.c_str());
		else
			fprintf(f, "%s" "case (", indent.c_str());
		children[0]->dumpVlog(f, "");
		fprintf(f, ")\n");
		for (size_t i = 1; i < children.size(); i++) {
			AstNode *child = children[i];
			child->dumpVlog(f, indent + "  ");
		}
		fprintf(f, "%s" "endcase\n", indent.c_str());
		break;

	case AST_COND:
	case AST_CONDX:
	case AST_CONDZ:
		for (auto child : children) {
			if (child->type == AST_BLOCK) {
				fprintf(f, ":\n");
				child->dumpVlog(f, indent + "  ");
				first = true;
			} else {
				fprintf(f, "%s", first ? indent.c_str() : ", ");
				if (child->type == AST_DEFAULT)
					fprintf(f, "default");
				else
					child->dumpVlog(f, "");
				first = false;
			}
		}
		break;

	case AST_ASSIGN:
		fprintf(f, "%sassign ", indent.c_str());
		children[0]->dumpVlog(f, "");
		fprintf(f, " = ");
		children[1]->dumpVlog(f, "");
		fprintf(f, ";\n");
		break;

	case AST_ASSIGN_EQ:
	case AST_ASSIGN_LE:
		fprintf(f, "%s", indent.c_str());
		children[0]->dumpVlog(f, "");
		fprintf(f, " %s ", type == AST_ASSIGN_EQ ? "=" : "<=");
		children[1]->dumpVlog(f, "");
		fprintf(f, ";\n");
		break;

	case AST_CONCAT:
		fprintf(f, "{");
		for (int i = GetSize(children)-1; i >= 0; i--) {
			auto child = children[i];
			if (!first)
				fprintf(f, ", ");
			child->dumpVlog(f, "");
			first = false;
		}
		fprintf(f, "}");
		break;

	case AST_REPLICATE:
		fprintf(f, "{");
		children[0]->dumpVlog(f, "");
		fprintf(f, "{");
		children[1]->dumpVlog(f, "");
		fprintf(f, "}}");
		break;

	if (0) { case AST_BIT_NOT:     txt = "~";  }
	if (0) { case AST_REDUCE_AND:  txt = "&";  }
	if (0) { case AST_REDUCE_OR:   txt = "|";  }
	if (0) { case AST_REDUCE_XOR:  txt = "^";  }
	if (0) { case AST_REDUCE_XNOR: txt = "~^"; }
	if (0) { case AST_REDUCE_BOOL: txt = "|";  }
	if (0) { case AST_POS:         txt = "+";  }
	if (0) { case AST_NEG:         txt = "-";  }
	if (0) { case AST_LOGIC_NOT:   txt = "!";  }
	if (0) { case AST_SELFSZ:      txt = "@selfsz@";  }
		fprintf(f, "%s(", txt.c_str());
		children[0]->dumpVlog(f, "");
		fprintf(f, ")");
		break;

	if (0) { case AST_BIT_AND:      txt = "&";   }
	if (0) { case AST_BIT_OR:       txt = "|";   }
	if (0) { case AST_BIT_XOR:      txt = "^";   }
	if (0) { case AST_BIT_XNOR:     txt = "~^";  }
	if (0) { case AST_SHIFT_LEFT:   txt = "<<";  }
	if (0) { case AST_SHIFT_RIGHT:  txt = ">>";  }
	if (0) { case AST_SHIFT_SLEFT:  txt = "<<<"; }
	if (0) { case AST_SHIFT_SRIGHT: txt = ">>>"; }
	if (0) { case AST_SHIFTX:       txt = "@shiftx@"; }
	if (0) { case AST_SHIFT:        txt = "@shift@"; }
	if (0) { case AST_LT:           txt = "<";   }
	if (0) { case AST_LE:           txt = "<=";  }
	if (0) { case AST_EQ:           txt = "==";  }
	if (0) { case AST_NE:           txt = "!=";  }
	if (0) { case AST_EQX:          txt = "===";  }
	if (0) { case AST_NEX:          txt = "!==";  }
	if (0) { case AST_GE:           txt = ">=";  }
	if (0) { case AST_GT:           txt = ">";   }
	if (0) { case AST_ADD:          txt = "+";   }
	if (0) { case AST_SUB:          txt = "-";   }
	if (0) { case AST_MUL:          txt = "*";   }
	if (0) { case AST_DIV:          txt = "/";   }
	if (0) { case AST_MOD:          txt = "%";   }
	if (0) { case AST_POW:          txt = "**";  }
	if (0) { case AST_LOGIC_AND:    txt = "&&";  }
	if (0) { case AST_LOGIC_OR:     txt = "||";  }
		fprintf(f, "(");
		children[0]->dumpVlog(f, "");
		fprintf(f, ")%s(", txt.c_str());
		children[1]->dumpVlog(f, "");
		fprintf(f, ")");
		break;

	case AST_TERNARY:
		fprintf(f, "(");
		children[0]->dumpVlog(f, "");
		fprintf(f, ") ? (");
		children[1]->dumpVlog(f, "");
		fprintf(f, ") : (");
		children[2]->dumpVlog(f, "");
		fprintf(f, ")");
		break;

	default:
		std::string type_name = type2str(type);
		fprintf(f, "%s" "/** %s **/%s", indent.c_str(), type_name.c_str(), indent.empty() ? "" : "\n");
		// dumpAst(f, indent, NULL);
	}

	fflush(f);
}

// check if two AST nodes are identical
bool AstNode::operator==(const AstNode &other) const
{
	if (type != other.type)
		return false;
	if (children.size() != other.children.size())
		return false;
	if (str != other.str)
		return false;
	if (bits != other.bits)
		return false;
	if (is_input != other.is_input)
		return false;
	if (is_output != other.is_output)
		return false;
	if (is_logic != other.is_logic)
		return false;
	if (is_reg != other.is_reg)
		return false;
	if (is_signed != other.is_signed)
		return false;
	if (is_string != other.is_string)
		return false;
	if (range_valid != other.range_valid)
		return false;
	if (range_swapped != other.range_swapped)
		return false;
	if (port_id != other.port_id)
		return false;
	if (range_left != other.range_left)
		return false;
	if (range_right != other.range_right)
		return false;
	if (integer != other.integer)
		return false;
	for (size_t i = 0; i < children.size(); i++)
		if (*children[i] != *other.children[i])
			return false;
	return true;
}

// check if two AST nodes are not identical
bool AstNode::operator!=(const AstNode &other) const
{
	return !(*this == other);
}

// check if this AST contains the given node
bool AstNode::contains(const AstNode *other) const
{
	if (this == other)
		return true;
	for (auto child : children)
		if (child->contains(other))
			return true;
	return false;
}

// create an AST node for a constant (using a 32 bit int as value)
AstNode *AstNode::mkconst_int(uint32_t v, bool is_signed, int width)
{
	AstNode *node = new AstNode(AST_CONSTANT);
	node->integer = v;
	node->is_signed = is_signed;
	for (int i = 0; i < width; i++) {
		node->bits.push_back((v & 1) ? State::S1 : State::S0);
		v = v >> 1;
	}
	node->range_valid = true;
	node->range_left = width-1;
	node->range_right = 0;
	return node;
}

// create an AST node for a constant (using a bit vector as value)
AstNode *AstNode::mkconst_bits(const std::vector<RTLIL::State> &v, bool is_signed, bool is_unsized)
{
	AstNode *node = new AstNode(AST_CONSTANT);
	node->is_signed = is_signed;
	node->bits = v;
	for (size_t i = 0; i < 32; i++) {
		if (i < node->bits.size())
			node->integer |= (node->bits[i] == State::S1) << i;
		else if (is_signed && !node->bits.empty())
			node->integer |= (node->bits.back() == State::S1) << i;
	}
	node->range_valid = true;
	node->range_left = node->bits.size()-1;
	node->range_right = 0;
	node->is_unsized = is_unsized;
	return node;
}

AstNode *AstNode::mkconst_bits(const std::vector<RTLIL::State> &v, bool is_signed)
{
	return mkconst_bits(v, is_signed, false);
}

// create an AST node for a constant (using a string in bit vector form as value)
AstNode *AstNode::mkconst_str(const std::vector<RTLIL::State> &v)
{
	AstNode *node = mkconst_str(RTLIL::Const(v).decode_string());
	while (GetSize(node->bits) < GetSize(v))
		node->bits.push_back(RTLIL::State::S0);
	log_assert(node->bits == v);
	return node;
}

// create an AST node for a constant (using a string as value)
AstNode *AstNode::mkconst_str(const std::string &str)
{
	std::vector<RTLIL::State> data;
	data.reserve(str.size() * 8);
	for (size_t i = 0; i < str.size(); i++) {
		unsigned char ch = str[str.size() - i - 1];
		for (int j = 0; j < 8; j++) {
			data.push_back((ch & 1) ? State::S1 : State::S0);
			ch = ch >> 1;
		}
	}
	AstNode *node = AstNode::mkconst_bits(data, false);
	node->is_string = true;
	node->str = str;
	return node;
}

bool AstNode::bits_only_01() const
{
	for (auto bit : bits)
		if (bit != State::S0 && bit != State::S1)
			return false;
	return true;
}

RTLIL::Const AstNode::bitsAsUnsizedConst(int width)
{
	RTLIL::State extbit = bits.back();
	while (width > int(bits.size()))
		bits.push_back(extbit);
	return RTLIL::Const(bits);
}

RTLIL::Const AstNode::bitsAsConst(int width, bool is_signed)
{
	std::vector<RTLIL::State> bits = this->bits;
	if (width >= 0 && width < int(bits.size()))
		bits.resize(width);
	if (width >= 0 && width > int(bits.size())) {
		RTLIL::State extbit = RTLIL::State::S0;
		if (is_signed && !bits.empty())
			extbit = bits.back();
		while (width > int(bits.size()))
			bits.push_back(extbit);
	}
	return RTLIL::Const(bits);
}

RTLIL::Const AstNode::bitsAsConst(int width)
{
	return bitsAsConst(width, is_signed);
}

RTLIL::Const AstNode::asAttrConst()
{
	log_assert(type == AST_CONSTANT);

	RTLIL::Const val;
	val.bits = bits;

	if (is_string) {
		val.flags |= RTLIL::CONST_FLAG_STRING;
		log_assert(val.decode_string() == str);
	}

	return val;
}

RTLIL::Const AstNode::asParaConst()
{
	RTLIL::Const val = asAttrConst();
	if (is_signed)
		val.flags |= RTLIL::CONST_FLAG_SIGNED;
	return val;
}

bool AstNode::asBool() const
{
	log_assert(type == AST_CONSTANT);
	for (auto &bit : bits)
		if (bit == RTLIL::State::S1)
			return true;
	return false;
}

int AstNode::isConst() const
{
	if (type == AST_CONSTANT)
		return 1;
	if (type == AST_REALVALUE)
		return 2;
	return 0;
}

uint64_t AstNode::asInt(bool is_signed)
{
	if (type == AST_CONSTANT)
	{
		RTLIL::Const v = bitsAsConst(64, is_signed);
		uint64_t ret = 0;

		for (int i = 0; i < 64; i++)
			if (v.bits.at(i) == RTLIL::State::S1)
				ret |= uint64_t(1) << i;

		return ret;
	}

	if (type == AST_REALVALUE)
		return uint64_t(realvalue);

	log_abort();
        return 0;
}

double AstNode::asReal(bool is_signed)
{
	if (type == AST_CONSTANT)
	{
		RTLIL::Const val(bits);

		bool is_negative = is_signed && !val.bits.empty() && val.bits.back() == RTLIL::State::S1;
		if (is_negative)
			val = const_neg(val, val, false, false, val.bits.size());

		double v = 0;
		for (size_t i = 0; i < val.bits.size(); i++)
			// IEEE Std 1800-2012 Par 6.12.2: Individual bits that are x or z in
			// the net or the variable shall be treated as zero upon conversion.
			if (val.bits.at(i) == RTLIL::State::S1)
				v += exp2(i);
		if (is_negative)
			v *= -1;

		return v;
	}

	if (type == AST_REALVALUE)
		return realvalue;

	log_abort();
        return 0.0;
}

RTLIL::Const AstNode::realAsConst(int width)
{
	double v = round(realvalue);
	RTLIL::Const result;
#ifdef EMSCRIPTEN
	if (!isfinite(v)) {
#else
	if (!std::isfinite(v)) {
#endif
		result.bits = std::vector<RTLIL::State>(width, RTLIL::State::Sx);
	} else {
		bool is_negative = v < 0;
		if (is_negative)
			v *= -1;
		for (int i = 0; i < width; i++, v /= 2)
			result.bits.push_back((fmod(floor(v), 2) != 0) ? RTLIL::State::S1 : RTLIL::State::S0);
		if (is_negative)
			result = const_neg(result, result, false, false, result.bits.size());
	}
	return result;
}

// AstModule destructor
AstModule::~AstModule()
{
	if (ast != NULL)
		delete ast;
}


// An interface port with modport is specified like this:
//    <interface_name>.<modport_name>
// This function splits the interface_name from the modport_name, and fails if it is not a valid combination
std::pair<std::string,std::string> AST::split_modport_from_type(std::string name_type)
{
	std::string interface_type = "";
	std::string interface_modport = "";
	size_t ndots = std::count(name_type.begin(), name_type.end(), '.');
	// Separate the interface instance name from any modports:
	if (ndots == 0) { // Does not have modport
		interface_type = name_type;
	}
	else {
		std::stringstream name_type_stream(name_type);
		std::string segment;
		std::vector<std::string> seglist;
		while(std::getline(name_type_stream, segment, '.')) {
			seglist.push_back(segment);
		}
		if (ndots == 1) { // Has modport
			interface_type = seglist[0];
			interface_modport = seglist[1];
		}
		else { // Erroneous port type
			log_error("More than two '.' in signal port type (%s)\n", name_type.c_str());
		}
	}
	return std::pair<std::string,std::string>(interface_type, interface_modport);

}

AstNode * AST::find_modport(AstNode *intf, std::string name)
{
	for (auto &ch : intf->children)
		if (ch->type == AST_MODPORT)
			if (ch->str == name) // Modport found
				return ch;
	return NULL;
}

// Iterate over all wires in an interface and add them as wires in the AST module:
void AST::explode_interface_port(AstNode *module_ast, RTLIL::Module * intfmodule, std::string intfname, AstNode *modport)
{
	for (auto w : intfmodule->wires()){
		AstNode *wire = new AstNode(AST_WIRE, new AstNode(AST_RANGE, AstNode::mkconst_int(w->width -1, true), AstNode::mkconst_int(0, true)));
		std::string origname = log_id(w->name);
		std::string newname = intfname + "." + origname;
		wire->str = newname;
		if (modport != NULL) {
			bool found_in_modport = false;
			// Search for the current wire in the wire list for the current modport
			for (auto &ch : modport->children) {
				if (ch->type == AST_MODPORTMEMBER) {
					std::string compare_name = "\\" + origname;
					if (ch->str == compare_name) { // Found signal. The modport decides whether it is input or output
						found_in_modport = true;
						wire->is_input = ch->is_input;
						wire->is_output = ch->is_output;
						break;
					}
				}
			}
			if (found_in_modport) {
				module_ast->children.push_back(wire);
			}
			else { // If not found in modport, do not create port
				delete wire;
			}
		}
		else { // If no modport, set inout
			wire->is_input = true;
			wire->is_output = true;
			module_ast->children.push_back(wire);
		}
	}
}

RTLIL::Module *AstModule::clone() const
{
	AstModule *new_mod = new AstModule;
	new_mod->name = name;
	cloneInto(new_mod);

	new_mod->ast = ast->clone();
	new_mod->nolatches = nolatches;
	new_mod->nomeminit = nomeminit;
	new_mod->nomem2reg = nomem2reg;
	new_mod->mem2reg = mem2reg;
	new_mod->noblackbox = noblackbox;
	new_mod->lib = lib;
	new_mod->nowb = nowb;
	new_mod->noopt = noopt;
	new_mod->icells = icells;
	new_mod->pwires = pwires;
	new_mod->autowire = autowire;

	return new_mod;
}

void AstModule::loadconfig() const
{
	current_ast = NULL;
	flag_dump_ast1 = false;
	flag_dump_ast2 = false;
	flag_dump_vlog1 = false;
	flag_dump_vlog2 = false;
	flag_nolatches = nolatches;
	flag_nomeminit = nomeminit;
	flag_nomem2reg = nomem2reg;
	flag_mem2reg = mem2reg;
	flag_noblackbox = noblackbox;
	flag_lib = lib;
	flag_nowb = nowb;
	flag_noopt = noopt;
	flag_icells = icells;
	flag_pwires = pwires;
	flag_autowire = autowire;
}

YOSYS_NAMESPACE_END
