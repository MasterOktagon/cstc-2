//
// ERRORS.cpp
//
// implements lexer errors
//

#include "errors.hpp"
#include <iostream>
#include <string>

void showError(String errstr, String errcol, String errcol_lite, String name, String msg, std::vector<lexer::Token> tokens, uint32 code, String appendix){
    if (tokens.size() == 0) return;

    String location;
    if (tokens.size() == 1){
        location = ":"s + std::to_string(tokens[0].l) + ":" + std::to_string(tokens[0].c); 
    }
    else {
        location = ":"s + std::to_string(tokens[0].l) + ":" + std::to_string(tokens[0].c); 
        location += " - " + std::to_string(tokens.at(tokens.size()-1).l) + ":" + std::to_string(tokens.at(tokens.size()-1).c + tokens.at(tokens.size()-1).value.size()-1);
    }

    std::cerr << errcol << errstr << ": " << name << "\e[0m @ \e[0m" << tokens[0].filename << "\e[1m" << location << "\e[0m" << (code == 0? ""s : " ["s + errstr[0] + std::to_string(code) + "]") << ":" << std::endl;
    std::cerr << msg << std::endl << std::endl;
    if (tokens.size() == 1){
        std::cerr << " " << fillup(std::to_string(tokens[0].l), 4) << " | " << *(tokens[0].line_contents) << std::endl;
        std::cerr << "      | " << errcol_lite << fillup("", tokens[0].c-1) << fillup("", tokens[0].value.size(), '^') << "\e[0m" << std::endl;
    } else {
        std::cerr << " " << fillup(std::to_string(tokens[0].l), 4) << " | " << (*(tokens[0].line_contents)) << std::endl;
        if (tokens[0].l == tokens.at(tokens.size()-1).l){
            std::cerr << "      | " << errcol_lite << fillup("", tokens[0].c-1) << fillup("", tokens.at(tokens.size()-1).c-(tokens[0].c-1)+tokens.at(tokens.size()-1).value.size()-1, '^') << "\e[0m" << std::endl;
        }
        else {
            std::cerr << "      | " << errcol_lite << fillup("", tokens[0].c-1) << fillup("", (*(tokens[0].line_contents)).size()-(tokens[0].c-1)-1, '^') << "\e[0m" << std::endl;
            if (tokens.at(tokens.size()-1).l - tokens[0].l > 1)
                std::cerr << "      | \t" << errcol_lite << "(" << std::to_string(tokens.at(tokens.size()-1).l - tokens[0].l - 1) << " line" << (tokens.at(tokens.size()-1).l - tokens[0].l - 1 == 1 ? "" : "s") << " hidden)\e[0m" << std::endl; 

            std::cerr << " " << fillup(std::to_string(tokens.at(tokens.size()-1).l), 4) << " | " << *(tokens.at(tokens.size()-1).line_contents) << std::endl;
            std::cerr << "      | " << errcol_lite  << fillup("", tokens.at(tokens.size()-1).c + tokens.at(tokens.size()-1).value.size()-1, '^') << "\e[0m" << std::endl;
        }
    }
    
    std::cerr << appendix << std::endl;
}

void lexer::error(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    showError("ERROR", "\e[1;31m", "\e[31m", name, msg, tokens, code, appendix);
}
void lexer::warn(String name, std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    showError("WARNING", "\e[1;33m", "\e[33m", name, msg, tokens, code, appendix);
}
void lexer::note(std::vector<lexer::Token> tokens, String msg, uint32 code, String appendix){
    showError("NOTE", "\e[1;36m", "\e[36m", "", msg, tokens, code, appendix);
}

