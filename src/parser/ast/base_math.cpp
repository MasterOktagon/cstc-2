#include "base_math.hpp"

#include "../../build/optimizer_flags.hpp"
#include "../../debug/debug.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "func.hpp"
#include "literal.hpp"
#include "type.hpp"
#include "var.hpp"

// #include <catch2/catch.hpp>
#include <cmath>
#include <map>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

CstType DoubleOperandAST::getCstType() const {
    return parser::hasOp(left->getCstType(), right->getCstType(), op);
}

LLType DoubleOperandAST::getLLType() const {
    return parser::LLType(getCstType());
}

uint64 DoubleOperandAST::nodeSize() const {
    return left->nodeSize() + right->nodeSize() + 1;
}

String DoubleOperandAST::_str() const {
    return "<"s + str(left.get()) + op_view + str(right.get()) + (is_const ? " [="s + value + "]>" : ">"s);
}

String DoubleOperandAST::emitCST() const {
    return PUT_PT(left->emitCST() + " " + op_view + " " + right->emitCST(), this->has_pt);
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
        if (ret != type) {
            parser::error("Mismatiching types",
                          tokens,
                          left->getCstType() + "::operator " + op_view + " (" + right->getCstType() + ") yields " +
                              ret + " (expected \e[1m" + type + "\e[0m)",
                          18);
        }

        else if (optimizer::do_constant_folding && right->is_const && left->is_const) {
            // std::cout << op_view << "constant_folding" << std::endl;
            if (const_folding_fn.count(std::make_tuple(left->getCstType(), right->getCstType()))) {
                value    = const_folding_fn[std::make_tuple(left->getCstType(), right->getCstType())](left->value,
                                                                                                   right->value);
                is_const = true;
            }
        }
    } else {
        parser::error("Unknown operator",
                      tokens,
                      left->getCstType() + "::operator " + op_view + " (" + right->getCstType() +
                          ") is not implemented.",
                      18);
    }
}

CstType UnaryOperandAST::getCstType() const {
    return parser::hasOp(left->getCstType(), left->getCstType(), op);
}

LLType UnaryOperandAST::getLLType() const {
    return parser::LLType(getCstType());
}

uint64 UnaryOperandAST::nodeSize() const {
    return left->nodeSize() + 1;
}

void UnaryOperandAST::forceType(CstType type) {
    String ret = parser::hasOp(left->getCstType(), left->getCstType(), op);
    if (ret != "") {
        if (ret != type) {
            parser::error("Mismatiching types",
                          tokens,
                          left->getCstType() + "::operator " + op_view + " () yields " + ret + " (expected \e[1m" +
                              type + "\e[0m)",
                          18);
        }

        else if (optimizer::do_constant_folding && left->is_const) {
            if (const_folding_fn.count(left->getCstType())) {
                value    = const_folding_fn[left->getCstType()](left->value);
                is_const = true;
            }
        }
    } else {
        parser::error("Unknown operator",
                      tokens,
                      left->getCstType() + "::operator " + op_view + " () is not implemented.",
                      18);
    }
}

String UnaryOperandAST::emitCST() const {
    return op_view + left->emitCST();
}

String UnaryOperandAST::_str() const {
    return "<"s + op_view + str(left.get()) + (is_const ? " [="s + value + "]>" : ">"s);
}

//  AddAST

#define CF_FUN_INT(type1, type2, type, op)                                                         \
    {std::make_tuple(type1, type2),                                                                \
     nlambda(String v1, String v2) {return std::to_string(type(std::stoll(v1) op std::stoll(v2))); \
    }                                                                                              \
    }
#define CF_FUN_FLT(type1, type2, type, op)                                                         \
    {std::make_tuple(type1, type2),                                                                \
     nlambda(String v1, String v2) {return std::to_string(type(std::stold(v1) op std::stold(v2))); \
    }                                                                                              \
    }

AddAST::AddAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::ADD;
    this->op_view          = "+";
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

AddAST::~AddAST() {};

