#include "var.hpp"
#include "../../lexer/lexer.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include "literal.hpp"
#include "type.hpp"
#include <iostream>
#include <string>
#include <vector>

String parse_name(std::vector<lexer::Token> tokens) {
    if (tokens.size() == 0)
        return "";
    String             name = "";
    lexer::Token::Type last = lexer::Token::Type::SUBNS;

    if (tokens[0].type == lexer::Token::Type::SUBNS) {
        parser::error("Expected Symbol", {tokens[0]}, "module name or variable name expected", 30);
        return "";
    }
    for (lexer::Token t : tokens) {
        if (last == lexer::Token::Type::SUBNS && t.type == lexer::Token::Type::ID) {
            name += t.value;
        } else if (last == lexer::Token::Type::ID && t.type == lexer::Token::Type::SUBNS) {
            name += "::";
        } else
            return "null";
        last = t.type;
    }
    if (last == lexer::Token::Type::SUBNS) {
        parser::error("Expected Symbol", {tokens[tokens.size() - 1]}, "module name or variable name expected", 30);
        return "";
    }
    return name;
}

sptr<AST> parseStatement(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() == 0 || tokens[tokens.size() - 1].type != lexer::Token::Type::END_CMD)
        return nullptr;
    return math::parse(parser::subvector(tokens, 0, 1, tokens.size() - 1), local, sr, expected_type);
}

VarDeclAST::VarDeclAST(String name, sptr<AST> type, symbol::Variable* v) {
    this->name = name;
    this->type = type;
    this->v    = v;
}

sptr<AST> VarDeclAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String) {
    if (tokens.size() < 3)
        return nullptr;
    if (tokens[tokens.size() - 1].type == lexer::Token::Type::END_CMD) {
        auto             tokens2 = tokens;
        String           name    = "";
        parser::Modifier m       = parser::getModifier(tokens);
        if (tokens[tokens.size() - 2].type == lexer::Token::Type::ID) {
            name           = tokens[tokens.size() - 2].value;
            sptr<AST> type = Type::parse(parser::subvector(tokens, 0, 1, tokens.size() - 2), local, sr);
            if (type != nullptr) {
                // if (!parser::isAtomic(type->getCstType())){
                //     parser::error("Unknown type", {tokens[0], tokens[tokens.size()-3]}, "A type of this name is
                //     unknown in this scope", 19); return new AST;
                // }
                if ((*sr)[name].size() > 0) {
                    parser::error("Variable already defined", {tokens[tokens.size() - 2]},
                                  "A variable of this name is already defined in this scope", 25);
                    parser::note((*sr)[name][0]->tokens, "defined here:", 0);
                    return sptr<AST>(new AST);
                }
                if (parser::isAtomic(name)) {
                    parser::error(
                        "Unsupported name", {tokens[tokens.size() - 2]},
                        "The name "s + name + " refers to a scope or type and cannot be used as a variable name", 25);
                    return sptr<AST>(new AST);
                }
                if (!parser::is_snake_case(name)) {
                    parser::warn("Wrong casing", {tokens[tokens.size() - 2]}, "Variable name should be snake_case", 16);
                }
                if (m & parser::Modifier::CONST) {
                    parser::error("const declaration without initizialiation", tokens2,
                                  "A variable can only be const if an initizialisation is given", 25);
                    return sptr<AST>(new AST);
                }

                symbol::Variable* v     = new symbol::Variable(name, type->getCstType(), tokens2, sr);
                v->isConst = m & parser::Modifier::CONST;
                sr->add(name, v);
                if (parser::isAtomic(type->getCstType())){
                    v->isFree = true;
                }
                return sptr<AST>(new VarDeclAST(name, type, v));
            }
        }
    }
    return nullptr;
}

String VarDeclAST::emitLL(int*, String) const { return v->getLLLoc() + " = alloca " + type->getLLType() + "\n"; }

VarInitlAST::VarInitlAST(String name, sptr<AST> type, sptr<AST> expr, symbol::Variable* v,
                         std::vector<lexer::Token> tokens) {
    this->name       = name;
    this->type       = type;
    this->expression = expr;
    this->v          = v;
    this->tokens     = tokens;
}

