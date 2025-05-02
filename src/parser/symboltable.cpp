
#include "symboltable.hpp"
#include <map>
#include <vector>
//#include "../debug.hpp"
#include "../snippets.h"

void symbol::Namespace::add(String loc, symbol::Reference* sr){
    size pos = loc.find("::");
    if (pos != String::npos && loc.substr(0,pos) != ""){
        contents.at(loc.substr(0,pos))[0]->add(loc.substr(pos+2), sr);
    }
    else {
        if (contents.count(loc) == 0) contents[loc] = {};
        contents.at(loc).push_back(sr);
    }

    //debug(str(sr) + " added at "s + loc.substr(0,pos), 3);
}

symbol::Reference::~Reference() = default;

CstType symbol::Function::getCstType() {
    String s = "["s + type;
    if (parameters.size() > 0) {
        s += "<-";
        for (CstType t : parameters) {
            s += t + ", ";
        }
        if (s.at(s.size()-1) == ',')
            s = s.substr(0, s.size()-1);
    }
    s += "]";
    return s;
}

symbol::Namespace::~Namespace() {
    for (std::pair<String, std::vector<Reference*>> v : contents) {
        for (Reference* t : v.second) {
            delete t;
        }
    }
}

std::vector<symbol::Reference*> symbol::Namespace::operator[] (String subloc){
    if (subloc == "") return {this};
    size pos = subloc.find("::");
    if (pos != String::npos){
        String head = subloc.substr(0,pos);
        String tail = subloc.substr(pos+2, subloc.size()-pos-2);
        if (contents.count(head) == 0) return {};
        if ((Namespace*) contents[head][0] != dynamic_cast<Namespace*>(contents[head][0])) return {};
        return (*((Namespace*) contents[head][0]))[tail];
    }
    if (contents.count(subloc) == 0) return {};
    return contents[subloc];
}

