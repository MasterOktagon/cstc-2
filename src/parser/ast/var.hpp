#pragma once
#include "../symboltable.hpp"
#include "ast.hpp"
#include "base_math.hpp"

#include <vector>

sptr<AST>     parseStatement(PARSER_FN);
extern String parse_name(lexer::TokenStream);

class VarDeclAST : public AST {
        String            name = "";
        sptr<AST>         type = nullptr;
        symbol::Variable* v    = nullptr;

    protected:
        String _str() const;

    public:
        VarDeclAST(String name, sptr<AST> type, symbol::Variable* v);

        virtual bool isConst() const { return false; } // do constant folding or not

        virtual ~VarDeclAST() {};

        virtual uint64 nodeSize() const { return 1; } // how many nodes to to do

        virtual String emitLL(int*, String) const;

        virtual String emitCST() const {
            return (v->isMutable ? "mut "s : ""s) + type->getCstType() + " " + name + ";";
        }

        virtual String getCstType() const { return name; }

        virtual String getLLType() const { return ""; }

        virtual void forceType(String) {}

        static sptr<AST> parse(PARSER_FN);
};

class VarInitlAST : public AST {
        String            name        = "";
        sptr<AST>         type        = nullptr;
        sptr<AST>         expression  = nullptr;
        symbol::Variable* v           = nullptr;
        bool              as_optional = false;

    protected:
        String _str() const;

    public:
        VarInitlAST(String name, sptr<AST> type, sptr<AST> expr, symbol::Variable* v, lexer::TokenStream tokens);

        virtual bool isConst() { return false; } // do constant folding or not

        virtual ~VarInitlAST() {};

        virtual uint64 nodeSize() const { return expression->nodeSize() + 1; } // how many nodes to to do

        virtual String emitLL(int*, String) const;

        virtual String emitCST() const {
            return (v->isMutable ? "mut "s : ""s) + (v->isConst ? "const "s : ""s) + type->getCstType() + " " + name +
                   " = " + expression->emitCST() + ";";
        }

        virtual String getCstType() const { return name; }

        virtual String getLLType() const { return ""; }

        virtual void forceType(String) {}

        static sptr<AST> parse(PARSER_FN);
};

class VarAccesAST : public AST {
        String            name = "";

    protected:
        String _str() const;

    public:
        symbol::Variable* var  = nullptr;
        VarAccesAST(String name, symbol::Variable* sr, lexer::TokenStream tokens);

        virtual bool isConst() { return false; } // do constant folding or not

        virtual ~VarAccesAST() {}

        virtual uint64 nodeSize() const { return 1; } // how many nodes to to do

        virtual String emitLL(int*, String) const;

        virtual String emitCST() const { return name; }

        virtual String getCstType() const { return var->getCstType(); }

        virtual String getLLType() const { return ""; }

        virtual void forceType(String type);

        static sptr<AST> parse(PARSER_FN);
};

class VarSetAST : public ExpressionAST {
        String            name        = "";
        symbol::Variable* var         = nullptr;
        sptr<AST>         expr        = nullptr;
        bool              as_optional = false;

    protected:
        String _str() const;

    public:
        VarSetAST(String name, symbol::Variable* sr, sptr<AST> expr, lexer::TokenStream tokens);

        virtual bool isConst() { return false; } // do constant folding or not

        virtual ~VarSetAST() {};

        virtual uint64 nodeSize() const { return expr->nodeSize() + 1; } // how many nodes to to do

        virtual String emitLL(int*, String) const;

        virtual String emitCST() const { return PUT_PT(name + " = " + expr->emitCST(), this->has_pt); }

        virtual String getCstType() const { return var->getCstType(); }

        virtual String getLLType() const { return ""; }

        virtual void forceType(String type);

        static sptr<AST> parse(PARSER_FN);
};

class DeleteAST : public AST {
        std::vector<symbol::Variable*> vars = {};

    protected:
        String _str() const {
            String s = "<DELETE ";
            for (symbol::Variable* v : vars) {
                s += "<"s + v->getLoc() + ">";
            }
            return s + ">";
        };

    public:
        DeleteAST(std::vector<symbol::Variable*> vars, lexer::TokenStream tokens) {
            this->vars = vars;
            this->tokens = tokens;
        }
        virtual ~DeleteAST() {};

        virtual bool isConst() { return false; }

        virtual uint64 nodeSize() const { return vars.size(); } // how many nodes to to do

        virtual String emitCST() const {
            String s = "delete ";
            for (symbol::Variable* var : vars) { s += var->getRawLoc() + ",";}
            s += "\b;";
            return s;
        }

        virtual String getCstType() const { return "void"; }

        virtual String emitLL(int*, String inp) const { return inp; };

        virtual void forceType(String type){}

        static sptr<AST> parse(PARSER_FN);
};
