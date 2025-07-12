

#include "func.hpp"
#include "../errors.hpp"
#include "../parser.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "base_math.hpp"
#include "var.hpp"
#include <string>
#include <sys/types.h>
#include <vector>

//#define DEBUG

sptr<AST> FuncCallAST::parse(PARSER_FN_PARAM) {
    if (tokens.size() < 3)
        return nullptr;

    lexer::TokenStream::Match split = tokens.rsplitStack({lexer::Token::OPEN}); //> find opening clamp
    if (!split.found()) {
        return nullptr;
    }
    if (tokens[-1].type != lexer::Token::Type::CLOSE) // require a closing clamp. TODO(?) Error
        return nullptr;
    String name = parse_name(split.before()); //> get function name
    if (name == "") // something went wrong. Error checking is done in parse_name
        return share<AST>(new AST);

    // parse arguments
    std::vector<sptr<AST>> params = {};           //> parameters
    std::map<String,sptr<AST>> named_params = {}; //> named parameters

    lexer::TokenStream t = split.after().slice(0, 1, -1);
    while (t.size() > 0) {
        lexer::TokenStream::Match m = t.rsplitStack({lexer::Token::COMMA});
        if (m.found() && (uint64)m == 0) {
            parser::error("Expression exptected", {t[m]}, "Expected an expression before ','", 31);
            return share<AST>(new AST);
        } else {
            lexer::TokenStream t2 = m.found() ? m.before() : t;
            lexer::TokenStream::Match m2 = t2.rsplitStack({lexer::Token::SET});
            if (m2.found()) {
                // named parameter
                String opt_name = "";
                if (m2.before().size() == 1 && m2.before()[0].type == lexer::Token::ID) {
                    opt_name = t2[0].value;
                } else {
                    parser::error("Illegal optional argument name", t2,
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
                    parser::error("Positional argument after optional argument", t2,
                                  "Positional arguments must before optional arguments", 0);
                    parser::note(named_params.end()->second->getTokens(),"used here:",0);
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
        parser::error("Unknown Function", split.before(),
                      "A Funtion with name of '" + name + "' was not found in this scope", 26);
        return sptr<AST>(new AST);
    }

    // check for valid function
    bool   matches = false;
    uint32 j       = 0;
    for (symbol::Reference* f : options) {
        if ((symbol::Function*)f == dynamic_cast<symbol::Function*>(f)) {
            symbol::Function* ft = (symbol::Function*) f;
            if (ft->parameters.size() != params.size())
                continue;
            for (uint32 i = 0; i < params.size(); i++) {
                if (!parser::typeEq(params[i]->getCstType(), ft->parameters[i]))
                    continue;
            }
            matches = true;
            break;
        }
        j++;
    }
    // format error msg
    if (!matches) {
        String paramlist = "";
        for (uint32 i = 0; i < params.size() - 1; i++) {
            paramlist += params[i]->getCstType() + ",";
        }
        if (params.size() > 0) {
            paramlist += params[params.size() - 1]->getCstType();
        }
        parser::error("Mismatching operands", tokens, name + "\e[1m(" + paramlist + ")\e[0m is not defined", 26);
        return share<AST>(new AST);
    }

    // TODO check for ambigous functions

    symbol::Function* p = (symbol::Function*)(*sr)[name][j];
    return share<AST>(new FuncCallAST(name, params, p));

    /*if (tokens.size() < 3)
        return nullptr;

    std::vector<sptr<AST>> params = {};                                         //> parameters
    uint32 split = parser::rsplitStack(tokens, {lexer::Token::Type::OPEN}, local); //> find opening clamp

#ifdef DEBUG
    std::cout << "FuncCallAST::parse: split:\t" << split << std::endl;
#endif

    if (split == 0 || split >= tokens.size() - 1) // no opening clamp was found
        return nullptr;
    if (tokens[tokens.size() - 1].type != lexer::Token::Type::CLOSE) // require a closing clamp. TODO(?) Error
        return nullptr;
    
    String name      = parse_name(parser::subvector(tokens, 0, 1, split)); // get function name
    uint32 namesplit = split;

    if (name == "") // something went wrong. Error checking is done in parse_name
        return share<AST>(new AST);

    // parse parameters
    auto tokens2 = parser::subvector(tokens, split + 1, 0, tokens.size() - 1);
    while (tokens2.size() > 0 && split < tokens2.size()) {
        split  = parser::rsplitStack(tokens2, {lexer::Token::Type::COMMA}, local + 1);
        sptr<AST> a = math::parse(parser::subvector(tokens2, 0, 1, split), local, sr);
        if (a == nullptr) {
            parser::error("Expected Expression", {tokens2[0], tokens2[split]}, "Expected a valid expression", 31);
            return share<AST>(new AST);
        }
        params.push_back(a);
        tokens2 = parser::subvector(tokens2, split, 1, tokens2.size());
    }
    if (tokens2.size() > 0) {
        split  = tokens2.size();
        sptr<AST> a = math::parse(parser::subvector(tokens2, 0, 1, split), local, sr);
        if (a == nullptr) {
            parser::error("Expected Expression", {tokens2[0], tokens2[split]}, "Expected a valid expression", 31);
            return share<AST>(new AST);
        }
        params.push_back(a);
    }

    // get function by name
    std::vector<symbol::Reference*> options = (*sr)[name];
    if (options.size() == 0) {
        parser::error("Unknown Function", {tokens[0], tokens[namesplit - 1]},
                      "A Funtion with name of '" + name + "' was not found in this scope", 26);
        return sptr<AST>(new AST);
    }

    // check for valid function
    bool   matches = false;
    uint32 j       = 0;
    for (symbol::Reference* f : options) {
        if ((symbol::Function*)f == dynamic_cast<symbol::Function*>(f)) {
            symbol::Function* ft = (symbol::Function*) f;
            if (ft->parameters.size() != params.size())
                continue;
            for (uint i = 0; i < params.size(); i++) {
                if (!parser::typeEq(params[i]->getCstType(), ft->parameters[i]))
                    continue;
            }
            matches = true;
            break;
        }
        j++;
    }
    // format error msg
    if (!matches) {
        String paramlist = "";
        for (uint32 i = 0; i < params.size() - 1; i++) {
            paramlist += params[i]->getCstType() + ",";
        }
        if (params.size() > 0) {
            paramlist += params[params.size() - 1]->getCstType();
        }
        parser::error("Mismatching operands", tokens, name + "\e[1m(" + paramlist + ")\e[0m is not defined", 26);
        return share<AST>(new AST);
    }

    // TODO check for ambigous functions

    symbol::Function* p = (symbol::Function*)(*sr)[name][j];
    return share<AST>(new FuncCallAST(name, params, p));*/
}

String FuncCallAST::emitLL(int* locc, String inp) const {
    String s = "invoke " + parser::LLType(fn->getCstType()) + " @" + name + "(";
    if (parser::LLType(fn->getCstType()) != "void")
        s = "{} = " + s;
    for (sptr<AST> p : params) {
        s += " ";
        s += parser::LLType(p->getCstType()) + " {}, ";
        s = p->emitLL(locc, s);
    }
    if (s[s.size() - 1] == ' ')
        s = s.substr(0, s.size() - 2);
    s += " ) to label %exc\n";

    if (parser::LLType(fn->getCstType()) != "void") {
        s   = insert(String("%") + std::to_string(++(*locc)), s);
        inp = rinsert(String("%") + std::to_string(*locc), inp);
    }
    return s + inp;
}

String FuncCallAST::emitCST() const {
    String s = name + "(";
    for (sptr<AST> p : params) {
        s += p->emitCST();
    }
    s += ")";
    return s;
}
