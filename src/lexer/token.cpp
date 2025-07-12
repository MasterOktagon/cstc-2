

// 
// TOKEN.cpp
//
// implements the Token class
//

#include "../snippets.h"
#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>
#include <stack>
#include "token.hpp"
#include "errors.hpp"

#define tokenToSTR(tok) case lexer::Token::Type::tok: return #tok;
String lexer::getTokenName(lexer::Token::Type type){
    switch(type){
        tokenToSTR(NONE)
        tokenToSTR(ID)
        tokenToSTR(EF)
        tokenToSTR(END_CMD)

        tokenToSTR(INT)
        tokenToSTR(HEX)
        tokenToSTR(BINARY)
        tokenToSTR(BOOL)
        tokenToSTR(STRING)
        tokenToSTR(CHAR)
        tokenToSTR(FLOAT)
        tokenToSTR(NULV)

        tokenToSTR(SET)
        tokenToSTR(ADD)
        tokenToSTR(SUB)
        tokenToSTR(MUL)
        tokenToSTR(DIV)
        tokenToSTR(MOD)
        tokenToSTR(INC)
        tokenToSTR(DEC)
        tokenToSTR(NEC)
        tokenToSTR(NEG)
        tokenToSTR(AND)
        tokenToSTR(OR)
        tokenToSTR(XOR)
        tokenToSTR(SHL)
        tokenToSTR(SHR)
        tokenToSTR(LSHR)
        tokenToSTR(LAND)
        tokenToSTR(LOR)
        tokenToSTR(NOT)

        tokenToSTR(EQ)
        tokenToSTR(NEQ)
        tokenToSTR(GREATER)
        tokenToSTR(GEQ)
        tokenToSTR(LEQ)

        tokenToSTR(QM)
        tokenToSTR(IN)
        tokenToSTR(UNPACK)
        tokenToSTR(ADDR)
        tokenToSTR(LIFETIME)
        tokenToSTR(SUBNS)
        tokenToSTR(ACCESS)
        tokenToSTR(COMMA)
        tokenToSTR(DOTDOT)
        tokenToSTR(DOTDOTDOT)

        tokenToSTR(OPEN)
        tokenToSTR(CLOSE)
        tokenToSTR(BLOCK_OPEN)
        tokenToSTR(BLOCK_CLOSE)
        tokenToSTR(INDEX_OPEN)
        tokenToSTR(INDEX_CLOSE)
        
        tokenToSTR(IF)
        tokenToSTR(ELSE)
        tokenToSTR(FOR)
        tokenToSTR(WHILE)
        tokenToSTR(DO)
        tokenToSTR(THROW)
        tokenToSTR(CATCH)
        tokenToSTR(BREAK)
        tokenToSTR(CONTINUE)
        tokenToSTR(NOIMPL)
        tokenToSTR(RETURN)
        tokenToSTR(AS)
        tokenToSTR(OPERATOR)

        tokenToSTR(IMPORT)
        tokenToSTR(CLASS)
        tokenToSTR(ENUM)
        tokenToSTR(STRUCT)
        tokenToSTR(VIRTUAL)
        tokenToSTR(FRIEND)
        tokenToSTR(PROTECTED)
        tokenToSTR(PRIVATE)
        tokenToSTR(STATIC)
        tokenToSTR(PUBLIC)
        tokenToSTR(MUT)
        tokenToSTR(CONST)
        tokenToSTR(NAMESPACE)
        tokenToSTR(ABSTRACT)
        tokenToSTR(FINALLY)
        tokenToSTR(DELETE)
        tokenToSTR(NOWRAP)

        default: return "UNKNOWN";
    }
    return "UNKNOWN";
}

String lexer::Token::_str() const {
    return "Token "s + getTokenName(type) + "\t\"" + fillup(value + "\"", 30) + " @ " + std::to_string(l) + ":" + std::to_string(c);
}

lexer::Token::Token(lexer::Token::Type t, String content, uint64 l, uint64 c, String filename, sptr<String> lc){
    type = t;
    this->l = l; this->c = c;
    this->filename = filename;
    this->value = content;
    this->line_contents = lc;
}

lexer::Token::~Token() = default;

bool lexer::Token::operator==(Token& other) { return type == other.type && value == other.value; }


lexer::TokenStream lexer::TokenStream::slice(int64 start, int64 step, int64 stop) const {
    std::vector<Token> t;
    if (start < 0)
        start += size();
    if (stop < 0)
        stop += size();
    bool d = start > stop;
    for (int64 i = start; d ? i > stop : i < stop; i += step) {
        t.push_back(tokens.at(i));
    }
    return TokenStream(t);
}