String AddAST::emitLL(int* locc, String inp) const {
    String op  = left->getLLType()[0] == 'i' ? "add" : "fadd contract nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert(String("%") + std::to_string(++(*locc)), r);
    inp        = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

sptr<AST> AddAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mAddAST::parse\e[0m");
    if (tokens.size() < 1) { return nullptr; }
    lexer::TokenStream::Match m =
        tokens.splitStack({lexer::Token::Type::ADD, lexer::Token::Type::SUB}, tokens[0].type == lexer::Token::SUB);
    if (m.found()) {
        DEBUGT(2, "AddAST::parse", &tokens)

        sptr<AST> left = math::parse(m.before(), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected",
                          m.before(),
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m",
                          111);
            return share<AST>(new AST());
        }

        sptr<AST> right = math::parse(m.after(), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected",
                          m.after(),
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m",
                          111);
            return share<AST>(new AST());
        }
        lexer::Token op = tokens[m];
        DEBUG(3, "\top.type: "s + lexer::getTokenName(op.type))
        if (op.type == lexer::Token::Type::ADD) {
            return share<AST>(new AddAST(left, right, tokens));
        }

        else if (op.type == lexer::Token::Type::SUB) {
            return share<AST>(new SubAST(left, right, tokens));
        }
    }
    return nullptr;
}

// SubAST

SubAST::SubAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::SUB;
    this->op_view          = "-";
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

SubAST::~SubAST() {};

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

MulAST::MulAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::MUL;
    this->op_view          = "*";
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

MulAST::~MulAST() {};

String MulAST::emitLL(int* locc, String inp) const {
    String op  = left->getLLType()[0] == 'i' ? "mul" : "fmul contract arcp nsz";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}

sptr<AST> MulAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mMulAST::parse\e[0m");
    if (tokens.size() < 1) { return nullptr; }
    lexer::TokenStream::Match m = tokens.splitStack({lexer::Token::MUL, lexer::Token::DIV, lexer::Token::MOD});
    if (m.found()) {
        DEBUGT(2, "MulAST::parse", &tokens)

        sptr<AST> left = math::parse(m.before(), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected",
                          m.before(),
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m",
                          111);
            return share<AST>(new AST());
        }

        sptr<AST> right = math::parse(m.after(), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected",
                          m.after(),
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m",
                          111);
            return share<AST>(new AST());
        }
        lexer::Token op = tokens[m];
        DEBUG(3, "\top.type: "s + lexer::getTokenName(op.type))
        if (op.type == lexer::Token::Type::MUL) {
            return share<AST>(new MulAST(left, right, tokens));
        }

        else if (op.type == lexer::Token::Type::DIV) {
            return share<AST>(new DivAST(left, right, tokens));
        }

        else if (op.type == lexer::Token::Type::MOD) {
            return share<AST>(new DivAST(left, right, tokens));
        }
    }
    return nullptr;
}

// DivAST

DivAST::DivAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::DIV;
    this->op_view          = "/";
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

DivAST::~DivAST() {};

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

ModAST::ModAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::MOD;
    this->op_view          = "%";
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

ModAST::~ModAST() {};

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

#define CF_FUN_POW(type, type2)                                                                                    \
    {std::make_tuple(type, type),                                                                                  \
     nlambda(String v1,                                                                                            \
             String v2) {return std::to_string(type2(std::pow(float80(std::stoll(v1)), float80(std::stoll(v2))))); \
    }                                                                                                              \
    }
#define CF_FUN_POW_FLT(type, type2)                                                                                \
    {std::make_tuple(type, type),                                                                                  \
     nlambda(String v1,                                                                                            \
             String v2) {return std::to_string(type2(std::pow(float80(std::stold(v1)), float80(std::stold(v2))))); \
    }                                                                                                              \
    }

PowAST::PowAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::POW;
    this->op_view          = "**";
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

PowAST::~PowAST() = default;

String PowAST::emitLL(int*, String) const {
    return "";
}

