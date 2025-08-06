#pragma once

#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include "flow.hpp"
#include "type.hpp"

#include <string>
#include <vector>

class FuncCallAST : public ExpressionAST {
        String                 name;
        symbol::Function*      fn = nullptr;
        std::vector<sptr<AST>> params;

    public:
        FuncCallAST(std::string name, std::vector<sptr<AST>> params, symbol::Function* f) {
            this->name   = name;
            this->params = params;
            this->fn     = f;
        }

        virtual bool isConst() { return false; } // do constant folding or not

        virtual ~FuncCallAST() {};

        virtual String emitLL(int* locc, std::string inp) const;

        virtual String emitCST() const;

        virtual CstType getCstType() const { return fn->getCstType(); }

        virtual LLType getLLType() const { return ""; }

        virtual void forceType(String) {}

        static sptr<AST> parse(PARSER_FN);
};

class FuncDefAST : public AST {
        String                 name;
        symbol::Function*      fn       = nullptr;
        sptr<SubBlockAST>      contents = nullptr;
        std::map<String, std::pair<std::vector<lexer::Token>, sptr<AST>>> params;
        sptr<TypeAST>          return_type = nullptr;

    public:
        FuncDefAST(std::string            name,
                   sptr<TypeAST>          return_type,
                   std::map<String, std::pair<std::vector<lexer::Token>, sptr<AST>>> params,
                   symbol::Function*      f,
                   sptr<SubBlockAST>      block) {
            this->contents    = block;
            this->name        = name;
            this->params      = params;
            this->fn          = f;
            this->return_type = return_type;
        }

        virtual bool isConst() { return false; } // do constant folding or not

        virtual ~FuncDefAST() {};

        virtual String emitLL(int* locc, std::string inp) const { return inp; };

        virtual String emitCST() const {
            String r = return_type->emitCST() + " " + name + " (";
            bool has_param = false;
            for (auto p : params) {
                r += p.second.second->emitCST() + " " + p.first + ",";
                has_param = true;
            }
            if (has_param) r += "\b";
            r += "){\n" + intab(contents->emitCST()) + "\n}";
            return r;
        }

        virtual CstType getCstType() const { return fn->getCstType(); }

        virtual LLType getLLType() const { return ""; }

        virtual void forceType(String) {}

        static sptr<AST> parse(PARSER_FN);
};
