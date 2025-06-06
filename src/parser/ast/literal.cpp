
//
// LITERAL.cpp
//
// implements parsing literals
//

#include "literal.hpp"
#include "../../lexer/token.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include <cstdint>
#include <regex>
#include <string>

bool StringIntBiggerThan(String a, String b) {
    if (a.size() > b.size())
        return true;
    if (b.size() > a.size())
        return false;
    for (uint32 i = 0; i < a.size(); i++) {
        if (a[i] > b[i])
            return true;
        if (b[i] > a[i])
            return false;
    }
    return false;
}

IntLiteralAST::IntLiteralAST(int bits, String value, bool tsigned, std::vector<lexer::Token> tokens) {
    this->bits    = bits;
    this->value   = value;
    this->tsigned = tsigned;
    this->tokens  = tokens;

    // if constant is too big for (u)int32 upgrade to uint64
    if (bits == 32) {
        String v = value;
        if (tsigned) {
            v = v.substr(1);
            if (StringIntBiggerThan(v, "2147483648")) {
                bits = 64;
            }
        } else if (StringIntBiggerThan(v, "4294967295")) {
            bits = 64;
        }
    }
    is_const = true;
}

String IntLiteralAST::emitCST() const { return value; }

String IntLiteralAST::emitLL(int*, String inp) const {
    String value = getValue();
    if (value.size() > 1 && value[1] == 'x')
        value = "u"s + value;
    return rinsert(value, inp);
}

sptr<AST> IntLiteralAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace*, String) {
    if (tokens.size() < 1 || tokens.size() > 2)
        return nullptr;
    if (tokens[0].type == lexer::Token::Type::INT) {
        return share<AST>(new IntLiteralAST(32, tokens[0].value, false, tokens));
    } else if (tokens[0].type == lexer::Token::Type::HEX) {
        return share<AST>(new IntLiteralAST(32, tokens[0].value, false, tokens));
    } else if ((tokens[0].type == lexer::Token::Type::SUB || tokens[0].type == lexer::Token::Type::NEC) &&
               tokens.size() == 2 && tokens[1].type == lexer::Token::Type::INT) {
        return share<AST>(new IntLiteralAST(32, "-"s + tokens[1].value, true, tokens));
    }

    // TODO parse binary integers
    return nullptr;
}

void IntLiteralAST::forceType(String type) {
    std::regex r("u?int(8|16|32|64|128)");
    bool       expected_int = std::regex_match(type, r);
    if (expected_int) {
        bool sig  = type[0] != 'u';
        int  bits = std::stoi(type.substr(3 + (!sig), type.size()));

        if (value[0] == '-' && !sig)
            parser::error("Sign mismatch", {tokens[0], tokens[1]},
                          "Found a signed value (expected \e[1m"s + type + "\e[0m)", 45);
        tsigned    = sig;
        this->bits = bits;

        // TODO fix this
        //if (StringIntBiggerThan(value.substr(value[0] == '-'),
        //                        std::to_string((1 << (bits - tsigned)) - (value[0] != '-')))) {
        //    parser::warn("Integer too big", tokens,
        //                 "trying to fit a number too big into "s + type + ". This will lead to information loss.", 17);
        //}
    } else
        parser::error("Type mismatch", tokens, "expected a \e[1m"s + type + "\e[0m, found int", 17, "Caused by");
}

BoolLiteralAST::BoolLiteralAST(bool value, std::vector<lexer::Token> tokens) {
    this->value  = value;
    this->tokens = tokens;
    is_const     = true;
}

String BoolLiteralAST::emitLL(int*, String inp) const { return rinsert(getValue(), inp); }

String    BoolLiteralAST::emitCST() const { return value ? "true" : "false"; }
sptr<AST> BoolLiteralAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace*, String) {
    if (tokens.size() == 1) {
        if (tokens[0].value == "true" || tokens[0].value == "false") {
            return share<AST>(new BoolLiteralAST(tokens[0].value == "true", tokens));
        }
    }
    return nullptr;
}
void BoolLiteralAST::forceType(String type) {
    if (type != "bool")
        parser::error("Type mismatch", tokens, "expected a \e[1m"s + type + "\e[0m, found bool", 17,
                      "Caused by");
}

FloatLiteralAST::FloatLiteralAST(int bits, String value, std::vector<lexer::Token> tokens) {
    this->bits   = bits;
    this->value  = value;
    this->tokens = tokens;
    is_const     = true;
}

String FloatLiteralAST::emitLL(int*, String inp) const { return rinsert(getValue(), inp); }

String FloatLiteralAST::emitCST() const { return value; }

LLType FloatLiteralAST::getLLType() const {
    if (bits < 32)
        return "half";
    else if (bits < 64)
        return "float";
    else if (bits < 128)
        return "double";
    else
        return "fp128";
}

