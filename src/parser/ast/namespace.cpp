#include "namespace.hpp"
#include <memory>
#include <string>
#include "flow.hpp"
#include "ast.hpp"
#include <vector>
#include "../../lexer/token.hpp"
#include "../symboltable.hpp"
#include <iostream>
#include "../parser.hpp"
#include "base_math.hpp"
#include "../errors.hpp"

String NamespaceAST::emitCST() const {
    String ret = "";
    for (sptr<AST> a : block->contents) {
        if (a != nullptr){
            ret += a->emitCST();
            if (instanceOf(a, ExpressionAST)){
                ret += ";"; // Expressions don't End on semicolons, therefore add one
            }
            ret += "\n";
        }
    }
    return "namespace "s + ns->getRawLoc() + " {\n" + intab(ret) + "\n}";
}

String NamespaceAST::_str() const {
    String ret = "";
    for (sptr<AST> a : block->contents) {
        if (a != nullptr){
            ret += str(a.get());
            ret += "\n";
        }
    }
    return "<namespace "s + ns->getRawLoc() + " \n" + intab(ret) + "\n>";
}

String NamespaceAST::emitLL(int* i, String s) const {
    return block->emitLL(i, s);
}

sptr<AST> NamespaceAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String){
    if (tokens.size() < 2) return nullptr;
    if (tokens.at(0).type == lexer::Token::Type::NAMESPACE){
        if (tokens.at(1).type == lexer::Token::Type::ID){
            if (!sr->ALLOWS_SUBCLASSES){
                parser::error("Namespace not allowed", tokens, "A Block of type Namespace was not allowed in a Block of type "s + sr->getName(), 60);
                return share<AST>(new AST);
            }
            if ((*sr)[tokens.at(1).value].size() > 0){
                parser::error("Name already known", tokens, "An id with the name of "s + tokens.at(1).value + "was already used", 60);
                parser::note((*sr)[tokens.at(1).value][0]->tokens, "defined here:", 0);
                return share<AST>(new AST);
            }
            if (tokens.size() < 4 || tokens.at(2).type != lexer::Token::Type::BLOCK_OPEN){
                parser::error("Expected Block open", {tokens[1]}, "A '{' was expected after this token", 51);
                return share<AST>(new AST);
            }
            if (tokens.at(tokens.size()-1).type != lexer::Token::Type::BLOCK_CLOSE){
                parser::error("Expected Block close", {tokens[tokens.size()-1]}, "A '}' was expected", 52);
                return share<AST>(new AST);
            }

            symbol::Namespace* ns = new symbol::Namespace(tokens.at(1).value);
            ns->parent = sr;
            sr->add(tokens.at(1).value, ns);
            sptr<AST> a = parser::parseOneOf(parser::subvector(tokens, 3,1,tokens.size()-1), {SubBlockAST::parse}, local+1, ns, "void");

            if (a == nullptr) return share<AST>(new AST);

            return sptr<AST>(new NamespaceAST(std::dynamic_pointer_cast<SubBlockAST>(a), ns));
        }
        else {
            parser::error("Identifier expected", {tokens[0]}, "Expected an identifier after 'namespace'", 50);
        }
    }
    return nullptr;
}

