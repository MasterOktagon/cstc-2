#include <string>
#include "flow.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include <iostream>
#include "../parser.hpp"
#include "namespace.hpp"
#include "var.hpp"
#include "../errors.hpp"

std::string SubBlockAST::emit_cst(){
    std::string ret = "";
    for (AST* a : contents) {
        ret += a->emit_cst();
        if ((ExpressionAST*) a == dynamic_cast<ExpressionAST*>(a)){
            ret += ";"; // Expressions don't End on semicolons, therefore add one
        }
        ret += "\n";
    }
    return ret;
}

std::string SubBlockAST::emit_ll(int* locc, std::string inp){
    std::string ret = "";
    for (AST* a : contents){
        ret += " ; " + a->emit_cst() + "\n";
        ret += a->emit_ll(locc, "");
    }

    return ret + inp;
}

AST* SubBlockAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, std::string){
    if (tokens.size() == 0) return new SubBlockAST;
    std::vector<AST*> contents;

    while (tokens.size() > 0){
        int split = parser::rsplitStack(tokens, {lexer::Token::Type::END_CMD, lexer::Token::Type::BLOCK_CLOSE}, local);
        std::vector<lexer::Token> buffer = parser::subvector(tokens, 0,1,split+1);
        AST* expr = parser::parseOneOf(buffer, {
            NamespaceAST::parse,
            VarInitlAST::parse,
            VarDeclAST::parse,
            parseStatement,
        }, local, sr, "void");

        if (expr == nullptr){
            parser::error("Expected expression", {tokens[0], tokens.at(split)}, "Expected a valid expression (Did you forget a ';'?)", 31);
        }
        else {
            contents.push_back(expr);
        }
        tokens = parser::subvector(tokens, split+1,1,tokens.size());
    }
    auto b = new SubBlockAST();
    b->contents = contents;

    return b;
}