sptr<AST> FloatLiteralAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace*, String) {
    if (tokens.size() < 1)
        return nullptr;
    bool sig = false;
    auto t   = tokens;
    if (tokens[0].type == lexer::Token::Type::SUB || tokens[0].type == lexer::Token::Type::NEC) {
        sig    = true;
        tokens = parser::subvector(tokens, 1, 1, tokens.size());
    }
    if (tokens.size() < 2)
        return nullptr;
    if (tokens.size() > 3)
        return nullptr;
    if (tokens[0].type == lexer::Token::Type::ACCESS && tokens[1].type == lexer::Token::Type::INT) {
        return share<AST>(new FloatLiteralAST(32, (sig ? String("-0.") : String("0.")) + tokens[1].value + "e00", t));
    } else if (tokens[0].type == lexer::Token::Type::INT && tokens[1].type == lexer::Token::Type::ACCESS) {
        String val = (sig ? String("-") : String("")) + tokens[0].value + ".";
        if (tokens.size() == 3) {
            if (tokens[2].type == lexer::Token::Type::INT) {
                val += tokens[2].value;
            } else
                return nullptr;
        }
        val += "0e00";
        return share<AST>(new FloatLiteralAST(32, val, t));
    }
    return nullptr;
}

void FloatLiteralAST::forceType(String type) {
    std::regex r("float(16|32|64|128)");
    bool       expected_float = std::regex_match(type, r);
    if (expected_float) {
        int bits   = std::stoi(type.substr(5, type.size()));
        this->bits = bits;
    } else
        parser::error("Type mismatch", tokens,
                      String("expected a \e[1m") + type + "\e[0m, found float" + std::to_string(bits), 17, "Caused by");
}

CharLiteralAST::CharLiteralAST(String value, std::vector<lexer::Token> tokens) {
    this->value  = value;
    this->tokens = tokens;
    is_const     = true;
}

String CharLiteralAST::emitLL(int*, String inp) const { return rinsert(getValue(), inp); }

sptr<AST> CharLiteralAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace*, String) {
    if (tokens.size() != 1)
        return nullptr;
    if (tokens[0].type == lexer::Token::Type::CHAR) {
        if (tokens[0].value.size() == 2) {
            parser::error("Empty char", tokens,
                          "This char value is empty. This is not supported. Did you mean '\\u0000' ?", 578);
            return share<AST>(new AST());
        }
        std::regex r("'\\\\u[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]'");
        std::regex r2("'\\\\(n|a|r|t|f|v|\\\\|'|\"|)'");
        if (std::regex_match(tokens[0].value, r) || std::regex_match(tokens[0].value, r2) ||
            tokens[0].value.size() == 3) {
            // std::cout<<"skdskdl"<<std::endl;
            return share<AST>(new CharLiteralAST(tokens[0].value, tokens));
        }
        parser::error("Invalid char", tokens,
                      "This char value is not supported. Chars are meant to hold only one character. Did you mean to "
                      "use \"Double quotes\" ?",
                      579);
        return share<AST>(new AST());
    }
    return nullptr;
}

String CharLiteralAST::getValue() const {
    if (this->value == "'\\n'")
        return "u0x000A";
    if (this->value == "'\\t'")
        return "u0x0009";
    if (this->value == "'\\v'")
        return "u0x000B";
    if (this->value == "'\\f'")
        return "u0x000C";
    if (this->value == "'\\r'")
        return "u0x000D";
    if (this->value == "'\\a'")
        return "u0x0007";
    if (this->value == "'\\\"'")
        return "u0x0022";
    if (this->value == "'\\\\'")
        return "u0x005C";
    if (this->value == "'\\\''")
        return "u0x0027";

    if (this->value[1] == '\\' && this->value.size() == 8) {
        return String("u0x") + this->value.substr(3, 2) + this->value.substr(5, 2);
    }

    return std::to_string((uint16_t)this->value[1]);
}

void CharLiteralAST::forceType(String type) {
    if (type != "char") {
        parser::error("Type mismatch", tokens, String("expected a \e[1m") + type + "\e[0m, found char", 17,
                      "Caused by");
    }
}

StringLiteralAST::StringLiteralAST(String value, std::vector<lexer::Token> tokens) {
    this->value  = value;
    this->tokens = tokens;
    is_const     = true;
}

sptr<AST> StringLiteralAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace*, String) {
    if (tokens.size() != 1)
        return nullptr;
    if (tokens[0].type == lexer::Token::Type::STRING) {
        return share<AST>(new StringLiteralAST(tokens[0].value, tokens));
    }
    return nullptr;
}

String StringLiteralAST::emitLL(int*, String inp) const { return rinsert(getValue(), inp); }

String StringLiteralAST::getValue() const {
    /*if (this->value == "'\\n'") return "\"\\00\\0A\"";
    if (this->value == "'\\t'") return "\"\\00\\09\"";
    if (this->value == "'\\v'") return "\"\\00\\0B\"";
    if (this->value == "'\\f'") return "\"\\00\\0C\"";
    if (this->value == "'\\r'") return "\"\\00\\0D\"";
    if (this->value == "'\\a'") return "\"\\00\\07\"";
    if (this->value == "'\\\"'") return "\"\\00\\22\"";
    if (this->value == "'\\\\'") return "\"\\00\\5C\"";
    if (this->value == "'\\\''") return "\"\\00\\27\"";

    if (this->value[1] == '\\' && this->value.size() == 8){
        return String("\"\\") + this->value.substr(3, 2) + "\\" + this->value.substr(5, 2) + "\"";
    }

    return String("\"") + this->value[1] + "\"";*/
    return "";
}

void StringLiteralAST::forceType(String type) {
    if (type != "String") {
        parser::error("Type mismatch", tokens, String("expected a \e[1m") + type + "\e[0m, found String", 17,
                      "Caused by");
    }
}
