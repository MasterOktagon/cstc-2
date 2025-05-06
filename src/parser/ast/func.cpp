

#include "ast.hpp"
#include <sys/types.h>
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "../parser.hpp"
#include <string>
#include "func.hpp"
#include "base_math.hpp"
#include "literal.hpp"
#include "type.hpp"
#include "var.hpp"
#include <iostream>
#include "../errors.hpp"

#define DEBUG

AST* FuncCallAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, std::string expected_type){
    if (tokens.size() < 3) return nullptr;

    std::vector<AST*> params = {};
    int split = parser::rsplitStack(tokens, {lexer::Token::Type::OPEN}, local);

    #ifdef DEBUG
        std::cout << "FuncCallAST::parse: split:\t" << split << std::endl;
    #endif

    if (tokens[tokens.size()-1].type != lexer::Token::Type::CLOSE) return nullptr;
    if (split == 0) return nullptr;
    if ((uint) split >= tokens.size()-1) return nullptr;
    std::string name = parse_name(parser::subvector(tokens, 0,1,split));
    int namesplit = split;
    if (name == "") return new AST;
    auto tokens2 = parser::subvector(tokens, split+1,0,tokens.size()-1);
    while (tokens2.size() > 0 && (uint) split < tokens2.size()){
        split = parser::rsplitStack(tokens2, {lexer::Token::Type::COMMA},local+1);
        AST* a = math::parse(parser::subvector(tokens2, 0,1,split), local, sr);
        if (a == nullptr){
            parser::error("Expected Expression", {tokens2[0], tokens2[split]}, "Expected a valid expression", 31);
            return new AST;
        }
        params.push_back(a);
        tokens2 = parser::subvector(tokens2, split,1,tokens2.size());
    }
    if (tokens2.size() > 0){
        split = tokens2.size();
        AST* a = math::parse(parser::subvector(tokens2, 0,1,split), local, sr);
        if (a == nullptr){
            parser::error("Expected Expression", {tokens2[0], tokens2[split]}, "Expected a valid expression", 31);
            return new AST;
        }
        params.push_back(a);
    }
    std::vector<symbol::Reference*> options = (*sr)[name];
    if (options.size() == 0){
        parser::error("Unknown Function", {tokens[0], tokens[namesplit-1]}, "A Funtion with name of '" + name + "' was not found in this scope", 26);
        return new AST;
    }

    bool matches = false;
    uint32 j = 0;
    for (symbol::Reference* f : options){
        if ((symbol::Function*) f == dynamic_cast<symbol::Function*>(f)){
            symbol::Function* f = (symbol::Function*) f;
            if (f->parameters.size() != params.size()) continue;
            for (uint i=0; i<params.size(); i++){
                if (!parser::typeEq(params[i]->getCstType(), f->parameters[i])) continue;
            }
            matches = true; break;
        }
        j++;
    }
    if (!matches){
        std::string paramlist = "";
        for (int i=0; i<params.size()-1; i++){
            paramlist += params[i]->getCstType() + ",";
        }
        if (params.size() > 0){
            paramlist += params[params.size()-1]->getCstType();
        }
        parser::error("Mismatching operands", tokens, name + "\e[1m(" + paramlist + ")\e[0m is not defined", 26);
        return new AST;
    }

    symbol::Function* p = (symbol::Function*)(*sr)[name][j];
    return new FuncCallAST(name, params, p);
}

std::string FuncCallAST::emit_ll(int* locc, std::string inp){
    std::string s = "invoke " + parser::LLType(fn->getCstType()) + " @" + name + "(";
    if (parser::LLType(fn->getCstType()) != "void") s = "{} = " + s;
    for (AST* p : params){
        s += " ";
        s += parser::LLType(p->getCstType()) + " {}, ";
        s = p->emit_ll(locc, s);
    }
    if (s[s.size()-1] == ' ') s = s.substr(0,s.size()-2);
    s += " ) to label %exc\n";

    if (parser::LLType(fn->getCstType()) != "void"){
        s = insert(std::string("%") + std::to_string(++(*locc)), s);
        inp = rinsert(std::string("%") + std::to_string(*locc), inp);
    }
    return s + inp;
}

std::string FuncCallAST::emit_cst(){
    std::string s = name + "(";
    for (AST* p : params){
        s += p->emit_cst();
    }
    s += ")";
    return s;
}
