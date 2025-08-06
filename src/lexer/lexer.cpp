
//
// LEXER.cpp
//
// implements the lexer functionality
//

#include "lexer.hpp"

#include "../snippets.h"
#include "errors.hpp"
#include "token.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

int32 lexer::pretty_size = 120; //> max line len before warning

/**
 * @brief try to fit a delimiting token into a single-char buffer
 *
 * @return token type found or Token::Type::NONE. @see @enum lexer::Token::Type
 */
lexer::Token::Type lexer::getSingleToken(char c) {
    switch (c) {
        case ';' : return lexer::Token::Type::END_CMD;

        case '=' : return lexer::Token::Type::SET;
        case '+' : return lexer::Token::Type::ADD;
        case '-' : return lexer::Token::Type::SUB;
        case '*' : return lexer::Token::Type::MUL;
        case '/' : return lexer::Token::Type::DIV;
        case '%' : return lexer::Token::Type::MOD;
        case '~' : return lexer::Token::Type::NEG;
        case '&' : return lexer::Token::Type::AND;
        case '|' : return lexer::Token::Type::OR;
        case '^' : return lexer::Token::Type::XOR;
        case '!' : return lexer::Token::Type::NOT;

        case '<' : return lexer::Token::Type::LESS;
        case '>' : return lexer::Token::Type::GREATER;

        case '?' : return lexer::Token::Type::QM;
        case ':' : return lexer::Token::Type::IN;
        case '#' : return lexer::Token::Type::ADDR;
        case '@' : return lexer::Token::Type::LIFETIME;
        case '.' : return lexer::Token::Type::ACCESS;
        case ',' : return lexer::Token::Type::COMMA;

        case '(' : return lexer::Token::Type::OPEN;
        case ')' : return lexer::Token::Type::CLOSE;
        case '{' : return lexer::Token::Type::BLOCK_OPEN;
        case '}' : return lexer::Token::Type::BLOCK_CLOSE;
        case '[' : return lexer::Token::Type::INDEX_OPEN;
        case ']' : return lexer::Token::Type::INDEX_CLOSE;
        default  : return lexer::Token::Type::NONE;
    }
}

/**
 * @brief try to fit a delimiting token into a string
 *
 * @param s String has to have length 2!
 *
 * @return token type found or Token::Type::NONE. @see @enum lexer::Token::Type
 */
lexer::Token::Type lexer::getDoubleToken(String s) {
    if (s == "++") { return lexer::Token::Type::INC; }
    if (s == "--") { return lexer::Token::Type::DEC; }
    if (s == "**") { return lexer::Token::Type::POW; }

    if (s == "<<") { return lexer::Token::Type::SHL; }
    if (s == ">>") { return lexer::Token::Type::SHR; }
    if (s == "!>") { return lexer::Token::Type::LSHR; }

    if (s == "&&") { return lexer::Token::Type::LAND; }
    if (s == "||") { return lexer::Token::Type::LOR; }

    if (s == "==") { return lexer::Token::Type::EQ; }
    if (s == "!=") { return lexer::Token::Type::NEQ; }
    if (s == ">=") { return lexer::Token::Type::GEQ; }
    if (s == "<=") { return lexer::Token::Type::LEQ; }

    if (s == "<-") { return lexer::Token::Type::UNPACK; }
    if (s == "::") { return lexer::Token::Type::SUBNS; }
    if (s == "..") { return lexer::Token::Type::DOTDOT; }

    return lexer::Token::Type::NONE;
}

/**
 * @brief try to find the token type of a token that does not fit getSingleToken or getDoubleToken
 *
 * @param c String buffer
 *
 * @return token type found or Token::Type::NONE. @see @enum lexer::Token::Type
 */
