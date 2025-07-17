
#include "targets.hpp"
#include "../snippets.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include "../debug/debug.hpp"

void target::list() {
    std::cout << "\e[1;36mINFO: Available targets:\e[0m" << std::endl << std::endl;
    std::cout << "\t\e[1;31m[BE]\e[0m - Bleeding Edge\t\e[1;33m[Beta]\e[0m - Beta" << std::endl << std::endl;

    for (String target : targets) {
        std::cout << "\t" << fillup(target, 30) << "\e[1;31m[BE]\e[0m" << std::endl; // TODO: change this
    }
}

bool target::isValid(String name) { return std::find(targets.begin(), targets.end(), name) != targets.end(); }

bool target::is(String subtarget) { return std::find(target.begin(), target.end(), subtarget) != target.end(); }

void target::set(String t) {
    String s;
    std::vector<String> parts = {};
    size   pos = t.find(":");
    while (pos != String::npos) {
        s += t.substr(0,pos) + ":";
        parts.push_back(s);
        DEBUG(3, "target::set s:"s + s);
        t = t.substr(pos+1);
        pos = t.find(":");
    }
    s += t;
    parts.push_back(s);
    parts.push_back(t);
    DEBUG(3, "target::set s:"s + s);

    target = parts;
}

symbol::Function* target::entry = nullptr;
std::vector<String> target::target = {};