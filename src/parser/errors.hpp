#pragma once

//
// ERRORS.hpp
//
// errors for the parser
//

#include "../snippets.h"
#include "../lexer/token.hpp"
#include <vector>

namespace parser {
    extern uint64 errc;
    extern uint64 warnc;
    extern bool   one_error;

    extern void mute();
    extern void unmute();

    extern void error(String name, lexer::TokenStream tokens, String msg, uint32 code, String appendix="");
    extern void warn (String name, lexer::TokenStream tokens, String msg, uint32 code, String appendix="");
    extern void note(lexer::TokenStream tokens, String msg, uint32 code, String appendix = "");
    extern void error(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix="");
    extern void warn (String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix="");
    extern void note (std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix="");
    extern void noteInsert (String msg, lexer::Token after, String insert, uint32 code, bool before=false, String appendix="");
    extern void showError(String errstr, String errcol, String errcol_lite, String name, String msg, std::vector<lexer::Token> tokens, uint32 code, String appendix);
}