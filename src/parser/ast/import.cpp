
//
// IMPORT.cpp
//
// import ingoring & debugging AST
//

#include "../../lexer/token.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"
#include "import.hpp"
#include "../errors.hpp"
#include <optional>
#include "../../module.hpp"

sptr<AST> ImportAST::parse(PARSER_FN_PARAM) {

    if (tokens.size() == 1 && tokens[0].type == lexer::Token::IMPORT) {
        parser::error("Expected name", {tokens[0]},
                                  "Exptected a module name after 'import'", 76);
                        return share<AST>(new AST);
    }
    if (tokens.size() < 2)
        return nullptr;
    if (tokens[0].type == lexer::Token::IMPORT) {
        uint32 i2 = 0;
        auto buffer = tokens.slice(1, 1, tokens.size());
        lexer::Token::Type last = lexer::Token::SUBNS;
        for (lexer::Token a : buffer.tokens){
            if (last == lexer::Token::SUBNS && (a.type == lexer::Token::DOTDOT || a.type == lexer::Token::ID)){
            } else if (a.type == lexer::Token::SUBNS &&
                       (last == lexer::Token::DOTDOT ||
                        last == lexer::Token::ID)) {
            }
            else if ((last == lexer::Token::DOTDOT || last == lexer::Token::ID) && a.type == lexer::Token::Type::END_CMD){
                break;
            }
            else if ((last == lexer::Token::DOTDOT || last == lexer::Token::ID) && a.type == lexer::Token::Type::AS){

                if (buffer.size() > i2 + 2){
                    if (buffer[i2 + 1].type == lexer::Token::Type::ID) {
                        if (!(buffer[i2 + 2].type ==
                              lexer::Token::Type::END_CMD)) {
                            parser::error("Expected End-of-Statement", {buffer[i2+2]},
                                  "Expected a ';'", 30);
                            return share<AST>(new AST);
                        }
                        String as = buffer[i2+1].value;
                        break;
                    } else {
                        parser::error("Expected Symbol", {buffer[i2+1]},
                                  "Exptected a name after 'as'", 30);
                        return share<AST>(new AST);
                    }
                } else {
                    parser::error("Expected Symbol", {a},
                                  "Exptected a name after 'as'", 30);
                    return share<AST>(new AST);
                }
            }
            else if ((last == lexer::Token::DOTDOT || last == lexer::Token::ID) && a.type == lexer::Token::Type::IN){
                if (buffer.size() > i2 + 3 &&
                    buffer[i2 + 1].type == lexer::Token::Type::BLOCK_OPEN &&
                    buffer[buffer.size() - 1].type == lexer::Token::Type::BLOCK_CLOSE) {
                
                    try {
                        getImportList(
                                buffer.slice(i2 + 2, 1,
                                        buffer.size() - 2))
                                .value();
                        break;
                    } catch (const std::bad_optional_access&) { // urgh!
                        parser::error(
                            "Expected symbol list",
                            buffer.slice(i2 + 2, 1, buffer.size() - 2),
                            "Expected a valid list of identifiers seperated with commas",
                            0); // TODO make this error better
                        return share<AST>(new AST);
                    }
                }
                else {
                    parser::error("Expected Block Open", {a},
                                  "Exptected a '{ after ':'", 51);
                    return share<AST>(new AST);
                }
            }
            else if (last == lexer::Token::SUBNS && a.type == lexer::Token::Type::MUL){
                
                if (buffer.size() > i2+1 && buffer[i2+1].type == lexer::Token::Type::END_CMD && i2 > 0){
                    break;
                } else {
                    parser::error("Import-all must be at line End", {a},
                                  "import-all must be in format 'import my::nice::module::*;'", 51);
                    return share<AST>(new AST);
                }
            }
            else {
                parser::error("Unexpected Token", {a},
                              "Encoutered an unexpected "s +
                                  lexer::getTokenName(a.type) +
                                  " Token in import statement",
                              76);
                return share<AST>(new AST);
            }
            last = a.type; i2++;
        }

        return share<AST>(new AST);
    }

    for (uint32 i = 0; i < tokens.size() - 1; i++) {
        if (tokens[i].type == lexer::Token::IMPORT && i != 0) {
            parser::error(
                "Unexpected import", {tokens[i]},
                "'import' is expected to be the first token in a line.", 75);
            return share<AST>(new AST);
        }
    }
    
    return nullptr;
}

