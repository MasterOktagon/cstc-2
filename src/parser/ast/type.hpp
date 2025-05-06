#pragma once
#include "ast.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "../parser.hpp"
#include <string>
#include "var.hpp"

namespace Type {
    extern AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
}

class TypeAST : public AST {

    String name = "";

    public:
    TypeAST(String name);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~TypeAST(){}
    virtual String emit_ll(int locc=0){return "";}
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */
    //virtual llvm::Value* codegen(){return nullptr;}
    /*
        Emit llvm-bitcode to be compiled later
    */

    virtual String emit_cst(){return name;}
    /*
        Emit C* code
    */
    
    virtual String getCstType(){return name;}
    virtual String getLLType(){return parser::LLType(name);}
    virtual void forceType(String type){}
    /*
        Try to enforce a specific type
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
};




