//
// Created by oskar on 15.04.25.
//
#include "snippets.h"

String operator ""s (const char* a, size){
    return a;
}

String fillup(String s, uint64 len, char with){
    String out = s;
    while (out.size() < len){
        out += with;
    }
    return out;
}

