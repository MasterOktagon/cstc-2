#pragma once

//
// FLOW.hpp
//
// layouts the flow ASTs
//

#include "ast.hpp"
#include <vector>
#include "../symboltable.hpp"


class SubBlockAST : public AST {
    protected:
    String _str() const;

    public:
    std::vector<sptr<AST>> contents = {}; //> block comments
    symbol::Namespace* parent = nullptr;

    SubBlockAST(){}
    virtual ~SubBlockAST() {}

    // fwd declarations @see @class AST

    virtual bool isConst(){return false;}
    virtual String emitLL(int* locc, String inp) const;

    virtual String emitCST() const;
    
    virtual CstType getCstType() const {return "void";}
    virtual LLType  getLLTtype() const { return ""; }
    virtual uint64 nodeSize() const { return contents.size();};
    virtual void forceType(CstType){}

    /**
     * @brief parse a Subblock
     */
    static sptr<AST> parse(PARSER_FN);
};

class IfAST : public AST {
    public:
    sptr<SubBlockAST> block;
    sptr<AST> cond;

    IfAST(){}
    virtual ~IfAST() {}

    // fwd declarations @see @class AST

    virtual bool isConst(){return false;}
    //virtual String emitLL(int* locc, String inp) const;

    virtual String emitCST() const;
    
    virtual CstType getCstType() const {return "void";}
    virtual LLType  getLLTtype() const { return ""; }
    virtual uint64 nodeSize() const { return block->contents.size();};
    virtual void forceType(CstType){}

    /**
     * @brief parse a Subblock
     */
    static sptr<AST> parse(PARSER_FN);

};










