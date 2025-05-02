//
// ERRORS.cpp
//
// implements lexer errors
//

#include "errors.hpp"
#include <iostream>
#include <string>

#include "../parser/errors.hpp"

void lexer::error(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    parser::showError("ERROR", "\e[1;31m", "\e[31m", name, msg, tokens, code, appendix);
}
void lexer::warn(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    parser::showError("WARNING", "\e[1;33m", "\e[33m", name, msg, tokens, code, appendix);
}
void lexer::note(std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    parser::showError("NOTE", "\e[1;36m", "\e[36m", "", msg, tokens, code, appendix);
}

