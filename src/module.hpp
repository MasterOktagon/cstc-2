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
    
/**
 * @class Module holds all information and contents of a Program module 
 */
class Module final : public symbol::Namespace {

    bool is_main_file = false;                 //> whether this is the main module
    bool is_stdlib = false;                    //> whether this is a stdlib module
    std::map<String, Module *> deps = {};      //> dependency modules
    std::vector<lexer::Token> tokens = {};     //> this module's tokens

    protected:
    /**
     * @brief get a visual representation of this Object
     */
    String _str() const;
    

    public:

    String module_name;             //> representation module name
    static std::fs::path directory; //> main program directory
    std::fs::path hst_file;         //> header location (relative)
    std::fs::path cst_file;         //> source location (relative)

    bool isHeader() const;
    bool isKnown() const;

    /**
     * @brief tokenize this module and parse for imports to include them
     */
    void preprocess();

    /**
     * @brief parse this module and create AST nodes
     */
    void parse();

    Module(String path, String dir, String name, bool is_stdlib=false, bool is_main_file=false);


    static std::map<String, Module*> known_modules; //> A map of all compile-time known modules to allow faster acces when importing
    static std::list<String> unknown_modules;       //> A map of all compile-time unknown modules to allow better error messages
    static std::list<Module*> modules;              //> A list of all modules loaded. This is used to determine the compile order 

    /**
     * @brief get the default stdlib location using the CSTC_STD environment variable
     */
    static String stdLibLocation();

    /**
     * @brief convert a path to a module name
     */
    static String path2Mod(String path);

    /**
     * @brief convert a module name to a path
     */
    static String mod2Path(String path);

    /**
     * @brief compare two modules about their load order
     *
     * @return true, if a should be loaded before b
     */
    static bool loadOrder(Module* a, Module* b);

    /**
     * @brief return an imported module. This method checks if there already is a module of this name
     * and reuses it if possible
     *
     * @param path path to check for can be a "real path" or an C* import module path: @see @param from_path
     * @param dir  directory in which the callee is. currently unused
     * @param overpath path of the callee
     * @param is_stdlib depicts if this file is a stdlib file (for looking in CSTC_STD path)
     * @param tokens tokens of the preprocessor-parser, used for error messages
     * @param is_main_file whether this is the main file. should not be set outside the main function.
     * @param from_path create this from a "real path" instead of a module
     *
     * @return Newly created Module (Pointer) if succesful or nullptr
     */
    static Module* create(String path, String dir, String overpath, bool is_stdlib,
                          std::vector<lexer::Token> tokens = {}, bool is_main_file = false, bool from_path = false);
    
};

/**
 * @brief get a symbol list for an import. Meta-function. Prefer not to use.
*/
extern std::optional<std::vector<String>> getImportList(lexer::TokenStream);