sptr<AST> VarInitlAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String) {
    if (tokens.size() < 5)
        return nullptr;
    if (tokens[tokens.size() - 1].type == lexer::Token::Type::END_CMD) {
        String           name    = "";
        auto             tokens2 = tokens;
        parser::Modifier m       = parser::getModifier(tokens);
        int              split   = parser::rsplitStack(tokens, {lexer::Token::Type::SET}, local);
        if (tokens[split - 1].type == lexer::Token::Type::ID && split > 1) {
            name           = tokens[split - 1].value;
            sptr<AST> type = Type::parse(parser::subvector(tokens, 0, 1, split - 1), local, sr);
            if (type == nullptr) {
                // parser::error("Expected type", {tokens[0], tokens[split-1]}, "Expected a type before the identifier",
                // 25);
                return nullptr;
            }
            sptr<AST> expr =
                math::parse(parser::subvector(tokens, split + 1, 1, tokens.size() - 1), local, sr, type->getCstType());
            if (expr == nullptr) {
                parser::error("Expected expression", {tokens[split + 1], tokens[tokens.size() - 1]},
                              "Expected an expression", 25);
                return share<AST>(new AST);
            }
            // if (!parser::isAtomic(type->getCstType()) ){
            //     parser::error("Unknown type", {tokens[0], tokens[tokens.size()-3]}, "A type of this name is unknown
            //     in this scope", 19); return new AST;
            // }
            if ((*sr)[name].size() > 0) {
                parser::error("Variable already defined", {tokens[tokens.size() - 2]},
                              "A variable of this name is already defined in this scope", 25);
                parser::note((*sr)[name][0]->tokens, "defined here:", 0);
                return share<AST>(new AST);
            }
            if (parser::isAtomic(name)) {
                parser::error("Unsupported name", {tokens[tokens.size() - 2]},
                              String("The name ") + name +
                                  " refers to a scope or type and cannot be used as a variable name",
                              25);
                return share<AST>(new AST);
            }
            if (!parser::is_snake_case(name) && !(m & parser::Modifier::CONST)) {
                parser::warn("Wrong casing", {tokens[split - 1]}, "Variable name should be snake_case", 16);
            }
            if (!parser::IS_UPPER_CASE(name) && m & parser::Modifier::CONST) {
                parser::warn("Wrong casing", {tokens[split - 1]}, "Variable name should be UPPER_CASE", 16);
            }
            expr->forceType(type->getCstType());

            auto v     = new symbol::Variable(name, type->getCstType(), tokens2, sr);
            v->isConst = m & parser::Modifier::CONST;
            sr->add(name, v);
            if (parser::isAtomic(type->getCstType())){
                v->isFree = true;
            }
            v->used = symbol::Variable::PROVIDED;
            
            return share<AST>(new VarInitlAST(name, type, expr, v, tokens));
        }
    }
    return nullptr;
}

String VarInitlAST::emitLL(int* locc, String) const {
    String s = v->getLLLoc() + " = alloca " + type->getLLType() + ", align 8\n" + "store " + expression->getLLType() +
               " {}, " + type->getLLType() + "* " + v->getLLLoc() + ", align 8\n";
    String l = expression->emitLL(locc, s);
    return l;
}

VarAccesAST::VarAccesAST(String name, symbol::Variable* sr, std::vector<lexer::Token> tokens) {
    this->name   = name;
    this->var    = sr;
    this->tokens = tokens;
}

sptr<AST> VarAccesAST::parse(std::vector<lexer::Token> tokens, int, symbol::Namespace* sr, String) {
    if (tokens.size() == 0)
        return nullptr;
    String name = parse_name(tokens);
    if (name == "")
        return share<AST>(new AST);
    if (name == "null")
        return nullptr;
    if ((*sr)[name].size() == 0) {
        parser::error("Unknown variable", tokens, "A variable of this name was not found in this scope", 20);
        return share<AST>(new AST);
    }

    symbol::Reference* p = (*sr)[name].at(0);
    if (p == dynamic_cast<symbol::Variable*>(p)){
        symbol::Variable::Status& u = ((symbol::Variable*) p)->used;
        if (u == symbol::Variable::UNINITIALIZED){
            parser::error("Variable uninitilialized", tokens, "Variable '\e[1m"s + name + "\e[0m' is uninitilialized at this point.\nMake sure the variable holds a value to resolve.", 0);
            parser::note(p->tokens, "declared here", 0);
            return share<AST>(new AST);
        }
        else if (u == symbol::Variable::CONSUMED && !((symbol::Variable*) p)->isFree) {
            parser::error("Type linearity violated", tokens, "Variable '\e[1m"s + name + "\e[0m' is consumed at this point.\nMake sure the variable holds a value to resolve.", 0, "");
            parser::note(p->last, "last consume here", 0);
            return share<AST>(new AST);
        }
        u = symbol::Variable::CONSUMED;
        p->last = tokens;
    }
    return share<AST>(new VarAccesAST(name, (symbol::Variable*) p, tokens));
}

