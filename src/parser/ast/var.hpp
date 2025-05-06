#pragma once
#include "ast.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "base_math.hpp"
#include <string>

AST* parseStatement(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type="@unknown");
extern String parse_name(std::vector<lexer::Token>);

class VarDeclAST : public AST {

    String name = "";
    AST* type = nullptr;

    public:
    VarDeclAST(String name, AST* type);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarDeclAST(){delete type;}
    virtual uint64 nodeSize(){return 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */
    //virtual llvm::Value* codegen(){return nullptr;}
    /*
        Emit llvm-bitcode to be compiled later
    */

    virtual String emit_cst(){return type->getCstType() + " " + name + ";";}
    /*
        Emit C* code
    */
    
    virtual String getCstType(){return name;}
    virtual String getLLType(){return "";}
    virtual void forceType(String type){}
    /*
        Try to enforce a specific type
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
};

class VarInitlAST : public AST {

    String name = "";
    AST* type = nullptr;
    AST* expression = nullptr;

    public:
    VarInitlAST(String name, AST* type, AST* expr);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarInitlAST(){delete type; delete expression;}
    virtual uint64 nodeSize(){return expression->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */
    //virtual llvm::Value* codegen(){return nullptr;}
    /*
        Emit llvm-bitcode to be compiled later
    */

    virtual String emit_cst(){return type->getCstType() + " " + name + " = " + expression->emit_cst() + ";";}
    /*
        Emit C* code
    */
    
    virtual String getCstType(){return name;}
    virtual String getLLType(){return "";}
    virtual void forceType(String type){}
    /*
        Try to enforce a specific type
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
};

class VarAccesAST : public AST {
    String name = "";
    symbol::Reference* var = nullptr;

    public:
    VarAccesAST(String name, symbol::Reference* sr);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarAccesAST(){}
    virtual uint64 nodeSize(){return 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
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
    
    virtual String getCstType(){return var->getCstType();}
    virtual String getLLType(){return "";}
    virtual void forceType(String type);
    /*
        Try to enforce a specific type
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
};

class VarSetAST : public ExpressionAST {
    String name = "";
    symbol::Reference* var = nullptr;
    AST* expr = nullptr;

    public:
    VarSetAST(String name, symbol::Reference* sr, AST* expr);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarSetAST(){delete expr;}
    virtual uint64 nodeSize(){return expr->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */
    //virtual llvm::Value* codegen(){return nullptr;}
    /*
        Emit llvm-bitcode to be compiled later
    */

    virtual String emit_cst(){return String("(") + name + " = " + expr->emit_cst() + ")";}
    /*
        Emit C* code
    */
    
    virtual String getCstType(){return var->getCstType();}
    virtual String getLLType(){return "";}
    virtual void forceType(String type);
    /*
        Try to enforce a specific type
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
};

