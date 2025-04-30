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
            /*
            Enum of all available Token types
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
            NEW
        };

        Type type;
        String value;

        uint64 l, c;
        String filename;
        sptr<String> line_contents;

        Token(Type t, String content, uint64 l, uint64 c, String filename, sptr<String> lc);
        Token() = default;
        virtual ~Token();
    };

    String getTokenName(Token::Type);
}




