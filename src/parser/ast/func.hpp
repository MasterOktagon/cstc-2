#pragma once

#include <string>
#include "ast.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "base_math.hpp"


class FuncCallAST : public ExpressionAST {
    String name;
    symbol::Function* fn = nullptr;
    std::vector<sptr<AST>> params;

    public:
    FuncCallAST(std::string name, std::vector<sptr<AST>> params, symbol::Function* f){this->name=name; this->params=params; this->fn = f;}
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~FuncCallAST(){};

    virtual String emitLL(int* locc, std::string inp) const;

    virtual String emitCST() const;
    
    virtual CstType getCstType() const {return fn->getCstType();}
    virtual LLType getLLType() const {return "";}
    virtual void forceType(String){}
    
    static sptr<AST> parse(PARSER_FN);
};