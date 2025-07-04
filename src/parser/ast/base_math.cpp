#include "base_math.hpp"
#include "../../lexer/lexer.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "../../build/optimizer_flags.hpp"
#include "ast.hpp"
#include "func.hpp"
#include "literal.hpp"
#include "type.hpp"
#include "var.hpp"
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>

// #define DEBUG

CstType DoubleOperandAST::getCstType() const { return parser::hasOp(left->getCstType(), right->getCstType(), op); }
LLType  DoubleOperandAST::getLLType() const { return parser::LLType(getCstType()); }
uint64  DoubleOperandAST::nodeSize() const { return left->nodeSize() + right->nodeSize() + 1; }
String  DoubleOperandAST::_str() const {
    return "<"s + str(left.get()) + op_view + str(right.get()) + (is_const ? " [="s + value + "]>" : ">"s );
}
String DoubleOperandAST::emitCST() const {
    return "("s + left->emitCST() + " " + op_view + " " + right->emitCST() + ")";
}
void DoubleOperandAST::forceType(CstType type) {
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool       int_required   = std::regex_match(type, i);
    bool       float_required = std::regex_match(type, f);

    // force the types of the sub-trees
    if (std::regex_match(left->getCstType(), i) && int_required) {
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), f) && float_required) {
        left->forceType(type);
        right->forceType(type);
    }

    // check for operator overloading [WIP/TODO]
    CstType ret = parser::hasOp(left->getCstType(), right->getCstType(), op);
    if (ret != "") {
        if (ret != type)
            parser::error("Mismatiching types", tokens,
                          left->getCstType() + "::operator " + op_view + " (" + right->getCstType() + ") yields " +
                              ret + " (expected \e[1m" + type + "\e[0m)",
                          18);

        else if (optimizer::do_constant_folding && right->is_const && left->is_const) {
            //std::cout << op_view << "constant_folding" << std::endl;
            if (const_folding_fn.count(std::make_tuple(left->getCstType(),right->getCstType()))) {
                value = const_folding_fn[std::make_tuple(left->getCstType(),right->getCstType())](left->value,right->value);
                is_const = true;
            }
        }
    } else
        parser::error("Unknown operator", tokens,
                      left->getCstType() + "::operator " + op_view + " (" + right->getCstType() + ") is not implemented.", 18);
}

CstType UnaryOperandAST::getCstType() const { return parser::hasOp(left->getCstType(), left->getCstType(), op); }
LLType  UnaryOperandAST::getLLType() const { return parser::LLType(getCstType()); }
uint64  UnaryOperandAST::nodeSize() const { return left->nodeSize() + 1; }
void    UnaryOperandAST::forceType(CstType type) {

    String ret = parser::hasOp(left->getCstType(), left->getCstType(), op);
    if (ret != "") {
        if (ret != type)
            parser::error("Mismatiching types", tokens,
                          left->getCstType() + "::operator " + op_view + " () yields " + ret + " (expected \e[1m" +
                              type + "\e[0m)",
                          18);

        else if (optimizer::do_constant_folding && left->is_const) {
            if (const_folding_fn.count(left->getCstType())) {
                value = const_folding_fn[left->getCstType()](left->value);
                is_const = true;
            }
        }
    } else
        parser::error("Unknown operator", tokens,
                      left->getCstType() + "::operator " + op_view + " () is not implemented.", 18);
}

String UnaryOperandAST::emitCST() const { return op_view + left->emitCST(); }

String UnaryOperandAST::_str() const {
    return "<"s + op_view + str(left.get()) + (is_const ? " [="s + value + "]>" : ">"s );
}

//  AddAST

#define CF_FUN_INT(type1, type2, type, op) {std::make_tuple(type1, type2), nlambda (String v1, String v2) {return std::to_string(type(std::stoll(v1) op std::stoll(v2)));} }
#define CF_FUN_FLT(type1, type2, type, op) {std::make_tuple(type1, type2), nlambda (String v1, String v2) {return std::to_string(type(std::stold(v1) op std::stold(v2)));} }


