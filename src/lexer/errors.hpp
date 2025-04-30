#pragma once

//
// ERRORS.hpp
//
// errors for the lexer
//

#include "../snippets.h"
#include "token.hpp"
#include <vector>

namespace lexer {
    extern void error(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix="");
    extern void warn (String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix="");
    extern void note (std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix="");
}

