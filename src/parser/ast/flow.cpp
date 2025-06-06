#include <memory>
#include <string>
#include "flow.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include <vector>
#include "../../lexer/lexer.hpp"
#include "../symboltable.hpp"
#include <iostream>
#include "../parser.hpp"
#include "import.hpp"
#include "namespace.hpp"
#include "var.hpp"
#include "../errors.hpp"

String SubBlockAST::emitCST() const {
    String ret = "";
    for (sptr<AST> a : contents) {
        ret += a->emitCST();
        if (instanceOf(a, ExpressionAST)){
            ret += ";"; // Expressions don't End on semicolons, therefore add one
        }
        ret += "\n";
    }
    return ret;
}

String SubBlockAST::emitLL(int* locc, String inp) const {
    String ret = "";
    for (sptr<AST> a : contents){
        //ret += " ; " + a->emitCST() + "\n";
        ret += a->emitLL(locc, "");
    }

    return ret + inp;
}

sptr<AST> SubBlockAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, std::string){
    if (tokens.size() == 0) return share<AST>(new SubBlockAST);
    std::vector<sptr<AST>> contents;

    while (tokens.size() > 0){
        int split = parser::rsplitStack(tokens, {lexer::Token::Type::END_CMD, lexer::Token::Type::BLOCK_CLOSE}, local);
        std::vector<lexer::Token> buffer =
            parser::subvector(tokens, 0, 1, split + 1);
        if (!(buffer.size() == 1 && buffer.at(0).type == lexer::Token::END_CMD)){
            sptr<AST> expr = parser::parseOneOf(
                buffer, {
                            NamespaceAST::parse, VarInitlAST::parse,
                            VarDeclAST::parse, parseStatement,
                            ImportAST::parse
                        }, local, sr, "void");

            if (expr == nullptr){
                parser::error("Expected expression", {tokens[0], tokens.at(split)}, "Expected a valid expression (Did you forget a ';'?)", 31);
            }
            else {
                contents.push_back(expr);
            }
        }
        tokens = parser::subvector(tokens, split+1,1,tokens.size());
    }
    auto b = share<SubBlockAST>(new SubBlockAST());
    b->contents = contents;

    return b;
}

