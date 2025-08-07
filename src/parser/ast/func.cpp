

#include "func.hpp"

#include "../../debug/debug.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include "flow.hpp"
#include "literal.hpp"
#include "type.hpp"
#include "var.hpp"

#include <map>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <utility>
#include <vector>

// #define DEBUG

sptr<AST> FuncCallAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mFuncCallAST::parse\e[0m");
    if (tokens.size() < 3) { return nullptr; }

    lexer::TokenStream::Match split = tokens.rsplitStack({lexer::Token::OPEN}); //> find opening clamp
    if (!split.found()) { return nullptr; }
    if (tokens[-1].type != lexer::Token::Type::CLOSE) { // require a closing clamp. TODO(?) Error
        return nullptr;
    }
    String name = parse_name(split.before()); //> get function name
    if (name == "") {                         // something went wrong. Error checking is done in parse_name
        return share<AST>(new AST);
    }

    // parse arguments
    std::vector<sptr<AST>>      params       = {}; //> parameters
    std::map<String, sptr<AST>> named_params = {}; //> named parameters

    lexer::TokenStream t = split.after().slice(0, 1, -1);
    while (t.size() > 0) {
        lexer::TokenStream::Match m = t.rsplitStack({lexer::Token::COMMA});
        if (m.found() && (uint64) m == 0) {
            parser::error("Expression exptected", {t[m]}, "Expected an expression before ','", 31);
            return share<AST>(new AST);
        } else {
            lexer::TokenStream        t2 = m.found() ? m.before() : t;
            lexer::TokenStream::Match m2 = t2.rsplitStack({lexer::Token::SET});
            if (m2.found()) {
                // named parameter
                String opt_name = "";
                if (m2.before().size() == 1 && m2.before()[0].type == lexer::Token::ID) {
                    opt_name = t2[0].value;
                } else {
                    parser::error("Illegal optional argument name",
                                  t2,
                                  "An optional argument needs to have a single argument name",
                                  0); // TODO: improve error message
                    return share<AST>(new AST);
                }
                sptr<AST> a = math::parse(m2.after(), local + 1, sr);
                if (a == nullptr) {
                    parser::error("Expected Expression", m2.after(), "Expected a valid expression", 31);
                    return share<AST>(new AST);
                }
                named_params[opt_name] = a;

            } else {
                // positional parameter
                if (named_params.size() > 0) {
                    parser::error("Positional argument after optional argument",
                                  t2,
                                  "Positional arguments must before optional arguments",
                                  0);
                    parser::note(named_params.end()->second->getTokens(), "used here:", 0);
                }
                sptr<AST> a = math::parse(t2, local + 1, sr);
                if (a == nullptr) {
                    parser::error("Expected Expression", t2, "Expected a valid expression", 31);
                    return share<AST>(new AST);
                }
                params.push_back(a);
            }
        }
        t = m.after(); // iterate over params
    }

    // get function by name
    std::vector<symbol::Reference*> options = (*sr)[name];
    if (options.size() == 0) {
        parser::error("Unknown Function",
                      split.before(),
                      "A Funtion with name of '" + name + "' was not found in this scope",
                      26);
        return sptr<AST>(new AST);
    }

    // check for valid function
    DEBUG(3, "\tchecking for matches...");
    bool   matches = false;
    uint32 j       = 0;
    for (symbol::Reference* f : options) {
        if ((symbol::Function*) f == dynamic_cast<symbol::Function*>(f)) {
            symbol::Function* ft = (symbol::Function*) f;
            if (ft->parameters.size() != params.size()) { continue; }
            for (uint32 i = 0; i < params.size(); i++) {
                if (!parser::typeEq(params[i]->getCstType(), ft->parameters[i])) { continue; }
            }
            for (auto np : named_params) {
                if (ft->name_parameters.count(np.first) == 0) { continue; } 
                if (!parser::typeEq(np.second->getCstType(), ft->name_parameters[np.first].first)) { continue; }
            }
            matches = true;
            break;
        }
        j++;
    }
    // format error msg
    DEBUG(3, "\tmatches: "s + std::to_string(matches));
    DEBUG(3, "\tparams.size(): "s + std::to_string(params.size()));
    if (!matches) {
        String paramlist = "";
        bool paramlist_has_contents = false;
        for (int32 i = 0; i < int64(params.size()) - 1; i++) {
            paramlist += params.at(i)->getCstType() + ",";
            paramlist_has_contents  = true;
        }
        for (auto np : named_params) {
            paramlist += np.first + "=" + np.second->getCstType() + ",";
            paramlist_has_contents  = true;
        }
        if (paramlist_has_contents) paramlist += "\b";
        if (params.size() > 0) { paramlist += params.at(params.size() - 1)->getCstType(); }
        String appendix = "";
        if (options.size() > 0) {
            appendix = "Valid options are: \n";
            for (symbol::Reference* f : options) {
                if ((symbol::Function*) f == dynamic_cast<symbol::Function*>(f)) {
                    symbol::Function* ft  = (symbol::Function*) f;
                    appendix             += "\e[1m" + ft->getLoc() + "(";
                    bool has_parameters   = false;
                    for (CstType c : ft->parameters) {
                        has_parameters  = true;
                        appendix       += c + ", ";
                    }
                    for (auto c : ft->name_parameters) {
                        has_parameters  = true;
                        appendix       += c.second.first + " " + c.first + "=" + c.second.second->emitCST() +  ", ";
                    }
                    if (has_parameters) { appendix += "\b\b"; }
                    appendix += ")\e[0m\n";
                }
            }
        }
        parser::error("Mismatching operands",
                      tokens,
                      "\e[1m"s + options[0]->getLoc() + "(" + paramlist + ")\e[0m is not defined",
                      26,
                      appendix);
        return share<AST>(new AST);
    }

    // TODO check for ambigous functions

    symbol::Function* p = (symbol::Function*) (*sr)[name][j];
    return share<AST>(new FuncCallAST(name, params, p));
}

