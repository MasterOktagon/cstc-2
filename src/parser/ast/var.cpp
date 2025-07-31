#include "var.hpp"
//#include "../../lexer/lexer.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "../../build/optimizer_flags.hpp"
#include "base_math.hpp"
#include "literal.hpp"
#include "type.hpp"
#include "../../debug/debug.hpp"
#include <string>
#include <vector>

String parse_name(lexer::TokenStream tokens) {
    if (tokens.size() == 0)
        return "";
    String             name = "";
    lexer::Token::Type last = lexer::Token::Type::SUBNS;

    if (tokens[0].type == lexer::Token::Type::SUBNS) {
        parser::error("Expected Symbol", {tokens[0]}, "module name or variable name expected", 30);
        return "null";
    }
    for (lexer::Token t : tokens.tokens) {
        if (last == lexer::Token::Type::SUBNS && t.type == lexer::Token::Type::ID) {
            name += t.value;
        } else if (last == lexer::Token::Type::ID && t.type == lexer::Token::Type::SUBNS) {
            name += "::";
        } else
            return "";
        last = t.type;
    }
    if (last == lexer::Token::Type::SUBNS) {
        parser::error("Expected Symbol", {tokens[tokens.size() - 1]}, "module name or variable name expected", 30);
        return "null";
    }
    return name;
}

sptr<AST> parseStatement(lexer::TokenStream tokens, int local, symbol::Namespace* sr, String expected_type) {
    if (tokens.size() == 0 || tokens[tokens.size() - 1].type != lexer::Token::Type::END_CMD)
        return nullptr;
    return math::parse(tokens.slice(0, 1, tokens.size() - 1), local, sr, expected_type);
}

VarDeclAST::VarDeclAST(String name, sptr<AST> type, symbol::Variable* v) {
    this->name = name;
    this->type = type;
    this->v    = v;
}

String VarDeclAST::_str() const {
    return "<DECLARE "s + name + " : ?" + " MUT>";
}

