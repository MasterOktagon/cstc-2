
#pragma once

//
// PARSER.hpp
//
// convienience functions for the parser
//

#include <vector>
#include "../lexer/token.hpp"
#include "ast/ast.hpp"
#include "symboltable.hpp"

template <typename T, typename ... K>
using fsignal = T (*) (K ... );

namespace parser {
    extern int splitStack(std::vector<lexer::Token> tokens, std::initializer_list<lexer::Token::Type>, int local, std::initializer_list<lexer::Token::Type> = {});
    extern int rsplitStack(std::vector<lexer::Token> tokens, std::initializer_list<lexer::Token::Type>, int local, std::initializer_list<lexer::Token::Type> = {});
    extern AST* parseOneOf(std::vector<lexer::Token> tokens, std::vector<fsignal<AST*, std::vector<lexer::Token>, int, symbol::Namespace*, String>> functions, int local, symbol::Namespace* sr, String expected_type);

    template <typename T>
    std::vector<T> subvector(std::vector<T> v, int start, int, int stop){
        auto s = v.begin() + start; auto end = v.begin() + stop;
        return std::vector<T>(s, end);
    }

    extern bool typeEq(String a, String b);
    extern String hasOp(String type1, String type2, lexer::Token::Type op);
    extern bool isAtomic(String type);

    extern bool is_snake_case(String text);
    extern bool isPascalCase(String text);
    extern bool IsCamelCase(String text);
    extern bool IS_UPPER_CASE(String text);

    extern LLType LLType(CstType);
}

