#pragma once

//
// MODULE.hpp
//
// layouts the module class
//

#include "lexer/token.hpp"
#include "snippets.h"
#include <list>
#include <vector>
#include <map>
#include <filesystem>

namespace std {
    namespace fs = std::filesystem;
}

class Module final : public Repr {
    /**
     * @class Module holds all information and contents of a Program module 
    */

    bool is_main_file = false; // wether this is the main module
    bool is_stdlib = false;
    std::map<String, Module*> deps = {}; // dependency modules
    std::vector<lexer::Token> tokens = {};

    protected:
    String _str();
    /**
     * @brief _str() required by Repr
    */

    public:

    String module_name;             // representation module name
    static std::fs::path directory; // main program directory
    std::fs::path hst_file;         // header location (relative)
    std::fs::path cst_file;         // source location (relative)

    bool isHeader();
    bool isKnown ();
    void preprocess();

    Module(String path, String dir, String name, bool is_stdlib=false, bool is_main_file=false);


    static std::map<String, Module*> known_modules;
    static std::list<String> unknown_modules;
    static std::list<Module*> modules;
    static String stdLibLocation();
    static String path2Mod(String path);
    static String mod2Path(String path);
    static Module* loadOrder(Module* a, Module* b);
    /**
     * @brief compare two modules about their load order
    */
    static Module* create(String path, String dir, String overpath, bool is_stdlib,std::vector<lexer::Token> tokens={}, bool is_main_file=false, bool from_path=false);

};





