//
// ERRORS.cpp
//
// implements parser errors
//

#include "errors.hpp"
#include "../lexer/token.hpp"
#include "../snippets.h"
#include "ast/ast.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>


uint64 parser::errc = 0;
uint64 parser::warnc = 0;
bool parser::one_error = false;

void parser::showError(String errstr, String errcol, String errcol_lite, String name, String msg, std::vector<lexer::Token> tokens, uint32 code, String appendix){
    if (tokens.size() == 0) {
        std::cerr << "OH NO! " << errstr << " " << name << " could not be displayed:\n" << intab(msg) << "\n";
        return;
    }
    String location;
    if (tokens.size() == 1){
        location = ":"s + std::to_string(tokens[0].l) + ":" + std::to_string(tokens[0].c); 
    }
    else {
        location = ":"s + std::to_string(tokens[0].l) + ":" + std::to_string(tokens[0].c); 
        location += " - " + std::to_string(tokens.at(tokens.size()-1).l) + ":" + std::to_string(tokens.at(tokens.size()-1).c + tokens.at(tokens.size()-1).value.size()-1);
    }

    std::cerr << "\r" << errcol << errstr << ": " << name << "\e[0m @ \e[0m" << tokens[0].filename << "\e[1m" << location << "\e[0m" << (code == 0? ""s : " ["s + errstr[0] + std::to_string(code) + "]") << ":" << std::endl;
    std::cerr << msg << std::endl;
    std::cerr << "       | " << std::endl;
    if (tokens.size() == 1){
        std::cerr << " " << fillup(std::to_string(tokens[0].l), 5) << " | " << *(tokens[0].line_contents) << std::endl;
        std::cerr << "       | " << errcol_lite << fillup("", tokens[0].c-1) << fillup("", tokens[0].value.size(), '^') << "\e[0m" << std::endl;
    } else {
        std::cerr << " " << fillup(std::to_string(tokens[0].l), 5) << " | " << (*(tokens[0].line_contents)) << std::endl;
        if (tokens[0].l == tokens.at(tokens.size()-1).l){
            std::cerr << "       | " << errcol_lite << fillup("", tokens[0].c-1) << fillup("", tokens.at(tokens.size()-1).c-(tokens[0].c-1)+tokens.at(tokens.size()-1).value.size()-1, '^') << "\e[0m" << std::endl;
        }
        else {
            std::cerr << "       | " << errcol_lite << fillup("", tokens[0].c-1) << fillup("", (*(tokens[0].line_contents)).size()-(tokens[0].c-1)-1, '^') << "\e[0m" << std::endl;
            if (tokens.at(tokens.size()-1).l - tokens[0].l > 1)
                std::cerr << "       | \t" << errcol_lite << "(" << std::to_string(tokens.at(tokens.size()-1).l - tokens[0].l - 1) << " line" << (tokens.at(tokens.size()-1).l - tokens[0].l - 1 == 1 ? "" : "s") << " hidden)\e[0m" << std::endl; 

            std::cerr << " " << fillup(std::to_string(tokens.at(tokens.size()-1).l), 5) << " | " << *(tokens.at(tokens.size()-1).line_contents) << std::endl;
            std::cerr << "       | " << errcol_lite  << fillup("", tokens.at(tokens.size()-1).c + tokens.at(tokens.size()-1).value.size()-1, '^') << "\e[0m" << std::endl;
        }
    }
    
    std::cerr << appendix << std::endl;
}

void parser::error(String name, lexer::TokenStream tokens, String msg, uint32 code, String appendix){
    showError("ERROR", "\e[1;31m", "\e[31m", name, msg, tokens.tokens, code, appendix);
    errc++;
    if (one_error){
        std::exit(3);
    }
}
void parser::error(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    showError("ERROR", "\e[1;31m", "\e[31m", name, msg, tokens, code, appendix);
    errc++;
    if (one_error){
        std::exit(3);
    }
}
void parser::warn(String name, lexer::TokenStream tokens, String msg, uint32 code, String appendix){
    showError("WARNING", "\e[1;33m", "\e[33m", name, msg, tokens.tokens, code, appendix);
    warnc++;
}
void parser::note(lexer::TokenStream tokens, String msg, uint32 code, String appendix){
    showError("NOTE", "\e[1;36m", "\e[36m", "", msg, tokens.tokens, code, appendix);
}
void parser::warn(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    showError("WARNING", "\e[1;33m", "\e[33m", name, msg, tokens, code, appendix);
    warnc++;
}
void parser::note(std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    showError("NOTE", "\e[1;36m", "\e[36m", "", msg, tokens, code, appendix);
}

void parser::noteInsert(String msg, lexer::Token after, String insert, uint32 code, bool before, String appendix){

    String location;
    location = ":"s + std::to_string(after.l) + ":" + std::to_string(after.c); 

    std::cerr << "\r" << "\e[1;36mNote" << ": " << "\e[0m @ \e[0m" << after.filename << "\e[1m" << location << "\e[0m" << (code == 0? ""s : " [N"s + std::to_string(code) + "]") << ":" << std::endl;
    std::cerr << msg << std::endl;
    std::cerr << "       | " << std::endl;
    std::cerr << " " << fillup(std::to_string(after.l), 5) << " | " << (*(after.line_contents)).insert(after.c-1 + (before? 0 : after.value.size()), "\e[36m"s + insert + "\e[0m") << std::endl;
    std::cerr << "       | " << std::endl;
    std::cerr << appendix << std::endl;
}

