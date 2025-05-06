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

    SubBlockAST* block = nullptr;
    symbol::Namespace* ns = nullptr;

    public:
    NamespaceAST(SubBlockAST* a, symbol::Namespace* ns){block = a; this->ns = ns;}
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~NamespaceAST(){}
    virtual std::string emit_ll(int* locc, std::string inp);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */
    //virtual llvm::Value* codegen(){return nullptr;}
    /*
        Emit llvm-bitcode to be compiled later
    */

    virtual std::string emit_cst();
    /*
        Emit C* code
    */
    
    virtual std::string getCstType(){return "void";}
    virtual std::string getLLTtype(){return "";}
    virtual void forceType(std::string){}
    /*
        Try to enforce a specific type
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, std::string expected_type="@unknown");
};





