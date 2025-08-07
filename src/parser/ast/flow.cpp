#include "flow.hpp"

#include "../../debug/debug.hpp"
#include "../../lexer/lexer.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include "func.hpp"
#include "import.hpp"
#include "namespace.hpp"
#include "struct.hpp"
#include "var.hpp"

#include <memory>
#include <string>
#include <vector>

String SubBlockAST::emitCST() const {
    String ret = "";
    for (sptr<AST> a : contents) {
        ret += a->emitCST();
        if ( instanceOf(a, ExpressionAST)){
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

sptr<AST> SubBlockAST::parse(PARSER_FN_PARAM) {
    if (tokens.size() == 0) return share<AST>(new SubBlockAST);
    std::vector<sptr<AST>> contents;

    while (tokens.size() > 0) {
        lexer::TokenStream::Match split =
            tokens.rsplitStack({lexer::Token::Type::END_CMD, lexer::Token::Type::BLOCK_CLOSE});
        //bool next_is_id = tokens[(uint64)split+1].type == lexer::Token::ID;
        lexer::TokenStream buffer = tokens.slice(0, 1, (int64)split + 1);
        DEBUGT(2, "SubBlockAST::parse", &buffer);
        if (!(buffer.size() == 1 && buffer[0].type == lexer::Token::END_CMD)) {
            if (buffer.size() == 1 && buffer[0].type == lexer::Token::DOTDOTDOT) continue;
            sptr<AST> expr = parser::parseOneOf(
                buffer, {
                            NamespaceAST::parse, VarInitlAST::parse,
                            VarDeclAST::parse, parseStatement, EnumAST::parse, IfAST::parse, FuncDefAST::parse,
                            DeleteAST::parse,
                            ImportAST::parse
                        }, local, sr, "void");

            if (expr == nullptr){
                parser::error("Expected expression", buffer, "Expected a valid expression (Did you forget a ';'?)", 31);
            }
            else {
                if(instanceOf(expr, ExpressionAST) && !sr->ALLOWS_EXPRESSIONS){
                    parser::error("Expression forbidden", expr->getTokens(), "A Block of type "s + sr->getName() + " does not allow Expressions", 0);
                }
                else if (instanceOf(expr, ExpressionAST) && !instanceOf(expr, VarSetAST) && expr->getCstType() != "void"){
                    if (parser::isAtomic(expr->getCstType())){
                        if (!instanceOf(expr, FuncCallAST)){
                            parser::warn("Unused output", expr->getTokens(), "The return type from this expression was discarded.", 0);
                        }
                    } else {
                        parser::error("Type linearity violated", expr->getTokens(), "The (non-atomic) return type from this expression was discarded.", 0);
                    }
                }
                contents.push_back(expr);
            }
        }
        tokens = split.after();
    }

    auto b = share<SubBlockAST>(new SubBlockAST());
    b->contents = contents;
    return b;
}


sptr<AST> IfAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mIfAST::parse\e[0m");
    if (tokens.size() < 3)
        return nullptr;
    if (tokens[0].type == lexer::Token::IF) {
        DEBUG(2, "IfAST::parse");
        lexer::TokenStream::Match m = tokens.rsplitStack({lexer::Token::BLOCK_OPEN});
        if (!m.found()) { // TODO maybe error
            return ERR;
        }
        if (!(tokens[-1].type == lexer::Token::BLOCK_CLOSE)) {
            parser::error("Expected Block close", {tokens[-1]},
                          "Expected a '}' token after '"s + tokens[-1].value + "'", 0);

            return ERR;
        }
        sptr<AST> condition = math::parse(m.before().slice(1, 1, m.before().size()), local, sr);
        if (condition == nullptr) {
            parser::error("Expression expected", m.before().slice(1, 1, tokens.size()), "Expected a valid expression in nowrap block",
                          0);
            return ERR;
        }
        condition->forceType("bool");
        DEBUG(3, "\tcondition: "s + condition->emitCST());
        symbol::Namespace::LinearitySnapshot ls = sr->snapshot();
        sptr<SubBlockAST> block = cast2(SubBlockAST::parse(m.after().slice(0, 1, -1), local + 1, sr), SubBlockAST);

        DEBUG(4, "\tls:  "s + str(&ls));
        symbol::Namespace::LinearitySnapshot ls2 = sr->snapshot();
        DEBUG(4, "\tls2: "s + str(&ls2))
        if (ls != ls2) {
            DEBUG(3, "\ttype linearity violated!");
            parser::error("Type linearity violated", tokens,
                          "The variables in this Block are not in the same state as before", 0);
            ls.traceback(ls2);
        }
        
        return share<AST>(new IfAST(block, condition, tokens));
    }
    
    return nullptr;
}