lexer::TokenStream::Match lexer::TokenStream::split(std::initializer_list<Token::Type> a, uint64 start_idx) const {
    for (uint64 i = start_idx; i < size(); i++) {
        Token t = tokens.at(i);
        if (std::find(a.begin(), a.end(), t.type) != a.end()) {
            return TokenStream::Match(i, true, this);
        }
    }
    return TokenStream::Match(0, false, this);
}


lexer::TokenStream::Match lexer::TokenStream::rsplit(std::initializer_list<Token::Type> a, uint64 start_idx) const {
    for (uint64 i = size() - start_idx; i > 0; i--) {
        Token t = tokens.at(i-1);
        if (std::find(a.begin(), a.end(), t.type) != a.end()) {
            return TokenStream::Match(i-1, true, this);
        }
    }
    return TokenStream::Match(0, false, this);
}

lexer::TokenStream::Match lexer::TokenStream::splitStack(std::initializer_list<Token::Type> delimiter, uint64 start_idx) const {
    std::stack<lexer::Token> s = {};
    const std::vector<Token>& t = tokens;

    uint64 i;
    for(i=t.size()-1-start_idx; i>0 ; i--){
        if (t[i].type == lexer::Token::Type::CLOSE || t[i].type == lexer::Token::Type::INDEX_CLOSE || t[i].type == lexer::Token::Type::BLOCK_CLOSE) {
            if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end())
                return TokenStream::Match(i, true, this);
            s.push(t[i]);
        }

        else if (t[i].type == lexer::Token::Type::OPEN){
            if(s.size() == 0) lexer::error("Unclosed PARANTHESIS", {t[i]}, "This PARANTHESIS was not closed" , 46);
            if(s.top().type != lexer::Token::Type::CLOSE) lexer::error("Unopened "s + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not opened" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::INDEX_OPEN){
            if(s.size() == 0) lexer::error("Unclosed INDEX", {t[i]}, "This INDEX was not closed" , 46);
            if(s.top().type != lexer::Token::Type::INDEX_CLOSE) lexer::error("Unopened "s + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not opened" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::BLOCK_OPEN){
            if(s.size() == 0) lexer::error("Unclosed BLOCK", {t[i]}, "This BLOCK was not closed" , 46);
            if(s.top().type != lexer::Token::Type::BLOCK_CLOSE) lexer::error("Unopened "s + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not opened" , 47);
            s.pop();
        }
        if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end())
            return TokenStream::Match(i, true, this);
    
    }
    for(uint64 j=0; j<s.size(); j++){
        lexer::error("Unopened " + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not opened" , 47);
        s.pop();
    }
    return TokenStream::Match(0, false, this);
}

lexer::TokenStream::Match lexer::TokenStream::rsplitStack(std::initializer_list<Token::Type> delimiter, uint64 start_idx) const {
    std::stack<lexer::Token> s = {};
    const std::vector<Token>& t = tokens;

    uint64 i;
    for(i=start_idx; i<t.size(); i++){
        if (t[i].type == lexer::Token::Type::OPEN || t[i].type == lexer::Token::Type::INDEX_OPEN || t[i].type == lexer::Token::Type::BLOCK_OPEN) {
            if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end())
                return TokenStream::Match(i, true, this);
            s.push(t[i]);
        }

        else if (t[i].type == lexer::Token::Type::CLOSE){
            if(s.size() == 0) lexer::error("Unopened PARANTHESIS", {t[i]}, "This PARANTHESIS was not opened" , 46);
            if(s.top().type != lexer::Token::Type::OPEN) lexer::error("Unclosed " + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not closed" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::INDEX_CLOSE){
            if(s.size() == 0) lexer::error("Unopened INDEX", {t[i]}, "This INDEX was not opened" , 46);
            if(s.top().type != lexer::Token::Type::INDEX_OPEN) lexer::error("Unclosed " + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not closed" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::BLOCK_CLOSE){
            if(s.size() == 0) lexer::error("Unopened BLOCK", {t[i]}, "This BLOCK was not opened" , 46);
            if(s.top().type != lexer::Token::Type::BLOCK_OPEN) lexer::error("Unclosed " + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not closed" , 47);
            s.pop();
        }
        if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end())
            return TokenStream::Match(i, true, this);
    
    }
    for(uint64 j=0; j<s.size(); j++){
        lexer::error("Unclosed " + getTokenName(s.top().type), {s.top()}, "This " + getTokenName(s.top().type) + " was not closed" , 47);
        s.pop();
    }
    return TokenStream::Match(0, false, this);
}

