#pragma once
#include "ast.hpp"
#include <vector>
#include "../../lexer/token.hpp"
#include "../symboltable.hpp"
//#include "../parser.hpp"

namespace math {
    extern AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    extern AST* parse_pt(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");

}

class ExpressionAST : public AST {
    public:
    ExpressionAST(){};
    virtual ~ExpressionAST(){};
};

class AddAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    AddAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~AddAST();
    String getCstType(){
        return left->getCstType();
    }
    virtual String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    // ALSO PARSES SUBAST
    void forceType(String type);
};

class SubAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    SubAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~SubAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    void forceType(String type);
};

class MulAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    MulAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~MulAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    void forceType(String type);
};

class DivAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    DivAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~DivAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    void forceType(String type);
};

class ModAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    ModAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~ModAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    void forceType(String type);
};

class PowAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    PowAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~PowAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    void forceType(String type);
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
};

class LorAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    LorAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~LorAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    void forceType(String type);
};

class LandAST : public ExpressionAST {
    AST* left;
    AST* right;

    public:
    LandAST(AST* left, AST* right, std::vector<lexer::Token> tokens);
    virtual ~LandAST();
    String getCstType(){
        return left->getCstType();
    }
    String getLLType();

    virtual uint64 nodeSize(){return left->nodeSize() + right->nodeSize() + 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    void forceType(String type);
};

class CastAST : public ExpressionAST {
    AST* from;
    AST* type;

    public:
    CastAST(AST* from, AST* type, std::vector<lexer::Token> tokens);
    virtual ~CastAST(){delete from; delete type;}
    String getCstType(){ return type->getCstType(); }
    String getLLType(){return type->getLLType();}

    virtual uint64 nodeSize(){return 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    void forceType(String type);
};

class AddrOfAST : public ExpressionAST {
    AST* of;

    public:
    AddrOfAST(AST* of);
    virtual ~AddrOfAST();
    String getCstType(){
        return of->getCstType()+"*";
    }
    String getLLType();

    virtual uint64 nodeSize(){return 1;} // how many nodes to to do
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst();
    /*
        Emit C* code
    */
};

