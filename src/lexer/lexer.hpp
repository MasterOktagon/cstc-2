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

    extern int32 pretty_size; //> max length before LTL warning

    /**
     * @brief get a list of tokens from a String. Might issue warnings that defer further processing.
     *
     * @return Vector of Tokens tokenized.
     */
    extern std::vector<Token> tokenize(String text, String filename);

    /**
     * @brief try to fit a delimiting token into a single-char buffer
     *
     * @return token type found or Token::Type::NONE. @see @enum lexer::Token::Type
     */
    extern Token::Type getSingleToken(char c);

    /**
     * @brief try to fit a delimiting token into a string
     *
     * @param s String has to have length 2!
     *
     * @return token type found or Token::Type::NONE. @see @enum lexer::Token::Type
     */
    extern Token::Type getDoubleToken(String s);

    /**
     * @brief try to find the token type of a token that does not fit getSingleToken or getDoubleToken
     *
     * @param s String buffer
     *
     * @return token type found or Token::Type::NONE. @see @enum lexer::Token::Type
     */
    extern Token::Type matchType(String s);
}


