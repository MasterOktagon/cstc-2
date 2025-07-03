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
#include "struct.hpp"
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

String SubBlockAST::_str() const {
    String ret = "";
    for (sptr<AST> a : contents) {
        ret += str(a.get());
        ret += "\n";
    }
    return intab(ret);
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
                            VarDeclAST::parse, parseStatement, EnumAST::parse,
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


sptr<AST> IfAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, std::string){
    if (tokens.size() < 1)
        return nullptr;
    if (tokens.at(0).type == lexer::Token::IF) {
        uint32 split = parser::rsplitStack(tokens, {lexer::Token::OPEN}, local);
        if (split == 2) {
            parser::error("Condition expected", parser::subvector(tokens, 0, 1, 2),
                          "A boolean condition was expected before '{'", 0);
            return share<AST>(new AST);
        }
        else if (split >= tokens.size()) {
            parser::error("Block expected", tokens, "Expected a Block opening (opening brace '{')", 0);
            return share<AST>(new AST);
        }
        if (tokens.at(tokens.size()).type != lexer::Token::CLOSE) {
            parser::error("Block end expected", {tokens.at(tokens.size())}, "Expected a Block ending (closing brace '}')", 0);
            return share<AST>(new AST);
        }

        sptr<AST> condition = math::parse(tokens, local + 1, sr);
        if (condition == nullptr) {
            parser::error("Condition expected", parser::subvector(tokens, 1, 1, split),
                          "A boolean condition was expected before '{'", 0);
            return share<AST>(new AST);
        }

        //sptr<SubBlockAST> sb = SubBlockAST::parse(parser::subvector(tokens, split,1,tokens.size()-1), local+1, sr);
    }
    
    return nullptr;
}



