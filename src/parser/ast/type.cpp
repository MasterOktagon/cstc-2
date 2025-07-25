#include "type.hpp"
#include "ast.hpp"
#include <memory>
#include <vector>
#include "../symboltable.hpp"
#include "../parser.hpp"

TypeAST::TypeAST(String name){
    this->name = name;
}

sptr<AST> TypeAST::parse(PARSER_FN_PARAM){
    if (tokens.size() != 1) return nullptr;
    if (tokens[0].type == lexer::Token::Type::ID) {
        return share<AST>(new TypeAST(tokens[0].value));
    }
    return nullptr;
}

sptr<AST> Type::parse(PARSER_FN_PARAM){
        return parser::parseOneOf(tokens, {
            OptionalTypeAST::parse,
            TypeAST::parse
        }, local, sr, expected_type);
}


OptionalTypeAST::OptionalTypeAST(sptr<TypeAST> t){
    this->type = t;
}

sptr<AST> OptionalTypeAST::parse(PARSER_FN_PARAM){
    if (tokens.size() < 2) return nullptr;
    if (tokens[tokens.size()-1].type == lexer::Token::Type::QM) {
        sptr<AST> t = Type::parse(tokens.slice(0,1,tokens.size()-1), local, sr);
        if (t == nullptr){
            return nullptr;
        }
        if (!instanceOf(t, TypeAST)){
            return nullptr;
        }
        return share<AST>(new OptionalTypeAST(std::dynamic_pointer_cast<TypeAST>(t)));
    }
    return nullptr;
}