AddAST::AddAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left    = left;
    this->right   = right;
    this->tokens  = tokens;
    this->op      = lexer::Token::ADD;
    this->op_view = "+";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, +),
        CF_FUN_INT("uint16", "uint16", uint16, +),
        CF_FUN_INT("uint32", "uint32", uint32, +),
        CF_FUN_INT("uint64", "uint64", uint64, +),

        CF_FUN_INT("int8", "int8", int8, +),
        CF_FUN_INT("int16", "int16", int16, +),
        CF_FUN_INT("int32", "int32", int32, +),
        CF_FUN_INT("int64", "int64", int64, +),

        CF_FUN_FLT("float16", "float16", float32, +),
        CF_FUN_FLT("float32", "float32", float32, +),
        CF_FUN_FLT("float64", "float64", float64, +),
        CF_FUN_FLT("float80", "float80", float80, +),
    };
}

AddAST::~AddAST(){};

String AddAST::emitLL(int* locc, String inp) const {

    String op  = left->getLLType()[0] == 'i' ? "add" : "fadd contract nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert(String("%") + std::to_string(++(*locc)), r);
    inp        = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

sptr<AST> AddAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {

    if (tokens.size() < 1)
        return nullptr;

    auto t = tokens; //> used to chache the original tokens

    lexer::Token first;                //> first token
    bool         first_is_sub = false; //> first token is SUB cache

    // first token is SUB => negation (TODO)
    if (tokens[0].type == lexer::Token::Type::SUB) {
        first        = tokens[0];
        first_is_sub = true;
        tokens       = parser::subvector(tokens, 1, 1, tokens.size());
    }

    // find position of operator
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::ADD, lexer::Token::Type::SUB}, local);
    if (first_is_sub)
        tokens.insert(tokens.begin(), first);
    if (tokens.size() > 2 && split != 0 && split < tokens.size() - 1) {
#ifdef DEBUG
        std::cout << "AddAST::parse:\tfirst_is_sub:\t" << first_is_sub << std::endl;
        std::cout << "AddAST::parse:\tvalue size:\t" << tokens.size() << std::endl;
        std::cout << "AddAST::parse:\tsplit:\t" << split << std::endl;
#endif

        lexer::Token& op = tokens[split + first_is_sub]; //> operator token
        sptr<AST> left   = math::parse(parser::subvector(tokens, 0, 1, split + first_is_sub), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split + first_is_sub - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + first_is_sub + 1, 1, tokens.size()), local, sr,
                                      expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split + first_is_sub], tokens[tokens.size() - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::ADD)
            return share<AST>(new AddAST(left, right, t));

        else if (op.type == lexer::Token::Type::SUB)
            return share<AST>(new SubAST(left, right, t));
    }

    return nullptr;
}

// SubAST

SubAST::SubAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::SUB;
    this->op_view = "-";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, -),
        CF_FUN_INT("uint16", "uint16", uint16, -),
        CF_FUN_INT("uint32", "uint32", uint32, -),
        CF_FUN_INT("uint64", "uint64", uint64, -),

        CF_FUN_INT("int8", "int8", int8, -),
        CF_FUN_INT("int16", "int16", int16, -),
        CF_FUN_INT("int32", "int32", int32, -),
        CF_FUN_INT("int64", "int64", int64, -),

        CF_FUN_FLT("float16", "float16", float32, -),
        CF_FUN_FLT("float32", "float32", float32, -),
        CF_FUN_FLT("float64", "float64", float64, -),
        CF_FUN_FLT("float80", "float80", float80, -),
    };
}

SubAST::~SubAST(){};

String SubAST::emitLL(int* locc, String inp) const {

    String op  = left->getLLType()[0] == 'i' ? "sub" : "fsub contract nsz";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}

// MulAST

