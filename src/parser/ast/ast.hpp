#pragma once

//
// AST.hpp
//
// layouts the AST class
//

#include "../../snippets.h"
#include "../../lexer/token.hpp"
#include <vector>

/**
 * @class represents an AST node
*/
class AST : public Repr {

    protected:
        /**
         * @brief debug represenstation
        */
        String _str();
        std::vector<lexer::Token> tokens;

    public:
        /**
         * @brief since this class is thought as a replacement for errorneous ASTs, there is only a default constructor
        */
        AST() = default;

        virtual ~AST() = default;

        virtual LLType  getLLType();
        virtual CstType getCstType();
        
        virtual void forceType(String t);
        virtual bool isConst();
        virtual String emit_ll(int*, String);
        virtual String emit_cst();
        virtual uint64 nodeSize(){return 0;};
};

extern std::string intab(std::string);
extern std::string insert(std::string val, std::string target);
extern std::string rinsert(std::string val, std::string target);


