#include <string>
#include "base_math.hpp"
#include "ast.hpp"
#include "func.hpp"
#include "literal.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include <iostream>
#include <regex>
#include "../parser.hpp"
#include "type.hpp"
#include "var.hpp"
#include "../errors.hpp"

#define DEBUG
// AddAST

AddAST::AddAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

AddAST::~AddAST(){
    delete left;
    delete right;
}
String AddAST::getLLType(){
    return left->getLLType();
}

String AddAST::emit_ll(int* locc, String inp){
    
    String op = left->getLLType()[0] == 'i'? "add" : "fadd contract nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String AddAST::emit_cst(){
    return String("(") + left->emit_cst() + " + " + right->emit_cst() + ")";
}

AST* AddAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    auto t = tokens;
    if (tokens.size() < 1) return nullptr;
    lexer::Token first; bool first_is_sub = false;
    if (tokens[0].type == lexer::Token::Type::SUB){
        first = tokens[0]; first_is_sub = true;
        tokens = parser::subvector(tokens, 1,1,tokens.size());
    }
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::ADD, lexer::Token::Type::SUB}, local);
    if (first_is_sub) tokens.insert(tokens.begin(), first);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()-1){
        #ifdef DEBUG
            std::cout << "AddAST::parse:\tfirst_is_sub:\t" << first_is_sub << std::endl;
            std::cout << "AddAST::parse:\tvalue size:\t" << tokens.size() << std::endl;
            std::cout << "AddAST::parse:\tsplit:\t" << split << std::endl;
        #endif
        lexer::Token op = tokens[split+first_is_sub];
        AST* left = math::parse(parser::subvector(tokens, 0,1,split + first_is_sub), local, sr, expected_type);
        if (left == nullptr){
            /*if(op.type == lexer::Token::Type::SUB){
                AST* right = math::parse(parser::subvector(tokens, split + first_is_sub+1,1,tokens.size()), local, sr, expected_type);
                if (right == dynamic_cast<IntLiteralAST*>(right)){
                    if (((IntLiteralAST*) right)->get_value()[0] != '-'){
                        return nullptr;
                    }
                }
            }*/
            parser::error("Expression expected", {tokens[0], tokens[split+first_is_sub-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return new AST();
        }
        AST* right = math::parse(parser::subvector(tokens, split + first_is_sub+1,1,tokens.size()), local, sr, expected_type);
        if (right == nullptr){
            parser::error("Expression expected", {tokens[split+first_is_sub], tokens[tokens.size()-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            delete left;
            return new AST();
        }
        if (op.type == lexer::Token::Type::ADD) return new AddAST(left, right, t);
        else if (op.type == lexer::Token::Type::SUB) return new SubAST(left, right, t);
    }

    return nullptr;
}

void AddAST::forceType(String type){
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool int_required = std::regex_match(type, i);
    bool float_required = std::regex_match(type, f);
    if (std::regex_match(left->getCstType(), i) && int_required){
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), f) && float_required){
        left->forceType(type);
        right->forceType(type);
    }

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::ADD);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator + (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator + (" + right->getCstType() + ") is not implemented.", 18);

}

// SubAST

SubAST::SubAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

SubAST::~SubAST(){
    delete left;
    delete right;
}
String SubAST::getLLType(){
    return left->getLLType();
}

String SubAST::emit_ll(int* locc, String inp){
    
    String op = left->getLLType()[0] == 'i'? "sub" : "fsub contract nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String SubAST::emit_cst(){
    return String("(") + left->emit_cst() + " - " + right->emit_cst() + ")";
}

void SubAST::forceType(String type){
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool int_required = std::regex_match(type, i);
    bool float_required = std::regex_match(type, f);
    if (std::regex_match(left->getCstType(), i) && int_required){
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), i) && float_required){
        left->forceType(type);
        right->forceType(type);
    }

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::SUB);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator - (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator - (" + right->getCstType() + ") is not implemented.", 18);

}

// MulAST

MulAST::MulAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

MulAST::~MulAST(){
    delete left;
    delete right;
}
String MulAST::getLLType(){
    return left->getLLType();
}

String MulAST::emit_ll(int* locc, String inp){
    
    String op = left->getLLType()[0] == 'i'? "mul" : "fmul contract arcp nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String MulAST::emit_cst(){
    return String("(") + left->emit_cst() + " * " + right->emit_cst() + ")";
}

AST* MulAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 1) return nullptr;
    auto t = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::MUL, lexer::Token::Type::DIV, lexer::Token::Type::MOD}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()){
        #ifdef DEBUG
            std::cout << "MulAST::parse:\tsplit:\t" << split << std::endl;
        #endif
        lexer::Token op = tokens[split];
        AST* left = math::parse(parser::subvector(tokens, 0,1,split), local, sr, expected_type);
        if (left == nullptr){
            parser::error("Expression expected", {tokens[0], tokens[split-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return new AST();
        }
        AST* right = math::parse(parser::subvector(tokens, split+1,1,tokens.size()), local, sr, expected_type);
        if (right == nullptr){
            parser::error("Expression expected", {tokens[split], tokens[tokens.size()-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            delete left;
            return new AST();
        }
        if (op.type == lexer::Token::Type::MUL) return new MulAST(left, right, t);
        else if (op.type == lexer::Token::Type::DIV) return new DivAST(left, right, t);
        else if (op.type == lexer::Token::Type::MOD) return new ModAST(left, right, t);
    }

    return nullptr;
}

void MulAST::forceType(String type){
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool int_required = std::regex_match(type, i);
    bool float_required = std::regex_match(type, f);
    if (std::regex_match(left->getCstType(), i) && int_required){
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), i) && float_required){
        left->forceType(type);
        right->forceType(type);
    }

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::MUL);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator * (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator * (" + right->getCstType() + ") is not implemented.", 18);

}


// DivAST

DivAST::DivAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

DivAST::~DivAST(){
    delete left;
    delete right;
}
String DivAST::getLLType(){
    return left->getLLType();
}

String DivAST::emit_ll(int* locc, String inp){

    String op = left->getLLType()[0] == 'i'? left->getCstType()[0] == 'u' ? "udiv" : "sdiv" : "fdiv contract arcp nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String DivAST::emit_cst(){
    return String("(") + left->emit_cst() + " / " + right->emit_cst() + ")";
}

void DivAST::forceType(String type){
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool int_required = std::regex_match(type, i);
    bool float_required = std::regex_match(type, f);
    if (std::regex_match(left->getCstType(), i) && int_required){
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), i) && float_required){
        left->forceType(type);
        right->forceType(type);
    }

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::DIV);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator / (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator / (" + right->getCstType() + ") is not implemented.", 18);

}

// ModAST

ModAST::ModAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

ModAST::~ModAST(){
    delete left;
    delete right;
}
String ModAST::getLLType(){
    return left->getLLType();
}

String ModAST::emit_ll(int* locc, String inp){
    String op = left->getLLType()[0] == 'i'? left->getCstType()[0] == 'u' ? "urem" : "srem" : "frem contract arcp nsz";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String ModAST::emit_cst(){
    return String("(") + left->emit_cst() + " % " + right->emit_cst() + ")";
}

void ModAST::forceType(String type){
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool int_required = std::regex_match(type, i);
    bool float_required = std::regex_match(type, f);
    if (std::regex_match(left->getCstType(), i) && int_required){
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), i) && float_required){
        left->forceType(type);
        right->forceType(type);
    }

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::MOD);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator % (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator % (" + right->getCstType() + ") is not implemented.", 18);

}