void VarAccesAST::forceType(String type) {
    if (var->getCstType() != type) {
        parser::error("Type mismatch", tokens,
                      String("expected a \e[1m") + type + "\e[0m, got a variable of type " + var->getCstType(), 17,
                      "Caused by");
    }
}

String VarAccesAST::emitLL(int* locc, String inp) const {
    String s = String("%") + std::to_string(++(*locc)) + " = load " + parser::LLType(var->getCstType()) + ", " +
               parser::LLType(var->getCstType()) + "* " + var->getLLLoc() + ", align 8\n";
    inp = rinsert("%" + std::to_string(*locc), inp);
    return s + inp;
}

VarSetAST::VarSetAST(String name, symbol::Variable* sr, sptr<AST> expr, std::vector<lexer::Token> tokens) {
    this->name   = name;
    this->var    = sr;
    this->expr   = expr;
    this->tokens = tokens;
}

sptr<AST> VarSetAST::parse(std::vector<lexer::Token> tokens, int local, symbol::Namespace* sr, String) {
    if (tokens.size() == 0)
        return nullptr;
    String name  = "";
    uint64 split = parser::rsplitStack(tokens, {lexer::Token::Type::SET}, local);
    if (split == 0) {
        parser::error("Expected Expression", {tokens[tokens.size() - 1]}, "Expected an expression after '='", 31);
        return share<AST>(new AST);
    }
    if (split == tokens.size())
        return nullptr;
    auto varname = parser::subvector(tokens, 0, 1, split);
    name         = parse_name(varname);
    if (name == "") {
        parser::error("Expected Symbol", varname, "module name or variable name expected", 30);
        return nullptr;
    }

    sptr<AST> expr = math::parse(parser::subvector(tokens, split + 1, 1, tokens.size()), local, sr);
    if (expr == nullptr) {
        parser::error("Expected Expression", {tokens[tokens.size() - 1]}, "Expected a valid expression after '='", 31);
        return share<AST>(new AST);
    }

    if ((*sr)[name].size() == 0) {
        parser::error("Unknown variable", tokens, "A variable of this name was not found in this scope", 20);
        return share<AST>(new AST);
    }

    expr->forceType((*sr)[name][0]->getCstType());
    symbol::Reference* p = (*sr)[name].at(0);
    if (p == dynamic_cast<symbol::Variable*>(p) && ((symbol::Variable*)p)->isConst) {
        parser::error("trying to set constant", tokens, "You are trying to set a variable which has a constant value.",
                      17);
        parser::note(p->tokens, "defined here:", 0);
        return share<AST>(new AST);
    }
    if (p == dynamic_cast<symbol::Variable*>(p)){
        symbol::Variable::Status& u = ((symbol::Variable*) p)->used;
        if (u == symbol::Variable::PROVIDED) {
            auto warn_error = parser::error;
            if (((symbol::Variable*) p)->isFree){
                warn_error = parser::warn;
            }

            warn_error("Type linearity violated", tokens, "Variable '\e[1m"s + name + "\e[0m' was never consumed.\nMake sure the variable is consumed before it is provided.", 0, "");
            parser::note(p->last, "last provided here", 0);
            return share<AST>(new AST);
        }
        u = symbol::Variable::PROVIDED;
        p->last = tokens;
    }
    return share<AST>(new VarSetAST(name, (symbol::Variable*) p, expr, tokens));
}

void VarSetAST::forceType(String type) {
    if (var->getCstType() != type) {
        parser::error("Type mismatch", tokens,
                      String("expected a \e[1m") + type + "\e[0m, got a variable of type " + var->getCstType(), 17,
                      "Caused by");
    }
}

String VarSetAST::emitLL(int* locc, String inp) const {
    String s = String("store ") + parser::LLType(var->getCstType()) + " {}, " + (as_optional ? " {"s : ""s) +
               parser::LLType(var->getCstType()) + "* " + var->getLLLoc() + (as_optional ? ", i1 true }"s : ""s) +
               ", align 8\n";
    String l = expr->emitLL(locc, s);
    if (instanceOf(expr, LiteralAST))
        inp = rinsert(String("%") + std::to_string(*locc), inp);
    else
        inp = expr->emitLL(locc, inp);

    return l + inp;
}
