#pragma once
#include "ast.hpp"
#include <vector>
#include "../symboltable.hpp"
#include "base_math.hpp"

sptr<AST> parseStatement(PARSER_FN);
extern String parse_name(std::vector<lexer::Token>);

class VarDeclAST : public AST {

    String name = "";
    sptr<AST> type = nullptr;
    symbol::Variable* v    = nullptr;

    protected:
    String _str() const;

    public:
    VarDeclAST(String name, sptr<AST> type, symbol::Variable* v);
    virtual bool isConst() const {return false;} // do constant folding or not
    virtual ~VarDeclAST(){};
    virtual uint64 nodeSize() const {return 1;} // how many nodes to to do
    virtual String emitLL(int*, String) const;
    virtual String emitCST() const {return type->getCstType() + " " + name + ";";}
    virtual String getCstType() const {return name;}
    virtual String getLLType() const {return "";}
    virtual void forceType(String){}

    static sptr<AST> parse(PARSER_FN);
};

class VarInitlAST : public AST {

    String name = "";
    sptr<AST> type = nullptr;
    sptr<AST> expression = nullptr;
    symbol::Variable* v=nullptr;
    bool              as_optional = false;

    protected:
    String _str() const;

    public:
    VarInitlAST(String name, sptr<AST> type, sptr<AST> expr, symbol::Variable* v, std::vector<lexer::Token> tokens);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarInitlAST(){};
    virtual uint64 nodeSize() const {return expression->nodeSize() + 1;} // how many nodes to to do
    virtual String emitLL(int*, String) const;

    virtual String emitCST() const {return type->getCstType() + " " + name + " = " + expression->emitCST() + ";";}
    
    virtual String getCstType() const {return name;}
    virtual String getLLType() const {return "";}
    virtual void forceType(String){}

    static sptr<AST> parse(PARSER_FN);
};

class VarAccesAST : public AST {
    String name = "";
    symbol::Variable* var  = nullptr;

    protected:
    String _str() const;

    public:
    VarAccesAST(String name, symbol::Variable* sr, std::vector<lexer::Token> tokens);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarAccesAST(){}
    virtual uint64 nodeSize() const {return 1;} // how many nodes to to do
    virtual String emitLL(int*, String) const;
    virtual String emitCST() const {return name;}

    virtual String getCstType() const {return var->getCstType();}
    virtual String getLLType() const {return "";}
    virtual void forceType(String type);

    static sptr<AST> parse(PARSER_FN);
};

class VarSetAST : public ExpressionAST {
    String name = "";
    symbol::Variable* var = nullptr;
    sptr<AST> expr = nullptr;
    bool              as_optional = false;

    protected:
    String _str() const;

    public:
    VarSetAST(String name, symbol::Variable* sr, sptr<AST> expr, std::vector<lexer::Token> tokens);
    virtual bool isConst(){return false;} // do constant folding or not
    virtual ~VarSetAST(){};
    virtual uint64 nodeSize() const {return expr->nodeSize() + 1;} // how many nodes to to do
    virtual String emitLL(int*, String) const;
    virtual String emitCST() const {return "("s + name + " = " + expr->emitCST() + ")";}

    virtual String getCstType() const {return var->getCstType();}
    virtual String getLLType() const {return "";}
    virtual void forceType(String type);

    static sptr<AST> parse(PARSER_FN);
};