String FuncCallAST::emitLL(int* locc, String inp) const {
    String s = "invoke " + parser::LLType(fn->getCstType()) + " @" + name + "(";
    if (parser::LLType(fn->getCstType()) != "void") { s = "{} = " + s; }
    for (sptr<AST> p : params) {
        s += " ";
        s += parser::LLType(p->getCstType()) + " {}, ";
        s  = p->emitLL(locc, s);
    }
    if (s[s.size() - 1] == ' ') { s = s.substr(0, s.size() - 2); }
    s += " ) to label %exc\n";

    if (parser::LLType(fn->getCstType()) != "void") {
        s   = insert(String("%") + std::to_string(++(*locc)), s);
        inp = rinsert(String("%") + std::to_string(*locc), inp);
    }
    return s + inp;
}

String FuncCallAST::emitCST() const {
    String s = name + "(";
    for (sptr<AST> p : params) { s += p->emitCST(); }
    s += ")";
    return s;
}

void FuncCallAST::forceType(CstType type) {
    if (!parser::typeEq(type, fn->getReturnType())) {
        parser::error("Type mismatch", tokens, "expected a \e1[1m"s + type + "\e[0m, but "s + (fn->is_method ? "method"s : "function"s) + " returns " + fn->getReturnType(),17);
    }
}

sptr<AST> ArrayLengthAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mArrayLengthAST::parse\e[0m");
    if (tokens.size() < 3) { return nullptr; }
    lexer::TokenStream::Match m = tokens.rsplitStack({lexer::Token::ACCESS});
    if (m.found()) {
        lexer::TokenStream after = m.after();
        if (after.size() != 3) { return nullptr; }
        if (after[0].type != lexer::Token::ID || after[0].value != "len") { return nullptr; }
        if (after[1].type != lexer::Token::OPEN || after[2].type != lexer::Token::CLOSE) { return nullptr; }
        sptr<AST> from = math::parse(m.before(), local, sr);
        if (from == nullptr) { return nullptr; }
        if (from->getCstType().size() > 1 && from->getCstType().substr(from->getCstType().size()-2) == "[]"){

            return share<AST>(new ArrayLengthAST(from, tokens.slice(m, 1, tokens.size())));
        }
    }
    return nullptr;
}

void ArrayLengthAST::forceType(CstType type) {
    from->forceType("@unknown[]");
    if (!parser::typeEq(type, "usize")) {
        parser::error("Type mismatch", tokens, "expected a \e[1m"s + type + "\e[0m, but method returns usize",17);
    }
}