#define STANDARD_MATH_PARSE(tokentype, type1)                                             \
    if (tokens.size() < 1) return nullptr;                                                \
    lexer::TokenStream::Match m = tokens.splitStack({tokentype});                         \
    if (m.found()) {                                                                      \
        sptr<AST> left = math::parse(m.before(), local, sr, expected_type);               \
        if (left == nullptr) {                                                            \
            parser::error("Expression expected",                                          \
                          m.before(),                                                     \
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", \
                          111);                                                           \
            return share<AST>(new AST());                                                 \
        }                                                                                 \
                                                                                          \
        sptr<AST> right = math::parse(m.after(), local, sr, expected_type);               \
        if (right == nullptr) {                                                           \
            parser::error("Expression expected",                                          \
                          m.after(),                                                      \
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m", \
                          111);                                                           \
            return share<AST>(new AST());                                                 \
        }                                                                                 \
        lexer::Token op = tokens[m];                                                      \
        if (op.type == tokentype) return share<AST>(new type1(left, right, tokens));      \
    }

sptr<AST> PowAST::parse(PARSER_FN_PARAM) {
    DEBUG(2, "PowAST::parse");
    STANDARD_MATH_PARSE(lexer::Token::POW, PowAST);
    return nullptr;
}

// LorAST

LorAST::LorAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::LOR;
    this->op_view          = "||";
    this->const_folding_fn = {
        {std::make_tuple("bool", "bool"),
         nlambda(String v1, String v2) {return (v1 == "true"s || v2 == "true") ? "true"s : "false"s;
}
}
}
;
}

LorAST::~LorAST() {};

String LorAST::emitLL(int* locc, String inp) const {
    String op  = "or";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}

sptr<AST> LorAST::parse(PARSER_FN_PARAM) {
    DEBUG(2, "LorAST::parse");
    STANDARD_MATH_PARSE(lexer::Token::LOR, LorAST);
    return nullptr;
}

// LandAST

LandAST::LandAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::LAND;
    this->op_view          = "&&";
    this->const_folding_fn = {
        {std::make_tuple("bool", "bool"),
         nlambda(String v1, String v2) {return (v1 == "true"s && v2 == "true") ? "true"s : "false"s;
}
}
}
;
}

LandAST::~LandAST() {};

String LandAST::emitLL(int* locc, String inp) const {
    String op  = "and";
    String inc = "{} = "s + op + " " + getLLType() + " {}, {}\n";
    String l   = right->emitLL(locc, inc);
    String r   = left->emitLL(locc, l);
    r          = insert("%"s + std::to_string(++(*locc)), r);
    inp        = rinsert("%"s + std::to_string(*locc), inp);

    return r + inp;
}

sptr<AST> LandAST::parse(PARSER_FN_PARAM) {
    DEBUG(2, "LandAST::parse");
    STANDARD_MATH_PARSE(lexer::Token::LAND, LandAST);
    return nullptr;
}

// OrAST

OrAST::OrAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::OR;
    this->op_view          = "|";
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

sptr<AST> OrAST::parse(PARSER_FN_PARAM) {
    DEBUG(2, "OrAST::parse");
    STANDARD_MATH_PARSE(lexer::Token::OR, OrAST);
    return nullptr;
}

// AndAST

AndAST::AndAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::AND;
    this->op_view          = "&";
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

sptr<AST> AndAST::parse(lexer::TokenStream tokens, int local, symbol::Namespace* sr, String expected_type) {
    DEBUG(2, "AndAST::parse");
    STANDARD_MATH_PARSE(lexer::Token::AND, AddAST);
    return nullptr;
}

// XorAST

XorAST::XorAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::XOR;
    this->op_view          = "^";
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

sptr<AST> XorAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mXorAST::parse\e[0m");
    STANDARD_MATH_PARSE(lexer::Token::XOR, XorAST);
    return nullptr;
}

// EqAST

EqAST::EqAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::EQ;
    this->op_view          = "==";
    this->const_folding_fn = {
        {std::make_tuple("bool", "bool"), nlambda(String v1, String v2) {return (v1 == v2) ? "true"s : "false"s;
}
}
}
;
}

String EqAST::emitLL(int* locc, String inp) const {
    return inp;
}

sptr<AST> EqAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mEqAST::parse\e[0m");
    STANDARD_MATH_PARSE(lexer::Token::EQ, EqAST);
    return nullptr;
}

// NeqAST

