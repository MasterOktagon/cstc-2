#include "type.hpp"
#include "ast.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include "../parser.hpp"
#include <string>
#include "var.hpp"

TypeAST::TypeAST(String name){
    this->name = name;
}

AST* TypeAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
    if (tokens.size() != 1) return nullptr;
    if (tokens[0].type == lexer::Token::Type::ID) {
        return new TypeAST(tokens[0].value);
    }
    return nullptr;
}

AST* Type::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
        return parser::parseOneOf(tokens, {
        TypeAST::parse
        }, local, sr, expected_type);
}