MulAST::MulAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::MUL;
    this->op_view = "*";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, *),
        CF_FUN_INT("uint16", "uint16", uint16, *),
        CF_FUN_INT("uint32", "uint32", uint32, *),
        CF_FUN_INT("uint64", "uint64", uint64, *),

        CF_FUN_INT("int8", "int8", int8, *),
        CF_FUN_INT("int16", "int16", int16, *),
        CF_FUN_INT("int32", "int32", int32, *),
        CF_FUN_INT("int64", "int64", int64, *),

        CF_FUN_FLT("float16", "float16", float32, *),
        CF_FUN_FLT("float32", "float32", float32, *),
        CF_FUN_FLT("float64", "float64", float64, *),
        CF_FUN_FLT("float80", "float80", float80, *),
    };
}

MulAST::~MulAST(){};

String MulAST::emitLL(int* locc, String inp) const {

    String op  = left->getLLType()[0] == 'i' ? "mul" : "fmul contract arcp nsz";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}

sptr<AST> MulAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t = tokens;
    size_t split =
        parser::splitStack(tokens, {lexer::Token::Type::MUL, lexer::Token::Type::DIV, lexer::Token::Type::MOD}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "MulAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>    left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::MUL)
            return share<AST>(new MulAST(left, right, t));
        else if (op.type == lexer::Token::Type::DIV)
            return share<AST>(new DivAST(left, right, t));
        else if (op.type == lexer::Token::Type::MOD)
            return share<AST>(new ModAST(left, right, t));
    }

    return nullptr;
}

// DivAST

DivAST::DivAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::DIV;
    this->op_view = "/";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, /),
        CF_FUN_INT("uint16", "uint16", uint16, /),
        CF_FUN_INT("uint32", "uint32", uint32, /),
        CF_FUN_INT("uint64", "uint64", uint64, /),

        CF_FUN_INT("int8", "int8", int8, /),
        CF_FUN_INT("int16", "int16", int16, /),
        CF_FUN_INT("int32", "int32", int32, /),
        CF_FUN_INT("int64", "int64", int64, /),

        CF_FUN_FLT("float16", "float16", float32, /),
        CF_FUN_FLT("float32", "float32", float32, /),
        CF_FUN_FLT("float64", "float64", float64, /),
        CF_FUN_FLT("float80", "float80", float80, /),
    };
}

DivAST::~DivAST(){};

String DivAST::emitLL(int* locc, String inp) const {

    String op = left->getLLType()[0] == 'i' ? left->getCstType()[0] == 'u' ? "udiv" : "sdiv" : "fdiv contract arcp nsz";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}

// ModAST

ModAST::ModAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::MOD;
    this->op_view = "%";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, %),
        CF_FUN_INT("uint16", "uint16", uint16, %),
        CF_FUN_INT("uint32", "uint32", uint32, %),
        CF_FUN_INT("uint64", "uint64", uint64, %),

        CF_FUN_INT("int8", "int8", int8, %),
        CF_FUN_INT("int16", "int16", int16, %),
        CF_FUN_INT("int32", "int32", int32, %),
        CF_FUN_INT("int64", "int64", int64, %),
    };
}

ModAST::~ModAST(){};

String ModAST::emitLL(int* locc, String inp) const {
    String op = left->getLLType()[0] == 'i' ? left->getCstType()[0] == 'u' ? "urem" : "srem" : "frem contract arcp nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert(String("%") + std::to_string(++(*locc)), r);
    inp        = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

// PowAST

#define CF_FUN_POW(type, type2) {std::make_tuple(type, type), nlambda (String v1, String v2) {return std::to_string(type2(std::pow(float80(std::stoll(v1)),float80(std::stoll(v2)))));} }
#define CF_FUN_POW_FLT(type, type2) {std::make_tuple(type, type), nlambda (String v1, String v2) {return std::to_string(type2(std::pow(float80(std::stold(v1)),float80(std::stold(v2)))));} }


PowAST::PowAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::POW;
    this->op_view = "**";
    this->const_folding_fn = {
        CF_FUN_POW("uint8", uint8),
        CF_FUN_POW("uint16", uint16),
        CF_FUN_POW("uint32", uint32),
        CF_FUN_POW("uint64", uint64),

        CF_FUN_POW("int8", int8),
        CF_FUN_POW("int16", int16),
        CF_FUN_POW("int32", int32),
        CF_FUN_POW("int64", int64),

        CF_FUN_POW_FLT("float16", float32),
        CF_FUN_POW_FLT("float32", float32),
        CF_FUN_POW_FLT("float64", float64),
        CF_FUN_POW_FLT("float80", float80),
    };
}