// PowAST

PowAST::PowAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

PowAST::~PowAST(){
    delete left;
    delete right;
}
String PowAST::getLLType(){
    return left->getLLType();
}

String PowAST::emit_ll(int* locc, String inp){
    return "";
}

String PowAST::emit_cst(){
    return String("(") + left->emit_cst() + " ** " + right->emit_cst() + ")";
}

void PowAST::forceType(String type){
    std::regex i("u?int(8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");
    bool int_required = std::regex_match(type, i);
    bool float_required = std::regex_match(type, f);
    if (std::regex_match(left->getCstType(), i) && int_required){
        left->forceType(type);
        right->forceType(type);
    }
    if (std::regex_match(left->getCstType(), i) && float_required){
        left->forceType(type);
        right->forceType(type);
    }

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::POW);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator ** (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator ** (" + right->getCstType() + ") is not implemented.", 18);

}

AST* PowAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 1) return nullptr;
    auto t = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::POW}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()){
        #ifdef DEBUG
            std::cout << "PowAST::parse:\tsplit:\t" << split << std::endl;
        #endif
        lexer::Token op = tokens[split];
        AST* left = math::parse(parser::subvector(tokens, 0,1,split), local, sr, expected_type);
        if (left == nullptr){
            parser::error("Expression expected", {tokens[0], tokens[split-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return new AST();
        }
        AST* right = math::parse(parser::subvector(tokens, split+1,1,tokens.size()), local, sr, expected_type);
        if (right == nullptr){
            parser::error("Expression expected", {tokens[split], tokens[tokens.size()-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            delete left;
            return new AST();
        }
        return new PowAST(left, right, t);
    }

    return nullptr;
}

// LorAST

LorAST::LorAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

LorAST::~LorAST(){
    delete left;
    delete right;
}
String LorAST::getLLType(){
    return left->getLLType();
}

String LorAST::emit_ll(int* locc, String inp){
    String op = "or";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String LorAST::emit_cst(){
    return String("(") + left->emit_cst() + " || " + right->emit_cst() + ")";
}

AST* LorAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 1) return nullptr;
    auto t = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LOR}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()){
        #ifdef DEBUG
            std::cout << "LorAST::parse:\tsplit:\t" << split << std::endl;
        #endif
        lexer::Token op = tokens[split];
        AST* left = math::parse(parser::subvector(tokens, 0,1,split), local, sr, expected_type);
        if (left == nullptr){
            parser::error("Expression expected", {tokens[0], tokens[split-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return new AST();
        }
        AST* right = math::parse(parser::subvector(tokens, split+1,1,tokens.size()), local, sr, expected_type);
        if (right == nullptr){
            parser::error("Expression expected", {tokens[split], tokens[tokens.size()-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            delete left;
            return new AST();
        }
        if (op.type == lexer::Token::Type::LOR) return new LorAST(left, right, t);
    }

    return nullptr;
}

void LorAST::forceType(String type){

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::LOR);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator || (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator || (" + right->getCstType() + ") is not implemented.", 18);

}

// LandAST

LandAST::LandAST(AST* left, AST* right, std::vector<lexer::Token> tokens){
    this->left = left;
    this->right = right;
    this->tokens = tokens;
}

LandAST::~LandAST(){
    delete left;
    delete right;
}
String LandAST::getLLType(){
    return left->getLLType();
}

String LandAST::emit_ll(int* locc, String inp){
    String op = "and";
    String inc = String("{} = ") + op + " " + getLLType() + " {}, {}\n";
    String l = right->emit_ll(locc, inc);
    String r = left->emit_ll(locc, l);
    r = insert(String("%") + std::to_string(++(*locc)), r);
    inp = rinsert(String("%") + std::to_string(*locc), inp);

    return r + inp;
}

String LandAST::emit_cst(){
    return String("(") + left->emit_cst() + " && " + right->emit_cst() + ")";
}

AST* LandAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 1) return nullptr;
    auto t = tokens;
    size_t split = parser::splitStack(tokens, {lexer::Token::Type::LAND}, local);
    if (tokens.size() > 2 && split != 0 && split < tokens.size()){
        #ifdef DEBUG
            std::cout << "LandAST::parse:\tsplit:\t" << split << std::endl;
        #endif
        lexer::Token op = tokens[split];
        AST* left = math::parse(parser::subvector(tokens, 0,1,split), local, sr, expected_type);
        if (left == nullptr){
            parser::error("Expression expected", {tokens[0], tokens[split-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            return new AST();
        }
        AST* right = math::parse(parser::subvector(tokens, split+1,1,tokens.size()), local, sr, expected_type);
        if (right == nullptr){
            parser::error("Expression expected", {tokens[split], tokens[tokens.size()-1]}, String("Expected espression of type \e[1m") + expected_type + "\e[0m", 111);
            delete left;
            return new AST();
        }
        if (op.type == lexer::Token::Type::LAND) return new LandAST(left, right, t);
    }

    return nullptr;
}

void LandAST::forceType(String type){

    String ret = parser::hasOp(left->getCstType(), right->getCstType(), lexer::Token::Type::LAND);
    if (ret != ""){
        if (ret != type) parser::error("Mismatiching types", tokens, left->getCstType() + "::operator && (" + right->getCstType() + ") yields " + ret + " (expected \e[1m" + type + "\e[0m)", 18);
    }
    else parser::error("Unknown operator", tokens, left->getCstType() + "::operator && (" + right->getCstType() + ") is not implemented.", 18);

}

// AddrOfAST

AddrOfAST::AddrOfAST(AST* of){
    this->of = of;
}

AddrOfAST::~AddrOfAST(){
    delete of;
}
String AddrOfAST::getLLType(){
    return of->getLLType() + "*";
}

String AddrOfAST::emit_cst(){
    return String("#") + of->emit_cst();
}
String AddrOfAST::emit_ll(int* locc, String){
    return "";
}

AST* math::parse_pt(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 2) return nullptr;
    if (tokens[0].type == lexer::Token::Type::OPEN){
        if (tokens[tokens.size()-1].type == lexer::Token::Type::CLOSE){
            #ifdef DEBUG
                std::cout << "parse_pt" << std::endl;
            #endif
            return math::parse(parser::subvector(tokens, 1,1,tokens.size()-1), local+1, sr, expected_type);
        }
    }

    return nullptr;
}

CastAST::CastAST(AST* from, AST* type, std::vector<lexer::Token> tokens){
    this->from = from;
    this->type = type;
    this->tokens = tokens;
}

AST* CastAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 3) return nullptr;
    int split = parser::rsplitStack(tokens, {lexer::Token::Type::AS}, local);
    #ifdef DEBUG
        std::cout << "CastAST::parse:\tsplit:\t" << split << std::endl;
    #endif
    if (split == (int) tokens.size()-1){
        parser::error("Expected type", {tokens[tokens.size()-1]}, "Expected a type after 'as'", 25);
        return new AST;
    }
    if (split == (int) tokens.size()) return nullptr;
    if (split == 0) {
        parser::error("Expression expected", {tokens[0]}, "Expected a valid expression", 31);
        return new AST();
    }
    auto buf = parser::subvector(tokens, split+1,1,tokens.size());
    AST* type = Type::parse(buf, local, sr);
    if (type == nullptr){
        parser::error("Expected type", buf, "Expected a type after 'as'", 25);
        return new AST;
    }

    buf = parser::subvector(tokens, 0,1,split);
    AST* expr = math::parse(buf, local, sr);
    if (expr == nullptr){
        parser::error("Expression expected", {tokens[0]}, "Expected a valid expression", 31);
        return new AST();
    }

    return new CastAST(expr, type, tokens);
}

String CastAST::emit_cst(){
    return from->emit_cst() + " as " + type->emit_cst();
}

void CastAST::forceType(String t){
    if (getCstType() != t){
        parser::error("Type mismatch", tokens ,String("expected a \e[1m") + t + "\e[0m, got a variable cast returning " + getCstType(), 17, "Caused by");
    }
}

String CastAST::emit_ll(int* locc, String inp){
    String in_type = from->getCstType();
    String out_type = type->getCstType();
    if (in_type == "char") in_type = "int16"; // chars are unicode
    if (out_type == "char") out_type = "int16"; // chars are unicode
    if (in_type == "bool") in_type = "int1"; 
    if (out_type == "bool") out_type = "int1";
    String op = "";
    String s = "";
    std::regex i("u?int(1|8|16|32|64|128)");
    std::regex f("float(16|32|64|128)");

    // From int to int
    if (std::regex_match(in_type, i) && std::regex_match(out_type, i)){
        int bits_in = std::stoi(in_type.substr(3 + uint(in_type[0] == 'u')));
        int bits_out = std::stoi(out_type.substr(3 + uint(out_type[0] == 'u')));

        if (bits_in > bits_out) op = String("{} = trunc ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (bits_in < bits_out) op = String("{} = zext ") + (in_type[0] == 'u' ? String(" ") : String("")) + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (op != ""){
            s = from->emit_ll(locc, op);
            s = insert(String("%") + std::to_string(++(*locc)), s);
            inp = rinsert(String("%") + std::to_string(*locc), inp);
        }
        else {
            inp = from->emit_ll(locc, inp);
        }
    }
    // From float to float
    else if (std::regex_match(in_type, f) && std::regex_match(out_type, f)){
        int bits_in = std::stoi(in_type.substr(5));
        int bits_out = std::stoi(out_type.substr(5));

        if (bits_in > bits_out) op = String("{} = fptrunc nsz ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (bits_in < bits_out) op = String("{} = fpext nsz ") + parser::LLType(in_type) + " {} to " + parser::LLType(out_type) + "\n";
        if (op != ""){
            s = from->emit_ll(locc, op);
            s = insert(String("%") + std::to_string(++(*locc)), s);
            inp = rinsert(String("%") + std::to_string(*locc), inp);
        }
        else {
            inp = from->emit_ll(locc, inp);
        }
    }
    // From int to float
    else if (std::regex_match(in_type, i) && std::regex_match(out_type, f)){
        //int bits_in = std::stoi(in_type.substr(5));
        //int bits_out = std::stoi(out_type.substr(5));

        op = in_type[0] == 'u' ? String("uitofp") : String("sitofp");
        String s = String("{} = ") + op + " " + parser::LLType(in_type) + " {} to " + parser::LLType(out_type);
        s = from->emit_ll(locc, op);
        s = insert(String("%") + std::to_string(++(*locc)), s);
        inp = rinsert(String("%") + std::to_string(*locc), inp);
    }

    // float to int
    else if (std::regex_match(in_type, f) && std::regex_match(out_type, i)){
        //int bits_in = std::stoi(in_type.substr(5));
        //int bits_out = std::stoi(out_type.substr(5));

        op = out_type[0] == 'u' ? String("fptoui") : String("fptosi");
        String s = String("{} = ") + op + " " + parser::LLType(in_type) + " {} to " + parser::LLType(out_type);
        s = from->emit_ll(locc, op);
        s = insert(String("%") + std::to_string(++(*locc)), s);
        inp = rinsert(String("%") + std::to_string(*locc), inp);
    }


    return s + inp;
}

AST* math::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    std::cout << "math::parse" << std::endl;
    return parser::parseOneOf(tokens, {
        IntLiteralAST::parse,
        FloatLiteralAST::parse,
        BoolLiteralAST::parse,
        CharLiteralAST::parse,
        StringLiteralAST::parse,
        VarAccesAST::parse,
        VarSetAST::parse,

        AddAST::parse,
        MulAST::parse,
        PowAST::parse,
        LandAST::parse,
        LorAST::parse,
        
        CastAST::parse,
        FuncCallAST::parse,
        parse_pt}, local, sr, expected_type);
}


