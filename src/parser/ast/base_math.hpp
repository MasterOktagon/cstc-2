#pragma once

//
// BASE_MATH.hpp
//
// layouts base math ASTs
//

#include "ast.hpp"
#include <vector>
#include "../../lexer/token.hpp"
#include "../symboltable.hpp"

/**
 * @namespace that includes the "default" expression parsing functions
 */
namespace math {

    /**
     * @brief parse an expression (e.g. variable call, function call, literal, math, ...)
     * 
     * @return AST or nullptr if nothing matched
     */
    extern sptr<AST> parse(PARSER_FN);

    /**
     * @brief parse paranthesis (with an expression inside)
     * 
     * @return AST or nullptr if nothing matched
     */
    extern sptr<AST> parse_pt(PARSER_FN);
}

/**
 * @class for generic expression ASTs
 */
class ExpressionAST : public AST {
    public:
    ExpressionAST(){};
    virtual ~ExpressionAST(){};
};

class DoubleOperandAST : public ExpressionAST {
    protected:
    sptr<AST> left;  //> left operand
    sptr<AST> right; //> right operand
    lexer::Token::Type op = lexer::Token::NONE;
    
    public:
    DoubleOperandAST() {};
    virtual ~DoubleOperandAST(){};

    LLType getLLType()   const final;
    CstType getCstType() const final;

    uint64 nodeSize() const final;
};

class UnaryOperandAST : public ExpressionAST {
    protected:
    sptr<AST> left;  //> left operand
    lexer::Token::Type op = lexer::Token::NONE;
    
    public:
    UnaryOperandAST() {};
    virtual ~UnaryOperandAST(){};

    // fwd declarations. @see @class AST 
    
    LLType getLLType()   const final;
    CstType getCstType() const final;

    uint64 nodeSize() const final;
};

/**
 * @class that represents a '+' operation
 */
class AddAST : public DoubleOperandAST {

    public:
    AddAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~AddAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse an addition or subtraction (due to both having the same precedences)
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '-' operation
 */
class SubAST : public DoubleOperandAST {

    public:
    SubAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~SubAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);
};

/**
 * @class that represents a '*' operation
 */
class MulAST : public DoubleOperandAST {

    public:
    MulAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~MulAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a multiplication, division or remainder (modulo) (due to them having the same precedences)
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '/' operation
 */
class DivAST : public DoubleOperandAST {

    public:
    DivAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~DivAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);
};

/**
 * @class that represents a '%' operation
 */
class ModAST : public DoubleOperandAST {

    public:
    ModAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~ModAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);
};

/**
 * @class that represents a '**' operation
 */
class PowAST : public DoubleOperandAST {

    public:
    PowAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~PowAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a power
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '||' operation
 */
class LorAST : public DoubleOperandAST {
    public:
    LorAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~LorAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a logical and
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '&&' operation
 */
class LandAST : public DoubleOperandAST {
    public:
    LandAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~LandAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a multiplication, division or remainder (modulo) (due to them having the same precedences)
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '|' operation
 */
class OrAST : public DoubleOperandAST {
    public:
    OrAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~OrAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a multiplication, division or remainder (modulo) (due to them having the same precedences)
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '&' operation
 */
class AndAST : public DoubleOperandAST {
    public:
    AndAST(sptr<AST> left, sptr<AST> right, std::vector<lexer::Token> tokens);
    virtual ~AndAST();

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a multiplication, division or remainder (modulo) (due to them having the same precedences)
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '!' operation
 */
class NotAST : public UnaryOperandAST {
    public:
    NotAST(sptr<AST> inner, std::vector<lexer::Token> tokens);
    virtual ~NotAST() = default;

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a not
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents a '~' operation
 */
class NegAST : public UnaryOperandAST {
    public:
    NegAST(sptr<AST> inner, std::vector<lexer::Token> tokens);
    virtual ~NegAST() = default;

    // fwd declarations. @see @class AST 

    virtual String emitLL  (int*, String) const;
    virtual String emitCST () const;

    virtual void forceType(String type);

    /**
     * @brief parse a bitwise negation
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents an 'as' cast operation
 */
class CastAST : public ExpressionAST {
    sptr<AST> from;
    sptr<AST> type;

    public:
    CastAST(sptr<AST> from, sptr<AST> type, std::vector<lexer::Token> tokens);
    virtual ~CastAST(){};

    // fwd declarations. @see @class AST 

    CstType getCstType() const { return type->getCstType(); }
    LLType  getLLType () const { return type->getLLType(); }

    virtual uint64 nodeSize () const { return 1; }
    virtual String emitLL   (int*, String) const;
    virtual String emitCST  () const;

    virtual void forceType (String type);

    /**
     * @brief parse a cast
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents an check-and-unwrap operation
 */
class CheckAST : public ExpressionAST {
    sptr<AST> of;

    public:
    CheckAST(sptr<AST> of, std::vector<lexer::Token> tokens){this->of = of; this->tokens = tokens;}
    virtual ~CheckAST(){};

    // fwd declarations. @see @class AST
    
    CstType getCstType() const {
        if (of->getCstType() == "") return "";
        return of->getCstType().substr(0, of->getCstType().size()-1);
    }
    LLType getLLType() const { return ""; }

    virtual uint64 nodeSize() const {return 1;} // how many nodes to to do
    virtual String emitLL(int*, String) const;

    String emitCST() const { return of->emitCST() + "?"; }
    
    void forceType(CstType type);

    /**
     * @brief parse a cast
     * 
     * @return AST or nullptr if no match
     */
    static sptr<AST> parse(PARSER_FN);
};

/**
 * @class that represents an check-and-unwrap operation
 */
class AddrOfAST : public ExpressionAST {
    sptr<AST> of;

    public:
    AddrOfAST(sptr<AST> of);
    virtual ~AddrOfAST();

    // fwd declarations. @see @class AST 

    CstType getCstType() const { return of->getCstType()+"*"; }
    LLType getLLType() const;

    virtual uint64 nodeSize() const {return 1;} // how many nodes to to do
    virtual String emitLL(int*, String) const;

    String emitCST() const;
};