PowAST::~PowAST(){};

String PowAST::emitLL(int*, String) const { return ""; }

sptr<AST> PowAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t     = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::POW}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "PowAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>         left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        return share<AST>(new PowAST(left, right, t));
    }

    return nullptr;
}

// LorAST

LorAST::LorAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::LOR;
    this->op_view = "||";
    this->const_folding_fn = {
        {std::make_tuple("bool","bool"), nlambda (String v1, String v2) {return (v1 == "true"s || v2 == "true")? "true"s : "false"s;} }
    };
}

LorAST::~LorAST(){};

String LorAST::emitLL(int* locc, String inp) const {
    String op  = "or";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}
sptr<AST> LorAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t     = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LOR}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "LorAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>         left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::LOR)
            return share<AST>(new LorAST(left, right, t));
    }

    return nullptr;
}

// LandAST

LandAST::LandAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::LAND;
    this->op_view = "&&";
    this->const_folding_fn = {
        {std::make_tuple("bool","bool"), nlambda (String v1, String v2) {return (v1 == "true"s && v2 == "true")? "true"s : "false"s;} }
    };
}

LandAST::~LandAST(){};

String LandAST::emitLL(int* locc, String inp) const {
    String op  = "and";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}
sptr<AST> LandAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t     = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LAND}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "LandAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>         left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::LAND)
            return share<AST>(new LandAST(left, right, t));
    }

    return nullptr;
}

// OrAST

OrAST::OrAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::OR;
    this->op_view = "|";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, |),
        CF_FUN_INT("uint16", "uint16", uint16, |),
        CF_FUN_INT("uint32", "uint32", uint32, |),
        CF_FUN_INT("uint64", "uint64", uint64, |),

        CF_FUN_INT("int8", "int8", int8, |),
        CF_FUN_INT("int16", "int16", int16, |),
        CF_FUN_INT("int32", "int32", int32, |),
        CF_FUN_INT("int64", "int64", int64, |),
    };
}
String OrAST::emitLL(int* locc, String inp) const {
    String op  = "or";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}
sptr<AST> OrAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t     = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LOR}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "OrAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>         left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::OR)
            return share<AST>(new OrAST(left, right, t));
    }

    return nullptr;
}

// AndAST

AndAST::AndAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::AND;
    this->op_view = "&";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, &),
        CF_FUN_INT("uint16", "uint16", uint16, &),
        CF_FUN_INT("uint32", "uint32", uint32, &),
        CF_FUN_INT("uint64", "uint64", uint64, &),

        CF_FUN_INT("int8", "int8", int8, &),
        CF_FUN_INT("int16", "int16", int16, &),
        CF_FUN_INT("int32", "int32", int32, &),
        CF_FUN_INT("int64", "int64", int64, &),
    };
}

String AndAST::emitLL(int* locc, String inp) const {
    String op  = "and";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}
sptr<AST> AndAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t     = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LAND}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "AndAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>         left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::AND)
            return share<AST>(new AndAST(left, right, t));
    }

    return nullptr;
}

// XorAST

XorAST::XorAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens) {
    this->left   = left;
    this->right  = right;
    this->tokens = tokens;
    this->op     = lexer::Token::XOR;
    this->op_view = "^";
    this->const_folding_fn = {
        CF_FUN_INT("uint8", "uint8", uint8, ^),
        CF_FUN_INT("uint16", "uint16", uint16, ^),
        CF_FUN_INT("uint32", "uint32", uint32, ^),
        CF_FUN_INT("uint64", "uint64", uint64, ^),

        CF_FUN_INT("int8", "int8", int8, ^),
        CF_FUN_INT("int16", "int16", int16, ^),
        CF_FUN_INT("int32", "int32", int32, ^),
        CF_FUN_INT("int64", "int64", int64, ^),
    };
}