NeqAST::NeqAST(sptr<AST> left, sptr<AST> right, lexer::TokenStream tokens) {
    this->left             = left;
    this->right            = right;
    this->tokens           = tokens;
    this->op               = lexer::Token::NEQ;
    this->op_view          = "!=";
    this->const_folding_fn = {
        {std::make_tuple("bool", "bool"), nlambda(String v1, String v2) {return (v1 != v2) ? "true"s : "false"s;
}
}
}
;
}

String NeqAST::emitLL(int* locc, String inp) const {
    return inp;
}

sptr<AST> NeqAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mNeqAST::parse\e[0m");
    STANDARD_MATH_PARSE(lexer::Token::NEQ, EqAST);
    return nullptr;
}

/*
sptr<AST> GtAST::parse(PARSER_FN_PARAM) {
    lexer::Token::Type tokentype = lexer::Token::GREATER;

    if (tokens.size() < 1) { return nullptr; }
    lexer::TokenStream::Match m = tokens.splitStack({tokentype});
    if (m.found()) {
        sptr<AST> left = math::parse(m.before(), local, sr, expected_type);
        if (left == nullptr) {
            parser::error("Expression expected",
                          m.before(),
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m",
                          111);
            return share<AST>(new AST());
        }

        sptr<AST> right = math::parse(m.after(), local, sr, expected_type);
        if (right == nullptr) {
            parser::error("Expression expected",
                          m.after(),
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m",
                          111);
            return share<AST>(new AST());
        }
        lexer::Token op = tokens[m];
        if (op.type == tokentype) { return share<AST>(new GtAST(left, right, tokens)); }
    }
    return nullptr;
}*/

// NotAST

NotAST::NotAST(sptr<AST> inner, lexer::TokenStream tokens) {
    this->left             = inner;
    this->tokens           = tokens;
    this->op               = lexer::Token::NOT;
    this->op_view          = "!";
    this->const_folding_fn = {{"bool", nlambda(String v) {return (v == "true" ? "false"s : "true"s);
}
}
}
;
}

#define UNARY_MATH_PARSE(tokentype, type1, after)                                                        \
    if (tokens.size() == 0) return nullptr;                                                              \
    if (tokens[0].type == tokentype) {                                                                   \
        sptr<AST> of = math::parse(tokens.slice(1, 1, tokens.size()), local, sr);                        \
        if (of == nullptr) {                                                                             \
            parser::error("Expression expected",                                                         \
                          tokens.slice(1, 1, 1),                                                         \
                          "Expected espression of type \e[1m"s + expected_type + "\e[0m after " + after, \
                          111);                                                                          \
            return share<AST>(new AST);                                                                  \
        }                                                                                                \
        return share<AST>(new type1(of, tokens));                                                        \
    }

sptr<AST> NotAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mNotAST::parse\e[0m");
    UNARY_MATH_PARSE(lexer::Token::NOT, NotAST, '!');

    return nullptr;
}

// NegAST

NegAST::NegAST(sptr<AST> inner, lexer::TokenStream tokens) {
    this->left             = inner;
    this->tokens           = tokens;
    this->op               = lexer::Token::NEG;
    this->op_view          = "~";
    this->const_folding_fn = {{"uint8"s, nlambda(String v) {return std::to_string(~(uint8) (std::stoi(v)));
}
}
, {"uint16"s, nlambda(String v) {return std::to_string(~(uint16) (std::stoi(v)));
}
}
, {"uint32"s, nlambda(String v) {return std::to_string(~(uint32) (std::stol(v)));
}
}
, {"uint64"s, nlambda(String v) {return std::to_string(~(uint64) (std::stoll(v)));
}
}
,

    {"int8"s, nlambda(String v) {return std::to_string(~(int8) (std::stoi(v)));
}
}
, {"int16"s, nlambda(String v) {return std::to_string(~(int16) (std::stoi(v)));
}
}
, {"int32"s, nlambda(String v) {return std::to_string(~(int32) (std::stol(v)));
}
}
, {"int64"s, nlambda(String v) {return std::to_string(~(int64) (std::stoll(v)));
}
}
,
}
;
}

sptr<AST> NegAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mNegAST::parse\e[0m");
    UNARY_MATH_PARSE(lexer::Token::NEG, NegAST, '~');

    return nullptr;
}

// AddrOfAST

