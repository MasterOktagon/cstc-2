#pragma once

// 
// TOKEN.hpp
//
// layouts the Token class
//

#include "../snippets.h"

namespace lexer {
    class Token : public Repr {
    /**
     * @class Holds functions adding to the tokenizing features
    */
        protected:
        virtual String _str();

        public:
        enum Type {
            /**
             * @enum of all available Token types
            */

                //  SPECIAL   //
            NONE     ,        // undefined token
            ID       ,        // a name
            EF       ,        // File end
            END_CMD  ,        // ;

                //  LITERALS  //
            INT      ,        // int literal
            HEX      ,        // int literal in hexadecimal
            BINARY   ,        // int literal in binary
            BOOL     ,        // bool literal
            STRING   ,        // String literal
            CHAR     ,        // char literal
            FLOAT    ,        // float literal
            NULV     ,        // "null"

                //    MATH    //
            SET      ,        // =
            ADD      ,        // +
            SUB      ,        // -
            MUL      ,        // *
            DIV      ,        // /
            MOD      ,        // %
            POW      ,        // **
            INC      ,        // ++
            DEC      ,        // --
            NEC      ,        // - <int>

            NEG      ,        // ~
            AND      ,        // &
            OR       ,        // |
            XOR      ,        // ^
            SHL      ,        // <<
            SHR      ,        // >>
            LSHR     ,        // !>

                //  LOGICAL   //
            LAND     ,        // &&
            LOR      ,        // ||
            NOT      ,        // !

                // COMPARISON //
            EQ       ,        // ==
            NEQ      ,        // !=
            LESS     ,        // <
            GREATER  ,        // >
            GEQ      ,        // >=
            LEQ      ,        // <=

                //    FLOW    //
            QM       ,        // ?
            IN       ,        // :
            UNPACK   ,        // <-
            ADDR     ,        // #
            LIFETIME ,        // @
            SUBNS    ,        // ::
            ACCESS   ,        // .
            COMMA    ,        // ,
            DOTDOT   ,        // ..
            DOTDOTDOT,        // ...

                // PARANTHESIS //
            OPEN     ,        // (
            CLOSE    ,        // )
            BLOCK_OPEN ,      // {
            BLOCK_CLOSE,      // }
            INDEX_OPEN ,      // [
            INDEX_CLOSE,      // ]

                //  KEYWORDS  //
            IF        ,         
            ELSE      ,         
            FOR       ,   
            WHILE     ,        
            DO        ,    
            THROW     ,      
            CATCH     ,
            TRY       ,
            BREAK     ,
            CONTINUE  ,
            NOIMPL    ,
            RETURN    ,
            AS        ,
            OPERATOR  ,
            SWITCH    ,
            CASE      ,

            IMPORT    ,
            CLASS     ,
            ENUM      ,
            STRUCT    ,
            VIRTUAL   ,
            ABSTRACT  ,
            FRIEND    ,
            PROTECTED ,
            PRIVATE   ,
            STATIC    ,
            PUBLIC    ,
            FINAL     ,
            CONST     ,
            NAMESPACE ,
            NEW       ,
            FINALLY
        };

        Type type;     //> this tokens type
        String value;  //> this tokens contents

        uint64 l, c;                //> this tokens position in the File
        String filename;            //> this tokens File's name (for error messages)
        sptr<String> line_contents; //> this tokens line's contents (for error messages)

        Token(Type t, String content, uint64 l, uint64 c, String filename, sptr<String> lc);
        Token() = default;
        virtual ~Token();

        bool operator == (Token& other);
    };

    String getTokenName(Token::Type);
    /**
     * get a TokenType's Name as String
     */
}

const lexer::Token nullToken = lexer::Token(lexer::Token::NONE, "", 0, 0, "", nullptr);

