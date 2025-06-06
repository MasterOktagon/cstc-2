#pragma once

//
// NAMESPACE.hpp
//
// layouts the namespace AST
//

#include <string>
#include "ast.hpp"
#include <vector>
#include "../../lexer/token.hpp"
#include "../symboltable.hpp"
#include "flow.hpp"


class NamespaceAST : public AST {

    sptr<SubBlockAST> block = nullptr;
    symbol::Namespace* ns = nullptr;

    public:
    NamespaceAST(sptr<SubBlockAST> a, symbol::Namespace* ns){block = a; this->ns = ns;}
    virtual bool isConst() {return false;} // do constant folding or not
    virtual ~NamespaceAST(){};
    virtual String emitLL(int* locc, String inp) const;
    virtual String emitCST() const;    
    virtual String getCstType() const {return "void";}
    virtual String getLLTtype() const { return ""; }
    
    virtual void forceType(String){}

    /**
     * @brief parse a namespace declaration
     *
     * @return namespace AST or nullptr is not found
     */
    static sptr<AST> parse(PARSER_FN);
};





