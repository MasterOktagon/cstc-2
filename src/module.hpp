#pragma once

//
// MODULE.hpp
//
// layouts the module class
//

#include "lexer/token.hpp"
#include "parser/symboltable.hpp"
#include "snippets.h"
#include <list>
#include <vector>
#include <map>
#include <filesystem>
#include <optional>

namespace std {
    namespace fs = std::filesystem;
}

class Module final : public symbol::Namespace {
    /**
     * @class Module holds all information and contents of a Program module 
    */

    bool is_main_file = false; // wether this is the main module
    bool is_stdlib = false;
    std::map<String, Module *> deps = {};      //> dependency modules
    std::vector<lexer::Token> tokens = {};     //> modules tokens

    protected:
    String _str();
    /**
     * @brief _str() required by Repr
    */

    public:

    String module_name;             //> representation module name
    static std::fs::path directory; //> main program directory
    std::fs::path hst_file;         //> header location (relative)
    std::fs::path cst_file;         //> source location (relative)

    bool isHeader();
    bool isKnown ();
    void preprocess();
    void parse();

    Module(String path, String dir, String name, bool is_stdlib=false, bool is_main_file=false);


    static std::map<String, Module*> known_modules;
    static std::list<String> unknown_modules;
    static std::list<Module*> modules;
    static String stdLibLocation();
    static String path2Mod(String path);
    static String mod2Path(String path);
    static bool loadOrder(Module* a, Module* b);
    /**
     * @brief compare two modules about their load order
    */
    static Module* create(String path, String dir, String overpath, bool is_stdlib,std::vector<lexer::Token> tokens={}, bool is_main_file=false, bool from_path=false);
};

extern std::optional<std::vector<String>> getImportList(std::vector<lexer::Token>);
/**
 * @brief get a symbol list for an import. Meta-function. Prefer not to use.
*/





