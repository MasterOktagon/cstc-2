
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

String intab(String i){
    //int j = 0;
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

