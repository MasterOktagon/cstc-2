#include "type.hpp"
#include "ast.hpp"
#include <memory>
#include <vector>
#include "../symboltable.hpp"
#include "../parser.hpp"

TypeAST::TypeAST(String name){
    this->name = name;
}

sptr<AST> TypeAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace*, String){
    if (tokens.size() != 1) return nullptr;
    if (tokens[0].type == lexer::Token::Type::ID) {
        return share<AST>(new TypeAST(tokens[0].value));
    }
    return nullptr;
}

sptr<AST> Type::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type){
        return parser::parseOneOf(tokens, {
            OptionalTypeAST::parse,
            TypeAST::parse
        }, local, sr, expected_type);
}


OptionalTypeAST::OptionalTypeAST(sptr<TypeAST> t){
    this->type = t;
}

sptr<AST> OptionalTypeAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String){
    if (tokens.size() < 2) return nullptr;
    if (tokens.at(tokens.size()-1).type == lexer::Token::Type::QM) {
        sptr<AST> t = Type::parse(parser::subvector(tokens, 0,1,tokens.size()-1), local, sr);
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
