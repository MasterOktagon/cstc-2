#include "ast.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "../parser.hpp"
#include <string>
#include "var.hpp"
#include "base_math.hpp"
#include "literal.hpp"
#include "type.hpp"
#include <iostream>
#include "../errors.hpp"

String parse_name(std::vector<lexer::Token> tokens){
    if (tokens.size() == 0) return "";
    String name = "";
    lexer::Token::Type last = lexer::Token::Type::SUBNS;
    //std::cout << tokens.size() << std::endl;
    
    if (tokens[0].type == lexer::Token::Type::SUBNS){
        parser::error("Expected Symbol", {tokens[0]}, "module name or variable name expected", 30);
        return "";
    }
    for (lexer::Token t : tokens){
        if (last == lexer::Token::Type::SUBNS && t.type == lexer::Token::Type::ID){
            name += t.value;
        }
        else if (last == lexer::Token::Type::ID && t.type == lexer::Token::Type::SUBNS){
            name += "::";
        }
        else return "null";
        last = t.type;
    }
    if (last == lexer::Token::Type::SUBNS){
        parser::error("Expected Symbol", {tokens[tokens.size()-1]}, "module name or variable name expected", 30);
        return "";
    }
    return name; 
}

AST* parseStatement(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() == 0 || tokens[tokens.size()-1].type != lexer::Token::Type::END_CMD) return nullptr;
    return math::parse(parser::subvector(tokens, 0,1,tokens.size()-1), local, sr, expected_type);
}

VarDeclAST::VarDeclAST(String name, AST* type){
    this->name = name;
    this->type = type;
}

AST* VarDeclAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 3) return nullptr;
    if (tokens[tokens.size()-1].type == lexer::Token::Type::END_CMD){
        String name = "";
        if (tokens[tokens.size()-2].type == lexer::Token::Type::ID){
            name = tokens[tokens.size()-2].value;
            AST* type = Type::parse(parser::subvector(tokens, 0,1,tokens.size()-2), local, sr);
            if (type != nullptr){
                if (!parser::isAtomic(type->getCstType())){
                    parser::error("Unknown type", {tokens[0], tokens[tokens.size()-3]}, "A type of this name is unknown in this scope", 19);
                    return new AST;
                }
                if ((*sr)[name].size() > 0){
                    parser::error("Variable already defined", {tokens[tokens.size()-2]}, "A variable of this name is already defined in this scope", 25);
                    parser::note((*sr)[name][0]->tokens, "defined here:", 0);
                    return new AST;
                }
                if (parser::isAtomic(name)){
                    parser::error("Unsupported name", {tokens[tokens.size()-2]}, String("The name ") + name + " refers to a scope or type and cannot be used as a variable name", 25);
                    return new AST;
                }
                if (!parser::is_snake_case(name)){
                    parser::warn("Wrong casing", {tokens[tokens.size()-2]}, "Variable name should be snake_case", 16);
                }

                sr->add(name, new symbol::Variable(name, type->getCstType(), {tokens[tokens.size()-2]}, sr));
                return new VarDeclAST(name, type);
            }
        }
    }
    return nullptr;
}

String VarDeclAST::emit_ll(int*, String){
    return String("%") + name + " = alloca " + type->getLLType() + "\n";
}





VarInitlAST::VarInitlAST(String name, AST* type, AST* expr){
    this->name = name;
    this->type = type;
    this->expression = expr;
}

AST* VarInitlAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() < 5) return nullptr;
    if (tokens[tokens.size()-1].type == lexer::Token::Type::END_CMD){
        String name = "";
        int split = parser::rsplitStack(tokens, {lexer::Token::Type::SET}, local);
        if (tokens[split-1].type == lexer::Token::Type::ID && split > 1){
            name  = tokens[split-1].value;
            AST* type = Type::parse(parser::subvector(tokens, 0,1,split-1), local, sr);
            if (type == nullptr){
                parser::error("Expected type", {tokens[0], tokens[split-1]}, "Expected a type before the identifier", 25);
                return new AST;
            }
            AST* expr = math::parse(parser::subvector(tokens, split+1, 1, tokens.size()-1), local, sr, type->getCstType());
            if (expr == nullptr){
                parser::error("Expected expression", {tokens[split+1], tokens[tokens.size()-1]}, "Expected an expression", 25);
                return new AST;
            }
            if (!parser::isAtomic(type->getCstType())){
                parser::error("Unknown type", {tokens[0], tokens[tokens.size()-3]}, "A type of this name is unknown in this scope", 19);
                return new AST;
            }
            if ((*sr)[name].size() > 0){
                parser::error("Variable already defined", {tokens[tokens.size()-2]}, "A variable of this name is already defined in this scope", 25);
                parser::note((*sr)[name][0]->tokens, "defined here:", 0);
                return new AST;
            }
            if (parser::isAtomic(name)){
                parser::error("Unsupported name", {tokens[tokens.size()-2]}, String("The name ") + name + " refers to a scope or type and cannot be used as a variable name", 25);
                return new AST;
            }
            if (!parser::is_snake_case(name)){
                parser::warn("Wrong casing", {tokens[split-1]}, "Variable name should be snake_case", 16);
            }
            expr->forceType(type->getCstType());

            sr->add(name, new symbol::Variable(name, type->getCstType(), {tokens[split-1]}, sr));
            return new VarInitlAST(name, type, expr);
        }
    }
    return nullptr;
}