lexer::Token::Type lexer::matchType(String c) {
    // fallback = everything else
    lexer::Token::Type type = lexer::Token::Type::ID;

    // bool literal
    if (c == "true" || c == "false") { return lexer::Token::Type::BOOL; }

    // int literal
    const std::regex int_regex("[0-9]+");
    const std::regex hex_regex("0x[0-9a-fA-F]+");
    const std::regex binary_regex("0b[0-1]+");

    if (regex_match(c.c_str(), int_regex)) { return lexer::Token::Type::INT; }
    if (regex_match(c.c_str(), hex_regex)) { return lexer::Token::Type::HEX; }
    if (regex_match(c.c_str(), binary_regex)) { return lexer::Token::Type::BINARY; }

    // char & String literal
    if (c[0] == '\'' && c[c.size() - 1] == '\'') { return lexer::Token::Type::CHAR; }
    if (c[0] == '\"' && c[c.size() - 1] == '\"') { return lexer::Token::Type::STRING; }

    // Keywords
    if (c == "as") {
        type = lexer::Token::Type::AS;
    } else if (c == "if") {
        type = lexer::Token::Type::IF;
    } else if (c == "else") {
        type = lexer::Token::Type::ELSE;
    } else if (c == "for") {
        type = lexer::Token::Type::FOR;
    } else if (c == "while") {
        type = lexer::Token::Type::WHILE;
    } else if (c == "return") {
        type = lexer::Token::Type::RETURN;
    } else if (c == "continue") {
        type = lexer::Token::Type::CONTINUE;
    } else if (c == "break") {
        type = lexer::Token::Type::BREAK;
    } else if (c == "namespace") {
        type = lexer::Token::Type::NAMESPACE;
    } else if (c == "import") {
        type = lexer::Token::Type::IMPORT;
    } else if (c == "noimpl") {
        type = lexer::Token::Type::NOIMPL;
    } else if (c == "class") {
        type = lexer::Token::Type::CLASS;
    } else if (c == "struct") {
        type = lexer::Token::Type::STRUCT;
    } else if (c == "enum") {
        type = lexer::Token::Type::ENUM;
    } else if (c == "mut") {
        type = lexer::Token::Type::MUT;
    } else if (c == "abstract") {
        type = lexer::Token::Type::ABSTRACT;
    } else if (c == "do") {
        type = lexer::Token::Type::DO;
    } else if (c == "public") {
        type = lexer::Token::Type::PUBLIC;
    } else if (c == "switch") {
        type = lexer::Token::Type::SWITCH;
    } else if (c == "case") {
        type = lexer::Token::Type::CASE;
    } else if (c == "private") {
        type = lexer::Token::Type::PRIVATE;
    } else if (c == "protected") {
        type = lexer::Token::Type::PROTECTED;
    } else if (c == "const") {
        type = lexer::Token::Type::CONST;
    } else if (c == "static") {
        type = lexer::Token::Type::STATIC;
    } else if (c == "throw") {
        type = lexer::Token::Type::THROW;
    } else if (c == "catch") {
        type = lexer::Token::Type::CATCH;
    } else if (c == "try") {
        type = lexer::Token::Type::TRY;
    } else if (c == "new") {
        type = lexer::Token::Type::NEW;
    } else if (c == "virtual") {
        type = lexer::Token::Type::VIRTUAL;
    } else if (c == "delete") {
        type = lexer::Token::Type::DELETE;
    } // Not sure about this
    else if (c == "operator") {
        type = lexer::Token::Type::OPERATOR;
    } else if (c == "finally") {
        type = lexer::Token::Type::FINALLY;
    } else if (c == "nowrap") {
        type = lexer::Token::Type::NOWRAP;
    } else if (c == "null") {
        type = lexer::Token::Type::NULV;
    }
    

    return type;
}

/**
 * @brief check if a is a delimiter
 */
#define delimiter(a) a == ' ' || a == '\t' || a == '\n'

/**
 * @brief handle and clear the buffer (add a token if the buffer is not empty)
 */
#define handleBuffer()                                                                                         \
    if (buffer.size() > 0) {                                                                                   \
        tokens.push_back(Token(matchType(buffer), buffer, line, col - buffer.size(), filename, lc));           \
        if (pretty_size != -1 && col > (uint64) pretty_size) too_long.push_back(tokens.at(tokens.size() - 1)); \
        buffer = "";                                                                                           \
    }

/**
 * @brief Update Variables and raise Warning if line is too long
 */
#define updateVars()                                                                                   \
    if (c == '\n') {                                                                                   \
        line_comment = false;                                                                          \
        line++;                                                                                        \
        col = 0;                                                                                       \
        lc  = share<String>(new String);                                                               \
        if (too_long.size() > 0) {                                                                     \
            warn("Line too long", {too_long}, "It will become hard to read if you do long lines", 14); \
            note(too_long,                                                                             \
                 "current max length is "s + std::to_string(pretty_size) +                             \
                     ", you can adjust this with the --max-line-len argument",                         \
                 0);                                                                                   \
            too_long.clear();                                                                          \
        }                                                                                              \
    }

/**
 * @brief get a list of tokens from a String. Might issue warnings that defer further processing.
 *
 * @return Vector of Tokens tokenized.
 */
