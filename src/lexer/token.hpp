#pragma once

// 
// TOKEN.hpp
//
// layouts the Token class
//

#include "../snippets.h"
#include <initializer_list>
#include <vector>

namespace lexer {
    class Token : public Repr {
    /**
     * @class Holds functions adding to the tokenizing features
    */
        protected:
        virtual String _str() const;

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
            MUT       ,
            CONST     ,
            NAMESPACE ,
            NEW       ,
            FINALLY   ,
            DELETE    ,
            NOWRAP   
        };

        Type type;     //> this tokens type
        String value;  //> this tokens contents

        uint64 l, c;                //> this tokens position in the File
        String filename;            //> this tokens File's name (for error messages)
        sptr<String> line_contents; //> this tokens line's contents (for error messages)

        Token(Type t, String content, uint64 l, uint64 c, String filename, sptr<String> lc);
        Token() = default;
        virtual ~Token();

        bool operator == (Token other);
    };

    /**
     * @brief get a TokenType's Name as String
     */
    String getTokenName(Token::Type);

    class TokenStream final : public Repr {
        
        public:
        std::vector<Token> tokens;

        protected:
        String _str() const {
            String s;
            for (Token t : tokens) {
                s += t.value + " ";
            }
            return s;
        }

        public:
        class Match final {
            uint64 at = 0;
            bool   was_found = false;
            const TokenStream* on = nullptr;

            public:
            Match(uint64 at, bool found, const TokenStream* on) {
                this->at = at;
                this->was_found = found;
                this->on = on;
            }
            ~Match() = default;

            inline bool found() const { return was_found; }
            TokenStream before() const { return found() ? on->slice(0, 1, at) : TokenStream({}); }
            TokenStream after() const { return found() ? on->slice(at + 1, 1, on->size()) : TokenStream({}); }

            operator uint64() { return at; }
            operator int64() { return at; }
            operator bool() {
                return was_found;
            }
        };
        
        TokenStream(std::vector<Token> tokens){this->tokens = tokens;}

        TokenStream slice(int64 start, int64 step, int64 stop) const;

        TokenStream getTS(int64 idx) {
            if (idx < 0)
                idx += size();
            return TokenStream({tokens.at(idx)});
        }

        Token operator[](int64 idx) const {
            if (idx < 0)
                idx += size();
            return tokens.at(idx);
        }
        inline uint64 size() const noexcept { return tokens.size(); }

        Match splitStack(std::initializer_list<lexer::Token::Type>, uint64 start_idx=0) const;
        Match rsplitStack(std::initializer_list<lexer::Token::Type>, uint64 start_idx=0) const;

        Match split(std::initializer_list<lexer::Token::Type>, uint64 start_idx=0) const;
        Match rsplit(std::initializer_list<lexer::Token::Type>, uint64 start_idx=0) const;

        inline Match operator [] (lexer::Token::Type type){return split({type});}

        inline bool empty() const { return size() == 0; }
    };
}

const lexer::Token nullToken = lexer::Token(lexer::Token::NONE, "", 0, 0, "", nullptr);

