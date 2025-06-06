#pragma once

//
// AST.hpp
//
// layouts the AST Node class
//

#include "../../snippets.h"
#include "../../lexer/token.hpp"
#include <vector>

#define PARSER_FN std::vector<lexer::Token>, int local, symbol::Namespace* sr, String expected_type="@unknown" //> used to template all parser functions
#define PARSER_FN_NO_DEFAULT fsignal<sptr<AST>, std::vector<lexer::Token>, int, symbol::Namespace*, String>

/**
 * @class represents an AST node
 */
class AST : public Repr {

    protected:
        /**
         * @brief debug represenstation
         */
        String _str();
        std::vector<lexer::Token> tokens; //> Tokens of this AST Node. these are mostly used for error messages

    public:
        /**
         * @note since this class is thought as a replacement for errorneous ASTs, there is only a default constructor
         */
        AST() = default;
        String value;          //> value, used for several purposes
        bool is_const = false; //> whether this is const and should be tried to be substituted in.

        virtual ~AST(){};

        /**
         * @brief get the LLVM IR type representation of this Nodes return type
         */
        virtual LLType getLLType() const;

        /**
         * @brief get the return type (C*) of this Node, or @unknown
         */
        virtual CstType getCstType() const;

        /**
         * @brief make sure the expected type does match with the provided. may cast errors
         *
         * @param t expected type
         */
        virtual void forceType(String t);

        /**
         * @brief emit LLVM IR [WIP]
         *
         * @param locc local variable counter. should be initionilized with 0 at the start
         * @param s Code generated until now
         */
        virtual String emitLL(int*, String) const;

        /**
         * @brief emit C* code
         */
        virtual String emitCST() const;

        /**
         * @brief get this Nodes work size (in Nodes) for progress reports
         */
        virtual uint64 nodeSize() const {return 0;}
};


///////// Various Helper Functions //////////

/**
 * @brief add a tab at every line beginning
 */
extern String intab(String);

/**
 * @brief insert a value into a String.
 *
 * @param val value to be inserted
 * @param target target String to insert into (first "{}")
 */
extern String insert(String val, String target);

/**
 * @brief insert a value into a String.
 *
 * @param val value to be inserted
 * @param target target String to insert into (last "{}")
 */
extern String rinsert(String val, String target);


