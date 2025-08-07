#pragma once

//
// LITERAL.hpp
//
// layouts literal parsing
//

#include "../symboltable.hpp"
#include "ast.hpp"

#include <string>
#include <vector>

extern AST* parse_neg(std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type = "@unknown");

class LiteralAST : public AST {
    public:
        virtual ~LiteralAST() {};
        virtual String getValue() const abstract;

        virtual uint64 nodeSize() const final { return 1; }
};

class IntLiteralAST : public LiteralAST {
        int  bits    = 32;   //> Integer Bit size
        bool tsigned = true; //> whether this integer is signed

    protected:
        String _str() const { return "<Int: "s + value + " | " + std::to_string(bits) + ">"; }

    public:
        IntLiteralAST(int bits, String value, bool tsigned, lexer::TokenStream tokens);

        virtual ~IntLiteralAST() {}

        // fwd declarations @see @class AST

        CstType getCstType() const { return (tsigned ? "int"s : "uint"s) + std::to_string(bits); }

        LLType getLLType() const { return "i"s + std::to_string(bits); }

        String getValue() const { return value; }

        virtual String emitLL(int*, String) const;
        virtual String emitCST() const;

        virtual void forceType(String type);

        /**
         * @brief parse an int literal
         *
         * @return Int literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class BoolLiteralAST : public LiteralAST {
    protected:
        String _str() const { return "<Bool: "s + value + ">"; }

    public:
        BoolLiteralAST(String value, lexer::TokenStream tokens);

        virtual ~BoolLiteralAST() {}

        // fwd declarations @see @class AST

        CstType getCstType() const { return "bool"; }

        LLType getLLType() const { return "i1"; }

        String getValue() const { return value; }

        virtual String emitLL(int*, String) const;
        virtual String emitCST() const;

        virtual void forceType(String type);

        /**
         * @brief parse a bool literal
         *
         * @return bool literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class FloatLiteralAST : public LiteralAST {
        int bits = 32; //> Float size (name)

    protected:
        String _str() const { return "<Float: "s + value + " | " + std::to_string(bits) + ">"; }

    public:
        FloatLiteralAST(int bits, String value, lexer::TokenStream tokens);

        virtual ~FloatLiteralAST() {}

        CstType getCstType() const { return "float"s + std::to_string(bits); }

        LLType getLLType() const;

        String getValue() const { return value; }

        virtual String emitLL(int*, String) const;
        virtual String emitCST() const;
        virtual void   forceType(String type);

        /**
         * @brief parse a float literal
         *
         * @return float literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class CharLiteralAST : public LiteralAST {
    protected:
        String _str() const { return "<Char: '"s + value + "'>"; }

    public:
        CharLiteralAST(String value, lexer::TokenStream tokens);

        virtual ~CharLiteralAST() {}

        CstType getCstType() const { return "char"; }

        LLType getLLType() const { return "i16"; };

        String         getValue() const;
        virtual String emitLL(int*, String) const;

        virtual String emitCST() const { return value; };

        virtual void forceType(String type);

        /**
         * @brief parse a char literal
         *
         * @return char literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class StringLiteralAST : public LiteralAST {
    protected:
        String _str() const { return "<String: \""s + value + "\">"; }

    public:
        StringLiteralAST(String value, lexer::TokenStream tokens);

        virtual ~StringLiteralAST() {}

        CstType getCstType() const { return "String"; }

        LLType getLLType() const { return "%std..lang..class.String"; };

        String getValue() const;

        virtual String emitLL(int*, String) const; // TODO: think of a possible struct for string

        virtual String emitCST() const { return value; };

        virtual void forceType(String type);

        /**
         * @brief parse a string literal
         *
         * @return string literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class NullLiteralAST : public LiteralAST {
    protected:
        String _str() const { return "<NULL>"; }

        CstType type = "";

    public:
        NullLiteralAST(lexer::TokenStream tokens){this->tokens = tokens;}

        virtual ~NullLiteralAST() {}

        CstType getCstType() const { return type; }

        LLType getLLType() const { return ""; };

        String getValue() const { return "null"; };

        // virtual String emitLL(int*, String) const;
        virtual String emitCST() const { return "null"; };

        virtual void forceType(String type);

        /**
         * @brief parse a null literal
         *
         * @return null literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class EmptyLiteralAST : public LiteralAST {
    protected:
        String _str() const { return "< [] >"; }

        CstType type = "@unknown[]";
        String const_len = "0";

    public:
        EmptyLiteralAST(lexer::TokenStream tokens){this->tokens = tokens; is_const = true;}

        virtual ~EmptyLiteralAST() {}

        CstType getCstType() const { return type; }

        LLType getLLType() const { return ""; };

        String getValue() const { return "[]"; };

        // virtual String emitLL(int*, String) const;
        virtual String emitCST() const { return "[]"; };

        virtual void forceType(String type);

        /**
         * @brief parse a null literal
         *
         * @return null literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};

class ArrayLiteralAST : public LiteralAST {
    protected:
        String _str() const { return "<Array ("s + type + ") [" + "] >"; }

        CstType type = "@unknown[]";
        std::vector<sptr<AST>> contents = {};

    public:
        String const_len = "0";
        ArrayLiteralAST(lexer::TokenStream tokens, std::vector<sptr<AST>> contents){this->tokens = tokens; this->contents=contents;}

        virtual ~ArrayLiteralAST() {}

        CstType getCstType() const { return type; }

        LLType getLLType() const { return ""; };

        String getValue() const { return "[]"; };

        // virtual String emitLL(int*, String) const;
        virtual String emitCST() const { return "[]"; };

        virtual void forceType(String type);

        /**
         * @brief parse a null literal
         *
         * @return null literal AST or nullptr is not found
         */
        static sptr<AST> parse(PARSER_FN);
};