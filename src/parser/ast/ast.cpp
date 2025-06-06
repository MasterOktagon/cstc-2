
//
// AST.cpp
//
// implements the default ASTs functionality
//

#include "ast.hpp"

String AST::_str(){
    return "<AST>";
}

LLType AST::getLLType() const {
    return "void"; // this should not break anything
}

CstType AST::getCstType() const {
    return "@unknown";
}

void AST::forceType(CstType){}

String AST::emitLL(int*, String s) const {return s;}

String AST::emitCST() const {return "";}

String intab(String i){
    size_t pos = i.find('\n', 0);
    while (pos != String::npos){
        i.replace(pos, 1, "\n\t");
        pos = i.find('\n', pos+1);
    }
    return String("\t") + i;
}

String insert(String val, String target){
    size_t pos = target.find_first_of("{}");
    if (pos != String::npos){
        target.replace(pos, 2, val);
    }
    return target;
}

String rinsert(String val, String target){
    size_t pos = target.rfind("{}");
    if (pos != String::npos){
        target.replace(pos, 2, val);
    }
    return target;
}