bool ArrayLengthAST::isConst() {
    if ( instanceOf(from, EmptyLiteralAST)){
        is_const=true;
        value = "0";
        return true;
    }
    if (from->is_const) {
        value = cast2(from, ArrayLiteralAST)->const_len;
    }
    return from->is_const;
}

sptr<AST> FuncDefAST::parse(PARSER_FN_PARAM) {
    DEBUG(4, "Trying \e[1mFuncDefAST::parse\e[0m");
    if (tokens.size() < 3) { return nullptr; }
    // parser::Modifier modifiers = parser::getModifier(tokens);
    lexer::TokenStream::Match m = tokens.rsplitStack({lexer::Token::OPEN});
    if (m.found()) {
        lexer::TokenStream t = m.before();
        if (t[-1].type != lexer::Token::ID) { return nullptr; }
        lexer::TokenStream        t2    = tokens.slice(m, 1, tokens.size());
        lexer::TokenStream::Match start = t2.rsplitStack({lexer::Token::BLOCK_OPEN});
        if (!start.found()) { return nullptr; }
        String name = t[-1].value;
        parser::checkName(name, t[-1]);

        DEBUG(2, "FuncDefAST::parse");

        sptr<AST> type = Type::parse(t.slice(0, 1, -1), local, sr);
        if (type == nullptr) {
            parser::error("Type expected", t.slice(0, 1, -1), "Exptected a valid type name", 0);
            return ERR;
        }
        // require a closing ')'. No error is thrown since rsplitstack should error already (TODO confirm this)
        DEBUG(3, "\tstart:  "s + std::to_string((uint64) m));
        DEBUGT(3, "\ttokens: ", &tokens);
        if (start.before()[-1].type != lexer::Token::CLOSE) { return ERR; }

        std::map<String, std::pair<std::vector<lexer::Token>, sptr<AST>>> parameters = {}; //> List of all parameters
        std::map<String, std::tuple<std::vector<lexer::Token>, sptr<AST>, sptr<AST>>> named_parameters =
            {}; //> List of all nonpositional parameters

        // parse function parameters
        lexer::TokenStream             param = start.before().slice(1, 1, -1);
        lexer::TokenStream             param_buffer({});
        std::map<String, lexer::Token> param_names = {};
        lexer::Token last_named = nullToken; //> last positional argument name for debugging and erroring
        DEBUGT(3, "\tparsing parameters...", &param);
        while (!param.empty()) {
            lexer::TokenStream::Match m = param.rsplitStack({lexer::Token::COMMA});
            if (m.found()) {
                param_buffer = m.before();
                param        = m.after();
            } else {
                param_buffer = param;
                param.tokens = {};
            }
            if (param_buffer.empty()) { continue; }
            m = param_buffer.rsplitStack({lexer::Token::SET});
            String pname;
            if (m.found()) {
                if (param_buffer[(uint64) m - 1].type != lexer::Token::ID) {
                    parser::error("Name expected",
                                  {param_buffer[(uint64) m - 1]},
                                  "expected a valid name for this parameter",
                                  0);
                    return ERR;
                }
                lexer::Token named = param_buffer[(uint64) m - 1];
                pname              = named.value;
                parser::checkName(pname, named);
                if (param_names.count(pname) > 0) {
                    parser::error("Parameter name already used",
                                  {named},
                                  "this parameter name was already used in this function",
                                  0);
                    parser::note({param_names[pname]}, "used here", 0);
                } else {
                    param_names[pname] = named;
                }
                last_named = named;

                sptr<AST> type = Type::parse(m.before().slice(0, 1, -1), local, sr);
                if (type == nullptr) {
                    if ((uint64) m < 2) {
                        parser::error("Type expected",
                                      {param_buffer[m]},
                                      "Expected a type before '"s + lexer::getTokenName(param_buffer[m].type) + "'",
                                      0);
                    } else {
                        parser::error("Type expected", param_buffer, "expected a type", 0);
                    }
                    return ERR;
                }

                sptr<AST> default_value = math::parse(m.after(), local, sr);
                if (default_value == nullptr) {
                    if ((uint64)m == param_buffer.size()) {
                        parser::error("Expression expected", {param_buffer[m]}, "Expected a default value after '='", 0);
                    } else {
                        parser::error("Expression expected", {m.after()}, "Expected a default value of type "s + type->getCstType() + " after '='", 0);
                    }
                    return ERR;
                }
                default_value->forceType(type->getCstType());

                named_parameters[pname] = std::tuple(param_buffer.tokens, default_value, type);
                
            } else {
                if (param_buffer[-1].type != lexer::Token::ID) {
                    parser::error("Name expected", {param_buffer[-1]}, "expected a valid name for this parameter", 0);
                    return ERR;
                }
                lexer::Token named = param_buffer[-1];
                pname              = named.value;
                parser::checkName(pname, named);
                if (param_names.count(pname) > 0) {
                    parser::error("Parameter name already used",
                                  {named},
                                  "this parameter name was already used in this function",
                                  0);
                    parser::note({param_names[pname]}, "used here", 0);
                } else {
                    param_names[pname] = named;
                }

                sptr<AST> type = Type::parse(param_buffer.slice(0, 1, -1), local, sr);
                if (type == nullptr) {
                    parser::error("Type expected",
                                  {param_buffer.slice(0, 1, -1)},
                                  "Expected a type before '"s + param_buffer[-1].value + "'",
                                  0);
                    return ERR;
                }
                parameters[pname] = std::pair(param_buffer.tokens, type);

                if (!(last_named == nullToken)) {
                    parser::error("positional parameter after named parameter",
                                  param,
                                  "This parameter does not have a default initializer (and is therefore "
                                  "positional),\nbut was created behind a named parameter",
                                  0);
                    parser::note({last_named}, "because of this parameter", 0);
                }
            }
        }
        // TODO add local variables

        lexer::TokenStream block = start.after();
        if (block[-1].type != lexer::Token::BLOCK_CLOSE) { return ERR; }
        DEBUG(3, "\tparsing block...");

        // TODO check for existing functions

        symbol::Function* f = new symbol::Function(sr, name, tokens);
        sr->add(name, f);
        f->include.push_back(sr);
        for (auto p : parameters) {
            sr->add(p.first, new symbol::Variable(p.first, p.second.second->getCstType(), p.second.first, f));
            ((symbol::Variable*) ((*sr)[p.first][0]))->used = symbol::Variable::PROVIDED;
            f->parameters.push_back(p.second.second->emitCST());
        }
        for (auto p : named_parameters) {
            sr->add(p.first, new symbol::Variable(p.first, std::get<2>(p.second)->getCstType(), std::get<0>(p.second), f));
            ((symbol::Variable*) ((*sr)[p.first][0]))->used = symbol::Variable::PROVIDED;
            f->name_parameters[p.first] = std::pair(std::get<2>(p.second)->emitCST(), std::get<1>(p.second));
        }

        sptr<AST> block_contents = SubBlockAST::parse(block.slice(0, 1, -1), local + 1, f);

        // check variables for usage
        for (std::pair<String, std::vector<symbol::Reference*>> sr : f->contents){
            if (sr.second.at(0) == dynamic_cast<symbol::Variable*>(sr.second.at(0))){
                auto var = (symbol::Variable*)sr.second.at(0);
                fsignal<void, String, std::vector<lexer::Token>, String, uint32, String> warn_error = parser::error;
                if (var->isFree){
                    warn_error = parser::warn;
                    if (var->getVarName()[0] == '_'){continue;}
                }
                if (var->used == symbol::Variable::PROVIDED){
                    warn_error("Type linearity violated", var->last, "This variable was provided, but never consumed." + (var->isFree ? "\nIf this was intended, prefix it with an '_'."s : ""s), 0, "");
                }
                if (var->used == symbol::Variable::UNINITIALIZED){
                    parser::warn("Unused Variable", var->tokens, "This variable was declared, but never used" + (var->isFree ? "\nIf this was intended, prefix it with an '_'."s : ""s), 0);
                }
            }
        }

        return share<AST>(new FuncDefAST(name,
        cast2(type, TypeAST) , parameters, f, cast2(block_contents, SubBlockAST)));
    }
    
    return nullptr;
}




