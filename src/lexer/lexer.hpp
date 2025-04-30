#pragma once

// 
// LEXER.hpp
//
// layouts the lexer functionality
//

#include "../snippets.h"
#include "token.hpp"
#include <vector>

namespace lexer {

    extern std::vector<Token> tokenize(String text, String filename);

    extern Token::Type getSingleToken(char c);
    extern Token::Type getDoubleToken(String s);
    extern Token::Type matchType(String s);
}