AddrOfAST::AddrOfAST(sptr<AST> of) {
    this->of = of;
}

AddrOfAST::~AddrOfAST() {};

String AddrOfAST::getLLType() const {
    return of->getLLType() + "*";
}

String AddrOfAST::emitCST() const {
    return String("#") + of->emitCST();
}

String AddrOfAST::emitLL(int*, String) const {
    return "";
}

sptr<AST> math::parse_pt(PARSER_FN_PARAM) {
    if (tokens.size() < 2) { return nullptr; }
    if (tokens[0].type == lexer::Token::Type::OPEN) {
        if (tokens[-1].type == lexer::Token::Type::CLOSE) {
            DEBUG(2, "PT::parse");
            auto a = math::parse(tokens.slice(1, 1, tokens.size() - 1), local + 1, sr, expected_type);
            a->has_pt = true;
            a->setTokens(tokens);
            return a;
        }
    }

    return nullptr;
}

CastAST::CastAST(sptr<AST> from, sptr<AST> type, lexer::TokenStream tokens) {
    this->from   = from;
    this->type   = type;
    this->tokens = tokens;
}

sptr<AST> CastAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mCastAST::parse\e[0m");
    if (tokens.size() < 3) { return nullptr; }
    lexer::TokenStream::Match m = tokens.splitStack({lexer::Token::AS});
    if (m.found()) {
        DEBUG(2, "CastAST::parse");
        if ((uint64) m == tokens.size() - 1) {
            parser::error("Expected type", tokens.getTS(m), "Expected a type after 'as'", 25);
            return share<AST>(new AST);
        }
        if ((uint64) m == 0) {
            parser::error("Expression expected", tokens.getTS(0), "Expected a valid expression", 31);
            return share<AST>(new AST());
        }
        sptr<AST> type = Type::parse(m.after(), local, sr);
        if (type == nullptr) {
            parser::error("Expected type", m.after(), "Expected a type after 'as'", 25);
            return share<AST>(new AST);
        }
        sptr<AST> expr = math::parse(m.before(), local, sr);
        if (expr == nullptr) {
            parser::error("Expression expected", m.before(), "Expected a valid expression", 31);
            return share<AST>(new AST());
        }
        return share<AST>(new CastAST(expr, type, tokens));
    }
    return nullptr;
}

String CastAST::emitCST() const {
    return from->emitCST() + " as " + type->emitCST();
}

void CastAST::forceType(String t) {
    if (getCstType() != t) {
        parser::error("Type mismatch",
                      tokens,
                      String("expected a \e[1m") + t + "\e[0m, got a variable cast returning " + getCstType(),
                      17,
                      "Caused by");
    }
}