String XorAST::emitLL(int* locc, String inp) const {
    String op  = "xor";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}
sptr<AST> XorAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 1)
        return nullptr;
    auto   t     = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LAND}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()) {
#ifdef DEBUG
        std::cout << "AndAST::parse:\tsplit:\t" << split << std::endl;
#endif
        lexer::Token op   = tokens[split];
        sptr<AST>         left = math::parse(parser::subvector(tokens, 0, 1, split), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected", {tokens[0], tokens[split - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        sptr<AST> right = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected", {tokens[split], tokens[tokens.size() - 1]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", 111);
            return share<AST>(new AST());
        }
        if (op.type == lexer::Token::Type::XOR)
            return share<AST>(new XorAST(left, right, t));
    }

    return nullptr;
}

// NotAST

NotAST::NotAST(sptr<AST> inner, std::vector<lexer::Token> tokens) {
    this->left   = inner;
    this->tokens = tokens;
    this->op     = lexer::Token::NOT;
    this->op_view = "!";
    this->const_folding_fn = {
        {"bool", nlambda (String v) {return (v == "true" ? "false"s : "true"s);} }
    };
}

sptr<AST> NotAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() == 0)
        return nullptr;
    if (tokens.at(0).type == lexer::Token::NOT) {
        sptr<AST> of = math::parse(parser::subvector(tokens, 1, 1, tokens.size()), local, sr);
        if (of == nullptr) {
            parser::error("Expression expected", {tokens[0]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m after '!'", 111);
            return share<AST>(new AST);
        }
        return share<NotAST>(new NotAST(of, tokens));
    }

    return nullptr;
}

// NegAST

NegAST::NegAST(sptr<AST> inner, std::vector<lexer::Token> tokens) {
    this->left    = inner;
    this->tokens  = tokens;
    this->op      = lexer::Token::NEG;
    this->op_view = "~";
    this->const_folding_fn = {
        {"uint8"s,  nlambda (String v) {return std::to_string(~(uint8) (std::stoi(v)));}  },
        {"uint16"s, nlambda (String v) {return std::to_string(~(uint16)(std::stoi(v)));}  },
        {"uint32"s, nlambda (String v) {return std::to_string(~(uint32)(std::stol(v)));}  },
        {"uint64"s, nlambda (String v) {return std::to_string(~(uint64)(std::stoll(v)));} },
        
        {"int8"s,  nlambda (String v) {return std::to_string(~(int8) (std::stoi(v)));}  },
        {"int16"s, nlambda (String v) {return std::to_string(~(int16)(std::stoi(v)));}  },
        {"int32"s, nlambda (String v) {return std::to_string(~(int32)(std::stol(v)));}  },
        {"int64"s, nlambda (String v) {return std::to_string(~(int64)(std::stoll(v)));} },
    };
}

sptr<AST> NegAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() == 0)
        return nullptr;
    if (tokens.at(0).type == lexer::Token::NEG) {
        sptr<AST> of = math::parse(parser::subvector(tokens, 1, 1, tokens.size()), local, sr);
        if (of == nullptr) {
            parser::error("Expression expected", {tokens[0]},
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m after '~'", 111);
            return share<AST>(new AST);
        }
        return share<NotAST>(new NotAST(of, tokens));
    }

    return nullptr;
}

// AddrOfAST

AddrOfAST::AddrOfAST(sptr<AST> of) { this->of = of; }

AddrOfAST::~AddrOfAST(){};
String AddrOfAST::getLLType() const { return of->getLLType() + "*"; }

String AddrOfAST::emitCST() const { return String("#") + of->emitCST(); }
String AddrOfAST::emitLL(int*, String) const { return ""; }

sptr<AST> math::parse_pt(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() < 2)
        return nullptr;
    if (tokens[0].type == lexer::Token::Type::OPEN) {
        if (tokens[tokens.size() - 1].type == lexer::Token::Type::CLOSE) {
#ifdef DEBUG
            std::cout << "parse_pt" << std::endl;
#endif
            return math::parse(parser::subvector(tokens, 1, 1, tokens.size() - 1), local + 1, sr, expected_type);
        }
    }

    return nullptr;
}

