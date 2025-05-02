
//
// AST.cpp
//
// implements the default ASTs functionality
//

#include "ast.hpp"

String AST::_str(){
    return "<AST>";
}

LLType AST::getLLType(){
    return "void";
}

CstType AST::getCstType(){
    return "@unknown";
}

void AST::forceType(CstType){}

bool AST::isConst(){
    return false;
}

String AST::emit_ll(int*, String s){return s;}

String AST::emit_cst(){return "";}