String CastAST::emitLL(int* locc, String inp) const {
    String in_type  = from->getCstType();
    String out_type = type->getCstType();

    if (in_type == "char") {
        in_type = "int16"; // chars are unicode
    }
    if (out_type == "char") {
        out_type = "int16"; // chars are unicode
    }
    if (in_type == "bool") { in_type = "int1"; }
    if (out_type == "bool") { out_type = "int1"; }
    String     op = "";
    String     s  = "";
    std::regex i("u?int(1|8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");

    // From int to int
    if (std::regex_match(in_type, i) && std::regex_match(out_type, i)) {
        int bits_in  = std::stoi(in_type.substr(3 + uint(in_type[0] == 'u')));
        int bits_out = std::stoi(out_type.substr(3 + uint(out_type[0] == 'u')));

        if (bits_in > bits_out) {
            op = String("{} = trunc ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        }
        if (bits_in < bits_out) {
            op = String("{} = zext ") + (in_type[0] == 'u' ? String(" ") : String("")) + parser::LLType(in_type) +
                 " {} to " + parser::LLType(out_type) + "\n";
        }
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

        if (bits_in > bits_out) {
            op = String("{} = fptrunc nsz ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        }
        if (bits_in < bits_out) {
            op = String("{} = fpext nsz ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        }
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
        s = insert(String("%") + std::to_string(*locc), s);
        inp =
            rinsert("{ "s + parser::LLType(in_type) + " " + String("%") + std::to_string(*locc) + " , i1 false }", inp);
    }

    return s + inp;
}

sptr<AST> CheckAST::parse(PARSER_FN_PARAM) {
    if (tokens.size() == 0) { return nullptr; }
    if (tokens[-1].type == lexer::Token::QM) {
        DEBUG(2, "CheckAST::parse");
        sptr<AST> of = math::parse(tokens.slice(0, 1, -1), local, sr, expected_type);
        if (of == nullptr) {
            if (tokens.size() == 1) {
                parser::error("Expected expression", tokens, "expected an expression before '?'", 31);
            } else {
                parser::error("Expected expression",
                              tokens.slice(0, 1, -1),
                              "expected a valid expression before '?'",
                              31);
            }
            return nullptr;
        }
        return share<AST>(new CheckAST(of, tokens));
    }
    return nullptr;
}

void CheckAST::forceType(CstType type) {
    if (!(type != of->getCstType() + '?')) {
        parser::error("Type mismatch",
                      tokens,
                      String("expected a \e[1m") + type + "\e[0m, got an optional check returning " + getCstType(),
                      17);
    }
}

String CheckAST::emitLL(int* locc, String inp) const {
    String s  = of->emitLL(locc, "");
    s        += "%"s + std::to_string(++(*locc)) + " = extractvalue " + parser::LLType(of->getCstType()) + " %" +
         std::to_string(*locc) + ", 1\n";
    s += "br i1 %"s + std::to_string(*locc) + ", label %l" + std::to_string(*locc) + ", label %l" +
         std::to_string((*locc) + 1) + "\n";
    s += "l" + std::to_string(*locc) + ":\n";
    s += "l" + std::to_string((*locc) + 1) + ":\n";
    s += "%"s + std::to_string(++(*locc)) + " = extractvalue " + parser::LLType(of->getCstType()) + " %" +
         std::to_string((*locc) - 1) + ", 0\n";
    return s + insert("%" + std::to_string((*locc)++), inp);
}

sptr<AST> NoWrapAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mNoWrapAST::parse\e[0m");
    if (tokens.size() < 3) { return nullptr; }
    if (tokens[0].type == lexer::Token::NOWRAP) {
        DEBUGT(2, "NoWrapAST::parse", &tokens);
        if (tokens[1].type != lexer::Token::OPEN) {
            parser::error("Expected Block open", {tokens[0]}, "Expected a '(' token after 'nowrap'", 0);
            return share<AST>(new AST);
        }
        if (tokens[-1].type != lexer::Token::CLOSE) {
            parser::error("Expected Block close",
                          {tokens[-1]},
                          "Expected a ')' token after '"s + tokens[-1].value + "'",
                          0);
            return share<AST>(new AST);
        }
        sptr<AST> a = math::parse(tokens.slice(2, 1, -1), local + 1, sr);
        if (a == nullptr) {
            parser::error("Expression expected",
                          tokens.slice(2, 1, -1),
                          "Expected a valid expression in nowrap block",
                          0);
            return share<AST>(new AST);
        }
        if (! instanceOf(a, DoubleOperandAST)) { // currently, nowrap can only be used on operators. Unary operands (~ and !) never wrap and only double have to be checked
            return a;
        }
        return share<AST>(new NoWrapAST(a, tokens));
    }
    return nullptr;
}

void NoWrapAST::forceType(CstType type) {
    of->forceType(type);
}



sptr<AST> math::parse(lexer::TokenStream tokens, int local, symbol::Namespace* sr, String expected_type) {
    DEBUGT(2, "math::parse", &tokens);
    return parser::parseOneOf(tokens,
                              {parse_pt, IntLiteralAST::parse, FloatLiteralAST::parse, BoolLiteralAST::parse,
                               CharLiteralAST::parse, StringLiteralAST::parse, NullLiteralAST::parse, VarAccesAST::parse, VarSetAST::parse,

                               NegAST::parse, LandAST::parse, LorAST::parse, XorAST::parse, AddAST::parse, MulAST::parse, PowAST::parse, NotAST::parse, NegAST::parse,
                              AndAST::parse, OrAST::parse,

                               NoWrapAST::parse, CastAST::parse, CheckAST::parse, FuncCallAST::parse},
                              local, sr, expected_type);
}