CastAST::CastAST(sptr<AST> from, sptr<AST> type, std::vector<lexer::Token> tokens) {
    this->from   = from;
    this->type   = type;
    this->tokens = tokens;
}

sptr<AST> CastAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String) {
    if (tokens.size() < 3)
        return nullptr;
    int split = parser::rsplitStack(tokens, {lexer::Token::Type::AS}, local);
#ifdef DEBUG
    std::cout << "CastAST::parse:\tsplit:\t" << split << std::endl;
#endif
    if (split == (int)tokens.size() - 1) {
        parser::error("Expected type", {tokens[tokens.size() - 1]}, "Expected a type after 'as'", 25);
        return share<AST>(new AST);
    }
    if (split == (int)tokens.size())
        return nullptr;
    if (split == 0) {
        parser::error("Expression expected", {tokens[0]}, "Expected a valid expression", 31);
        return share<AST>(new AST());
    }
    auto buf  = parser::subvector(tokens, split + 1, 1, tokens.size());
    sptr<AST> type = Type::parse(buf, local, sr);
    if (type == nullptr) {
        parser::error("Expected type", buf, "Expected a type after 'as'", 25);
        return share<AST>(new AST);
    }

    buf       = parser::subvector(tokens, 0, 1, split);
    sptr<AST> expr = math::parse(buf, local, sr);
    if (expr == nullptr) {
        parser::error("Expression expected", {tokens[0]}, "Expected a valid expression", 31);
        return share<AST>(new AST());
    }

    return share<AST>(new CastAST(expr, type, tokens));
}

String CastAST::emitCST() const { return from->emitCST() + " as " + type->emitCST(); }

void CastAST::forceType(String t) {
    if (getCstType() != t) {
        parser::error("Type mismatch", tokens,
                      String("expected a \e[1m") + t + "\e[0m, got a variable cast returning " + getCstType(), 17,
                      "Caused by");
    }
}

