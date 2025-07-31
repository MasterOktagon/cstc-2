#pragma once

//
// ERRORS.hpp
//
// errors for the lexer
//

#include "../snippets.h"
#include "token.hpp"

#include <vector>

/*
Currently lexer errors are working exactly as parser errors and use the same background logic,
but this might change in the future, so the lexer errors are different functions
*/

namespace lexer {
    extern void error(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix = "");
    extern void warn(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix = "");
    extern void note(std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix = "");
} // namespace lexer