String VarInitlAST::emit_ll(int* locc, String){
    String s = String("%") + name + " = alloca " + type->getLLType() + ", align 8\n" +
        "store " + expression->getLLType() + " {}, " + type->getLLType() + "* " + String("%") + name + ", align 8\n" ;
    String l = expression->emit_ll(locc, s);
    return l;
}





VarAccesAST::VarAccesAST(String name, symbol::Reference* sr){
    this->name = name;
    this->var = sr;
}

AST* VarAccesAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() == 0) return nullptr;
    String name = parse_name(tokens);
    if (name == "") return new AST;
    if (name == "null") return nullptr;
    if ((*sr)[name].size() == 0){
        parser::error("Unknown variable", tokens, "A variable of this name was not found in this scope", 20);
        return new AST;
    }

    symbol::Reference* p = (*sr)[name].at(0);
    if (p == dynamic_cast<symbol::Variable*>(p)) ((symbol::Variable*) p)->used = true;
    auto a = new VarAccesAST(name, p); a->tokens = tokens;
    return a;
}

void VarAccesAST::forceType(String type){
    if (var->getCstType() != type){
        parser::error("Type mismatch", tokens ,String("expected a \e[1m") + type + "\e[0m, got a variable of type " + var->getCstType(), 17, "Caused by");
    }
}

String VarAccesAST::emit_ll(int* locc, String inp){
    String s = String("%") + std::to_string(++(*locc)) + " = load " + parser::LLType(var->getCstType()) + ", " + parser::LLType(var->getCstType()) + "* %" + var->getLLLoc() + ", align 8\n";
    inp = rinsert("%" + std::to_string(*locc), inp);
    return s + inp;
}






VarSetAST::VarSetAST(String name, symbol::Reference* sr, AST* expr){
    this->name = name;
    this->var  = sr;
    this->expr = expr;
}

AST* VarSetAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() == 0) return nullptr;
    String name = "";
    lexer::Token::Type last = lexer::Token::Type::SUBNS;
    int split = parser::rsplitStack(tokens, {lexer::Token::Type::SET}, local);
    if (split == 0){
        parser::error("Expected Expression", {tokens[tokens.size()-1]}, "Expected an expression after '='", 31);
        return new AST;
    }
    if (split == tokens.size()) return nullptr;
    auto varname = parser::subvector(tokens, 0,1,split);
    if (varname[0].type == lexer::Token::Type::SUBNS){
        parser::error("Expected Symbol", {tokens[0]}, "module name or variable name expected", 30);
        return new AST;
    }
    for (lexer::Token t : varname){
        if (last == lexer::Token::Type::SUBNS && t.type == lexer::Token::Type::ID){
            name += t.value;
        }
        else if (last == lexer::Token::Type::ID && t.type == lexer::Token::Type::SUBNS){
            name += "::";
        }
        else return nullptr;
        last = t.type;
    }
    if (last == lexer::Token::Type::SUBNS) parser::error("Expected Symbol", {varname[varname.size()-1]}, "module name or variable name expected", 30);
    
    AST* expr = math::parse(parser::subvector(tokens, split+1,1,tokens.size()), local, sr);
    if (expr == nullptr){
        parser::error("Expected Expression", {tokens[tokens.size()-1]}, "Expected a valid expression after '='", 31);
        return new AST;
    }
    
    String type = (*sr)[name].at(0)->getCstType();
    if (type == ""){
        parser::error("Unknown variable", tokens, "A variable of this name was not found in this scope", 20);
        return new AST;
    }
    expr->forceType(type);
    symbol::Reference* p = (*sr)[name].at(0);
    return new VarSetAST(name, p, expr);
}

void VarSetAST::forceType(String type){
    if (var->getCstType() != type){
        if (var == dynamic_cast<symbol::Variable*>(var)) ((symbol::Variable*) var)->used = true;
        parser::error("Type mismatch", tokens ,String("expected a \e[1m") + type + "\e[0m, got a variable of type " + var->getCstType(), 17, "Caused by");
    }
}

String VarSetAST::emit_ll(int* locc, String inp){
    String s = String("store ") + parser::LLType(var->getCstType()) + " {}, " + parser::LLType(var->getCstType()) + "* %" + name + "\n";
    String l = expr->emit_ll(locc, s);
    if (expr != dynamic_cast<LiteralAST*>(expr)) inp = rinsert(String("%") + std::to_string(*locc), inp);
    else inp = expr->emit_ll(locc, inp);

    return l + inp;
}