String CastAST::emitLL(int* locc, String inp) const {
    String in_type  = from->getCstType();
    String out_type = type->getCstType();

    if (in_type == "char")
        in_type = "int16"; // chars are unicode
    if (out_type == "char")
        out_type = "int16"; // chars are unicode
    if (in_type == "bool")
        in_type = "int1";
    if (out_type == "bool")
        out_type = "int1";
    String     op = "";
    String     s  = "";
    std::regex i("u?int(1|8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");

    // From int to int
    if (std::regex_match(in_type, i) && std::regex_match(out_type, i)) {
        int bits_in  = std::stoi(in_type.substr(3 + uint(in_type[0] == 'u')));
        int bits_out = std::stoi(out_type.substr(3 + uint(out_type[0] == 'u')));

        if (bits_in > bits_out)
            op = String("{} = trunc ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (bits_in < bits_out)
            op = String("{} = zext ") + (in_type[0] == 'u' ? String(" ") : String("")) + parser::LLType(in_type) +
                 " {} to " + parser::LLType(out_type) + "\n";
        if (op != "") {
            s   = from->emitLL(locc, op);
            s   = insert(String("%") + std::to_string(++(*locc)), s);
            inp = rinsert(String("%") + std::to_string(*locc), inp);
        } else {
            inp = from->emitLL(locc, inp);
        }
    }
    // From float to float
    else if (std::regex_match(in_type, f) && std::regex_match(out_type, f)) {
        int bits_in  = std::stoi(in_type.substr(5));
        int bits_out = std::stoi(out_type.substr(5));

        if (bits_in > bits_out)
            op = String("{} = fptrunc nsz ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (bits_in < bits_out)
            op = String("{} = fpext nsz ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (op != "") {
            s   = from->emitLL(locc, op);
            s   = insert(String("%") + std::to_string(++(*locc)), s);
            inp = rinsert(String("%") + std::to_string(*locc), inp);
        } else {
            inp = from->emitLL(locc, inp);
        }
    }
    // From int to float
    else if (std::regex_match(in_type, i) && std::regex_match(out_type, f)) {
        // int bits_in = std::stoi(in_type.substr(5));
        // int bits_out = std::stoi(out_type.substr(5));

        op       = in_type[0] == 'u' ? String("uitofp") : String("sitofp");
        String s = String("{} = ") + op + " " + parser::LLType(in_type) + " {} to " + parser::LLType(out_type);
        s        = from->emitLL(locc, op);
        s        = insert(String("%") + std::to_string(++(*locc)), s);
        inp      = rinsert(String("%") + std::to_string(*locc), inp);
    }

    // float to int
    else if (std::regex_match(in_type, f) && std::regex_match(out_type, i)) {
        // int bits_in = std::stoi(in_type.substr(5));
        // int bits_out = std::stoi(out_type.substr(5));

        op       = out_type[0] == 'u' ? String("fptoui") : String("fptosi");
        String s = String("{} = ") + op + " " + parser::LLType(in_type) + " {} to " + parser::LLType(out_type);
        s        = from->emitLL(locc, op);
        s        = insert("%"s + std::to_string(++(*locc)), s);
        inp      = rinsert("%"s + std::to_string(*locc), inp);
    }
    if (out_type == in_type + '?') {
        s = from->emitLL(locc, op);
        s = insert(String("%") + std::to_string((*locc)), s);
        inp =
            rinsert("{ "s + parser::LLType(in_type) + " " + String("%") + std::to_string(*locc) + " , i1 false }", inp);
    }

    return s + inp;
}

sptr<AST> CheckAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() == 0)
        return nullptr;
    if (tokens.at(tokens.size() - 1).type == lexer::Token::Type::QM) {
        sptr<AST> of = math::parse(parser::subvector(tokens, 0, 1, tokens.size() - 1), local, sr, expected_type);
        if (of == nullptr) {
            if (tokens.size() == 1) {
                parser::error("Expected expression", tokens, "expected an expression before '?'", 31);
            } else {
                parser::error("Expected expression", parser::subvector(tokens, 0, 1, tokens.size() - 1),
                              "expected a valid expression before '?'", 31);
            }
            return nullptr;
        }
        return share<AST>(new CheckAST(of, tokens));
    }

    return nullptr;
}

void CheckAST::forceType(CstType type) {
    if (!(type != of->getCstType() + '?')) {
        parser::error("Type mismatch", tokens,
                      String("expected a \e[1m") + type + "\e[0m, got an optional check returning " + getCstType(), 17);
    }
}

String CheckAST::emitLL(int* locc, String inp) const {
    String s = of->emitLL(locc, "");
    s += "%"s + std::to_string(++(*locc)) + " = extractvalue " + parser::LLType(of->getCstType()) + " %" +
         std::to_string((*locc)) + ", 1\n";
    s += "br i1 %"s + std::to_string((*locc)) + ", label %l" + std::to_string((*locc)) + ", label %l" +
         std::to_string((*locc) + 1) + "\n";
    s += "l" + std::to_string((*locc)) + ":\n";
    s += "l" + std::to_string((*locc) + 1) + ":\n";
    s += "%"s + std::to_string(++(*locc)) + " = extractvalue " + parser::LLType(of->getCstType()) + " %" +
         std::to_string((*locc) - 1) + ", 0\n";
    return s + insert("%" + std::to_string(((*locc)++)), inp);
}

sptr<AST> math::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
#ifdef DEBUG
    std::cout << "math::parse" << std::endl;
#endif
    return parser::parseOneOf(tokens,
                              {IntLiteralAST::parse, FloatLiteralAST::parse, BoolLiteralAST::parse,
                               CharLiteralAST::parse, StringLiteralAST::parse, VarAccesAST::parse, VarSetAST::parse,

                               AddAST::parse, MulAST::parse, PowAST::parse, LandAST::parse, LorAST::parse, NotAST::parse, NegAST::parse,

                               CastAST::parse, CheckAST::parse, FuncCallAST::parse, parse_pt},
                              local, sr, expected_type);
}
