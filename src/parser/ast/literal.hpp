#pragma once

//
// LITERAL.hpp
//
// layouts literal parsing
//

#include <string>
#include "../symboltable.hpp"
#include "ast.hpp"

extern AST* parse_neg(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");

class LiteralAST : public AST {

    public:
    virtual ~LiteralAST(){}
    virtual String getValue(){return "";}
    bool isConst () {return true;}
    virtual uint64 nodeSize(){return 1;}
};

class IntLiteralAST : public LiteralAST {

    int bits = 32;           // Integer Bit size
    bool tsigned = true;

    protected:
    String _str(){
        return "<Int: "s + value + " | " + std::to_string(bits) + ">";
    }

    public:

    String value = "0"; // Integer value
    IntLiteralAST(int bits, String value, bool tsigned=true, std::vector<lexer::Token> tokens={});
    virtual ~IntLiteralAST(){}
    String getCstType(){return (tsigned ? "int"s : "uint"s) + std::to_string(bits);}
    String getLLType(){return "i"s + std::to_string(bits);}
    String getValue(){return value;}
    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    //virtual llvm::Value* codegen();
    /*
        Emit llvm-bitcode to be compiled later
    */

    String emit_cst();
    /*
        Emit C* code
    */
    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    virtual void forceType(String type);
};

class BoolLiteralAST : public LiteralAST {

    bool value = false; // boolean value

    protected:
    String _str(){
        return "<Bool: "s + (value ? "true"s : "false"s) + ">";
    }

    public:

    BoolLiteralAST(bool value, std::vector<lexer::Token> tokens);
    virtual ~BoolLiteralAST(){}
    String getCstType(){return "bool";}
    String getLLType(){return "i1";}
    String getValue(){return std::to_string(value);}
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
    virtual void forceType(String type);
};

class FloatLiteralAST : public LiteralAST {

    String value = "0";    // Float value
    int bits = 32; // Float size (name)

    protected:
    String _str(){
        return "<Float: "s + value + " | " + std::to_string(bits) + ">";
    }

    public:

    FloatLiteralAST(int bits, String value, std::vector<lexer::Token> tokens={});
    virtual ~FloatLiteralAST(){}
    String getCstType(){return "float"s + std::to_string(bits);}
    String getLLType();
    String getValue(){return value;}
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
    virtual void forceType(String type);
};

class CharLiteralAST : public LiteralAST {

    String value = "a";    // Float value

    protected:
    String _str(){
        return "<Char: '"s + value + "'>";
    }

    public:

    CharLiteralAST(String value, std::vector<lexer::Token> tokens);
    virtual ~CharLiteralAST(){}
    String getCstType(){return "char";}
    String getLLType(){return "i16";};
    String getValue();

    virtual String emit_ll(int*, String);
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst(){return value;};
    /*
        Emit C* code
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    virtual void forceType(String type);
};

class StringLiteralAST : public LiteralAST {

    String value = "";

    protected:
    String _str(){
        return "<String: \""s + value + "\">";
    }

    public:

    StringLiteralAST(String value, std::vector<lexer::Token> tokens);
    virtual ~StringLiteralAST(){}
    String getCstType(){return "String";}
    String getLLType(){return "%class.String";};
    String getValue();

    virtual String emit_ll(int*, String); // TODO: think of a possible struct for string
    /*
        Emit llvm IR code in human-readable form

        [param locc] local variable name counter
    */

    String emit_cst(){return value;};
    /*
        Emit C* code
    */

    static AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown");
    virtual void forceType(String type);
};