lexer::TokenStream lexer::tokenize(String text, String filename) {
    std::vector<lexer::Token> tokens       = {};    //> output Token vector
    uint64                    col          = 0;     //> current column
    uint64                    line         = 1;     //> current line
    bool                      line_comment = false; //> if currently in a line comment
    uint64                    ml_comment   = 0;     //> multiline comment level. If 0 => no comment
#define NO_COMMENT 0
    sptr<String> lc = share<String>(
        new String); //> current line buffer for token debug. Used with an sptr to autodelete when not required
    Token              ml_open;       //> cached fist multiline open
    std::vector<Token> too_long = {}; //> Tokens after LTL limit

    String      buffer = ""; //> current token buffer
    Token::Type t;           //> current Token type
    for (uint32 i = 0; i < text.size(); i++) {
        // update variables
        char c = text[i];
        col++;
        if (c != '\n') {
            *lc += c; // add char to line buffer
        }

        if (line_comment) { goto update; } // ignore rest of line

        // comments
        if (c == '/' && i < text.size() - 1 && text[i + 1] == '/') { // Single line comment
            handleBuffer();
            if (ml_comment == NO_COMMENT) { line_comment = true; }
            goto update;
        }
        if (c == '/' && i < text.size() - 1 && text[i + 1] == '*') { // Multiline comment start
            handleBuffer();
            ml_comment++;
            if (ml_comment == 1) {
                ml_open = Token(Token::Type::NONE, "/*", line, col, filename, lc); // cache opening token
            }
            goto update;
        }
        if (c == '*' && i < text.size() - 1 && text[i + 1] == '/') {
            *lc += '/';
            if (ml_comment == NO_COMMENT) {
                lexer::error("Unopened multiline comment",
                             {Token(Token::Type::NONE, "*/", line, col, filename, lc)},
                             "This multiline comment was never opened",
                             2350);
            }
            ml_comment -= ml_comment > NO_COMMENT ? 1 : 0; // make sure ml_comment doesn't underflow
            i++;
            col++;
            goto update;
        }
        if (ml_comment > NO_COMMENT) { goto update; } // => in multiline_comment

        // Special Error: unresolved Git merge conflict
        if (c == '<' && text.size() >= i + 12 && text.substr(i, 13) == "<<<<<<<< HEAD") {
            *lc += "<<<<<<< HEAD"; // Add to line buffer
            lexer::error(
                "Unresolved merge conflict",
                {Token(lexer::Token::Type::NONE, "<<<<<<<< HEAD", line, col, filename, lc)},
                "There is an unresolved git merge conflict in this file.\nTry\n \e[36m$\e[0m git mergetool\nfor help",
                -3);
            while (!std::regex_match(*lc, std::regex(">>>>>>> .*"))) { // move fwd until merge conflict end
                i++;
                col++;
                c = text[i];
                if (i >= text.size()) { return TokenStream({}); }
                updateVars();
            }
            while (i + 1 < text.size() && text[i + 1] != '\n') {
                i++;
                col++;
                c = text[i];
                updateVars()
            }
        }

        // Special Token: ...
        if (i < text.size() - 2) {
            if (c == '.' && text[i + 1] == '.' && text[i + 2] == '.') {
                handleBuffer();
                tokens.push_back(Token(Token::Type::DOTDOTDOT, "...", line, col, filename, lc));
                col += 2;
                i   += 2;
                *lc += text.substr(i, 2);
                goto update;
            }
        }

        // Double delimiter Tokens (ex. ||, &&, <<)
        if (i < text.size() - 1) {
            t = getDoubleToken(""s + c + text[i + 1]);
            if (t != Token::Type::NONE) {
                handleBuffer();
                tokens.push_back(Token(t, ""s + c + text[i + 1], line, col, filename, lc));
                col++;
                i   += 1;
                *lc += text[i];
                goto update;
            }
        }

        // Single delimiter Tokens (ex. ^, &, ?, <)
        t = getSingleToken(c);
        if (t != Token::Type::NONE) {
            handleBuffer();
            tokens.push_back(Token(t, ""s + c, line, col, filename, lc));
            goto update;
        }

        if (delimiter(c)) {
            handleBuffer();
        } else {
            buffer += c;
        }
update:
        updateVars();
    }
    if (ml_comment > NO_COMMENT) { // Some Multiline comment was never closed
        lexer::warn("Unclosed multiline comment",
                    {ml_open},
                    "This multiline comment was never closed. This could cause problems with commented code",
                    0);
    }
    if (tokens.size() == 0) { // Empty file warning
        std::cerr << "\r\e[1;33mWARNING:\e[0m\e[1m " << filename << "\e[0m appears to be empty.\n";
    }

    return TokenStream(tokens);
}
