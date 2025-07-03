

// 
// TOKEN.cpp
//
// implements the Token class
//

#include "../snippets.h"
#include <string>
#include "token.hpp"

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

        default: return "UNKNOWN";
    }
    return "UNKNOWN";
}

String lexer::Token::_str(){
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

bool lexer::Token::operator==(Token& other) {
    return type == other.type && value == other.value;
}