sptr<AST> VarDeclAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mVarDeclAST::parse\e[0m");
    if (tokens.size() < 3)
        return nullptr;
    if (tokens[tokens.size() - 1].type == lexer::Token::Type::END_CMD) {
        auto             tokens2 = tokens;
        String           name    = "";
        parser::Modifier m       = parser::getModifier(tokens);
        if (tokens[tokens.size() - 2].type == lexer::Token::Type::ID) {
            name           = tokens[tokens.size() - 2].value;
            sptr<AST> type = Type::parse(tokens.slice(0, 1, tokens.size() - 2), local, sr);
            if (type != nullptr) {
                // if (!parser::isAtomic(type->getCstType())){
                //     parser::error("Unknown type", {tokens[0], tokens[tokens.size()-3]}, "A type of this name is
                //     unknown in this scope", 19); return new AST;
                // }
                if ((*sr)[name].size() > 0) {
                    parser::error("Variable already defined", {tokens[tokens.size() - 2]},
                                  "A variable of this name is already defined in this scope", 25);
                    parser::note((*sr)[name][0]->tokens, "defined here:", 0);
                    return ERR;
                }
                if (parser::isAtomic(name)) {
                    parser::error(
                        "Unsupported name", {tokens[tokens.size() - 2]},
                        "The name "s + name + " refers to a scope or type and cannot be used as a variable name", 25);
                    return ERR;
                }
                if (!parser::is_snake_case(name)) {
                    parser::warn("Wrong casing", {tokens[tokens.size() - 2]}, "Variable name should be snake_case", 16);
                }
                if (m & parser::Modifier::CONST) {
                    parser::error("const declaration without initialization", tokens2,
                                  "A variable can only be const if an initialization is given", 25);
                    parser::note(tokens2, "remove the 'const' keyword to resolve this easily", 0);
                    return ERR;
                }
                else if (!(m & parser::Modifier::MUTABLE)) {
                    parser::error("immutable declaration without initialization", tokens2,
                                  "A variable can only be immutable if an initialization is given", 25);
                    parser::noteInsert("Make this variable mutable if required", tokens2[0], "mut ", 0, true);
                    return ERR;
                }
                if (!sr->ALLOWS_NON_STATIC && !(parser::Modifier::STATIC | parser::Modifier::CONST)) {
                    parser::error("only static variables allowed", tokens2,
                                  "only static variables are allowed in a Block of type "s + sr->getName(), 25);
                                  
                }

                symbol::Variable* v     = new symbol::Variable(name, type->getCstType(), tokens2.tokens, sr);
                v->isConst              = m & parser::Modifier::CONST;
                v->isMutable            = m & parser::Modifier::MUTABLE;
                v->isStatic             = m & parser::Modifier::STATIC;
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
                         lexer::TokenStream tokens) {
    this->name       = name;
    this->type       = type;
    this->expression = expr;
    this->v          = v;
    this->tokens     = tokens;
}
String VarInitlAST::_str() const {
    return "<DECLARE "s + name + " : " + type->getCstType() + " = " + str(expression.get()) + (v->isConst? " CONST"s : ""s) + (v->isMutable? " MUT"s : ""s) + (v->isStatic? " STATIC"s : ""s) +  ">";
}

sptr<AST> VarInitlAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mVarInitlAST::parse\e[0m");
    if (tokens.size() < 5)
        return nullptr;
    if (tokens[tokens.size() - 1].type == lexer::Token::Type::END_CMD) {
        String           name    = "";
        auto             tokens2 = tokens;
        parser::Modifier m       = parser::getModifier(tokens);
        int64            split   = tokens.rsplitStack({lexer::Token::Type::SET});
        if (tokens[split - 1].type == lexer::Token::Type::ID && split > 1) {
            name           = tokens[split - 1].value;
            sptr<AST> type = Type::parse(tokens.slice(0, 1, split - 1), local, sr);
            if (type == nullptr) {
                 parser::error("Expected type", {tokens[0], tokens[split-1]}, "Expected a type before the identifier",0);
                // 25);
                return nullptr;
            }
            sptr<AST> expr =
                math::parse(tokens.slice(split + 1, 1, tokens.size() - 1), local, sr, type->getCstType());
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
                parser::warn("Wrong casing", {tokens[split - 1]}, "Constant name should be UPPER_CASE", 16);
            }
            if (m & parser::Modifier::CONST) {
                if (m & parser::Modifier::MUTABLE) {
                    parser::error(
                        "Variable declared as constant and mutable", {tokens[split - 1]},
                        "This variable was declared as 'const' (unchangeable) and 'mut' (changeable) at the same time.",
                        0);
                    return share<AST>(new AST);
                }
                if (m & parser::Modifier::STATIC) {
                    parser::warn(
                        "Variable declared as constant and static", {tokens[split - 1]},
                        "declaring a constant static is ambigous",
                        0);
                }
            }
            expr->forceType(type->getCstType());
            auto v     = new symbol::Variable(name, type->getCstType(), tokens2.tokens, sr);

            if (m & parser::Modifier::CONST) {
                if (!expr->is_const) {
                    parser::error(
                        "Non-constant value in constant variable", expr->getTokens(),
                        "you are trying to assign a non-constant value to a constant variable. This is not "
                        "supported.\nRemove the 'const' keyword to get an immutable variable which allows that.",
                        0);
                    if (!optimizer::do_constant_folding) parser::note(expr->getTokens(), "This could be a direct result of disabling --opt:constant-folding", 0);
                    delete v;
                    return share<AST>(new AST);
                }
                v->const_value = expr->value;
            }
            if (!sr->ALLOWS_NON_STATIC && !(m & (parser::Modifier::STATIC | parser::Modifier::CONST))) {
                parser::error("only static variables allowed", {tokens[split - 1]},
                                "only static variables are allowed in a Block of type "s + sr->getName(), 25);
            }

            v->isConst = m & parser::Modifier::CONST;
            v->isMutable = m & parser::Modifier::MUTABLE;
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

VarAccesAST::VarAccesAST(String name, symbol::Variable* sr, lexer::TokenStream tokens) {
    this->name   = name;
    this->var    = sr;
    this->tokens = tokens;
    this->is_const = var->isConst;
    if (is_const) {
        value = var->const_value;
    }
}

String VarAccesAST::_str() const {
    return "<ACCES "s + name + (is_const ? "[="s + var->const_value + "]" : ""s) + ">";
}

sptr<AST> VarAccesAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mVarAccesAST::parse\e[0m");
    if (tokens.size() == 0)
        return nullptr;
    String name = parse_name(tokens);
    DEBUG(5, "\tname: "s + name)
    if (name == "")
        return nullptr;
    if (name == "null")
        return share<AST>(new AST);
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
        p->last = tokens.tokens;
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

VarSetAST::VarSetAST(String name, symbol::Variable* sr, sptr<AST> expr, lexer::TokenStream tokens) {
    this->name   = name;
    this->var    = sr;
    this->expr   = expr;
    this->tokens = tokens;
}

String VarSetAST::_str() const {
    return "<"s + name + " = " + str(expr.get()) + ">";
}

sptr<AST> VarSetAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mVarSetAST::parse\e[0m");
    if (tokens.size() == 0)
        return nullptr;
    String name  = "";
    lexer::TokenStream::Match split = tokens.rsplitStack({lexer::Token::Type::SET});
    if (split.found() && (uint64)split == 0) {
        parser::error("Expected Expression", {tokens[tokens.size() - 1]}, "Expected an expression after '='", 31);
        return share<AST>(new AST);
    }
    if (!split.found())
        return nullptr;
    auto varname = split.before();
    name         = parse_name(varname);
    if (name == "") {
        parser::error("Expected Symbol", varname, "module name or variable name expected", 30);
        return nullptr;
    }

    sptr<AST> expr = math::parse(split.after(), local, sr);
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
        parser::error("Trying to set constant", tokens, "You are trying to set a variable which has a constant value.",
                      17);
        parser::note(p->tokens, "defined here:", 0);
        return share<AST>(new AST);
    }
    if (!(p == dynamic_cast<symbol::Variable*>(p) && ((symbol::Variable*)p)->isMutable)) {
        parser::error("Trying to set immutable", tokens, "You are trying to set a variable which was declared as immutable.",
                      18);
        parser::note(p->tokens, "defined here:", 0);
        return share<AST>(new AST);
    }
    if (p == dynamic_cast<symbol::Variable*>(p)){
        symbol::Variable::Status& u = ((symbol::Variable*) p)->used;
        if (u == symbol::Variable::PROVIDED) {
            fsignal<void, String, lexer::TokenStream, String, uint32, String> warn_error = parser::error;
            if (((symbol::Variable*) p)->isFree){
                warn_error = parser::warn;
            }

            warn_error("Type linearity violated", tokens, "Variable '\e[1m"s + name + "\e[0m' was never consumed.\nMake sure the variable is consumed before it is provided.", 0, "");
            parser::note(p->last, "last provided here", 0);
            if (warn_error == (fsignal<void, String, lexer::TokenStream, String, uint32, String>)parser::error) return share<AST>(new AST);
        }
        u = symbol::Variable::PROVIDED;
        p->last = tokens.tokens;
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
