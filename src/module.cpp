
//
// MODULE.cpp
//
// implements the module class
//


#include "lexer/errors.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/ast/ast.hpp"
#include "parser/ast/flow.hpp"
#include "parser/ast/func.hpp"
#include "parser/errors.hpp"
#include "parser/parser.hpp"
#include "parser/symboltable.hpp"
#include "snippets.h"
#include <algorithm>
#include <asm-generic/errno.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <list>
#include <map>
#include "module.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <sys/types.h>
#include <utility>
#include <vector>


std::map<String, Module*> Module::known_modules = {};  //> A map of all compile-time known modules to allow faster acces when importing
std::list<String> Module::unknown_modules = {};        //> A map of all compile-time unknown modules to allow better error messages
std::list<Module*> Module::modules = {};               //> A list of all modules loaded. This is used to determine the compile order 
std::fs::path Module::directory;                       //> Project directory

uint64 parsed_modules = 0; //> amount of parsed modules

/**
 * @brief get the default stdlib location using the CSTC_STD environment variable
 */
String Module::stdLibLocation() {
    if (const char* p = std::getenv("CSTC_STD"))
        return p;
    return "";
}

/**
 * @brief replace the ~ for the home path using the HOME environment variable
 */
inline String h(String s) {
    if (s[0] == '~'){
        return String(std::getenv("HOME")) + s.substr(1);
    }
    return s;
}

bool Module::isHeader() const {
    return std::fs::exists(hst_file);
}

bool Module::isKnown() const{
   return std::fs::exists(cst_file);
}

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
Module* Module::create(String path, String, String overpath, bool is_stdlib,
                       std::vector<lexer::Token> tokens, bool is_main_file,
                       bool from_path) {
    
    String module_name = "<unknown>";
    if (!from_path){
        path = mod2Path(path);
    }
    size pos = path.rfind(".");
    if (pos != String::npos){
        path = path.substr(0,pos);
    }
    pos = overpath.rfind(".");
    if (pos != String::npos){
        overpath = overpath.substr(0,pos);
    }

    std::fs::path directory = std::fs::current_path();
    if (is_stdlib){
        directory = std::fs::u8path(h(stdLibLocation()));
        module_name = path2Mod(path);
        module_name = "std::"s + module_name;
    }
    else {
        if (!from_path){
            directory = std::fs::u8path(overpath).parent_path();
            module_name = path2Mod(std::fs::relative(std::fs::u8path(overpath).parent_path(), std::fs::current_path()).string() + "/" + path);
        } else {
            module_name = path2Mod(path);
        }
    }

    ////std::cout << directory << std::endl;
    ////std::cout << path << std::endl;
    ////std::cout << module_name << std::endl;

    if (known_modules.count(module_name) > 0){
        return known_modules[module_name];
    }
    if (std::fs::exists(directory.string() + "/" + path + ".hst")){
        if(!std::fs::exists(directory.string() + "/" + path + ".cst")){
            lexer::warn("No implementation file found", tokens, "Missing an implementation file (\".cst\") @ " + directory.string() + "/" + path, 0);
        }
        known_modules[module_name] = new Module(path, directory.string(), module_name, is_stdlib, is_main_file);
        modules.push_back(known_modules[module_name]);
        return known_modules[module_name];
    }
    if (std::fs::exists(directory.string() + "/" + path + ".cst")){
        known_modules[module_name] = new Module(path, directory.string(), module_name, is_stdlib, is_main_file);
        modules.push_back(known_modules[module_name]);
        return known_modules[module_name];
    }
    if (std::find(unknown_modules.begin(), unknown_modules.end(), module_name) == unknown_modules.end()){
        lexer::error("Module not found", subvector(tokens, 1,1,tokens.size()-1), "A module at "s + directory.string() + "/" + path + " was not found", 3434);
        unknown_modules.push_back(module_name);
    }

    return nullptr;
}

Module::Module(String path, String dir, String module_name, bool is_stdlib, bool is_main_file){
    loc = "";
    for (uint64 i = 0; i<module_name.size(); i++){
        if (i < module_name.size()-2 && module_name[i] == ':' && module_name[i+1] == ':'){
            loc += "..";
            i++;
        }
        else {
            loc += module_name[i];
        }
    }

    this->is_main_file = is_main_file;
    this->is_stdlib = is_stdlib;
    this->module_name = module_name;
    ////std::cout << "name: " <<  this->module_name << std::endl;

    directory = std::fs::u8path(dir);
    cst_file = std::fs::u8path(dir + "/" + path + ".cst");
    hst_file = std::fs::u8path(dir + "/" + path + ".hst");

    std::cout << "\rFetching modules: (" << known_modules.size()+1 << "/?)";
    if (module_name != "std::lang" && known_modules.count("std::lang") > 0){
        include.push_back(known_modules["std::lang"]);
    }

    preprocess();
}

/**
 * @brief convert a path to a module name
 */
String Module::path2Mod(String path){
    String out;
    size pos = path.find("/");
    while (pos != String::npos){
        out += path.substr(0, pos) + "::";
        path = path.substr(pos+1);
        pos = path.find("/");
    }
    out += path;
    return out;
}

/**
 * @brief convert a module name to a path
 */
String Module::mod2Path(String mod){
    String out;

    for (uint64 i = 0; i<mod.size(); i++){
        if (i < mod.size()-2 && mod[i] == ':' && mod[i+1] == ':'){
            out += '/';
            i++;
        }
        else {
            out += mod[i];
        }
    }
    return out;
}

/**
 * @brief get a visual representation of this Object
 */
String Module::_str() const {
  return fillup("\e[1m"s + module_name + "\e[0m", 50) +
        (isHeader() ? "\e[36;1m[h]\e[0m"s : "   "s) +
        (is_main_file ? "\e[32;1m[m]\e[0m"s : "   "s) +
        (is_stdlib ? "\e[33;1m[s]\e[0m"s : "   "s) +
        (module_name == "std::lang" ? "\e[1m[l]\e[0m"s : "   "s) +
        " @ " + (cst_file.string())
    ;
}

/**
 * @brief compare two modules about their load order
 *
 * @return true, if a should be loaded before b
 */
bool Module::loadOrder(Module *a, Module *b){
    for (std::pair<String, Module*> p : a->deps){
        if (p.second == b) return false;
    }
    for (symbol::Namespace* m : a->include){
        if (m == b) return false;
    }
    return true;
}

/**
 * @brief get a symbol list for an import. Meta-function. Prefer not to use.
*/
std::optional<std::vector<String>> getImportList(lexer::TokenStream t) {
    if (t.size() == 0)
        return {};
    std::vector<String> out = {};

    lexer::Token::Type last = lexer::Token::Type::COMMA;

    for (lexer::Token tok : t.tokens) {
        if (tok.type == lexer::Token::Type::COMMA) {};
        if (tok.type == lexer::Token::Type::ID) {
            if (last == lexer::Token::Type::COMMA) {
                out.push_back(tok.value);
            } else {
                return {};
            }
        }
        else { return {}; }
        last = tok.type;
    }
    return out;
}

/**
 * @brief tokenize this module and parse for imports to include them
 */
void Module::preprocess(){
    std::ifstream f(cst_file.string());
        String content((std::istreambuf_iterator<char>(f) ),
                       (std::istreambuf_iterator<char>()));

        tokens = lexer::tokenize(content, cst_file).tokens;
        //std::cout << "\r" << module_name << ": " <<std::endl;
        //std::cout << tokens.size() << std::endl;
        //for (lexer::Token t : tokens){
        //    //std::cout << str(&t) << std::endl;
        //}

        bool at_top = true;
        lexer::Token first;
        std::vector<lexer::Token> buffer = {};
        for (lexer::Token t : tokens){
            buffer.push_back(t);
            if (t.type == lexer::Token::Type::END_CMD){
                if (buffer.at(0).type == lexer::Token::Type::IMPORT){
                    lexer::Token::Type last = lexer::Token::Type::SUBNS;
                    String new_module_name;
                    String as;
                    uint64 i2 = 0;
                    bool importall = false;
                    bool has_import = false;
                    std::vector<String> from = {};
                    for (lexer::Token a : buffer){
                        //std::cout << i2 << " " << new_module_name << std::endl;
                        if (a.type == lexer::Token::IMPORT && !has_import){
                            i2++; has_import = true;
                            continue;
                        }
                        else if (last == lexer::Token::SUBNS && (a.type == lexer::Token::DOTDOT || a.type == lexer::Token::ID)){
                            new_module_name += a.value;
                        }
                        else if (a.type == lexer::Token::SUBNS && (last == lexer::Token::DOTDOT || last == lexer::Token::ID)){
                            new_module_name += a.value;
                        }
                        else if ((last == lexer::Token::DOTDOT || last == lexer::Token::ID) && a.type == lexer::Token::Type::END_CMD){
                            break;
                        }
                        else if ((last == lexer::Token::DOTDOT || last == lexer::Token::ID) && a.type == lexer::Token::Type::AS){
                            
                            if (buffer.size() > i2+2 && buffer.at(i2+1).type == lexer::Token::Type::ID && buffer.at(i2+2).type == lexer::Token::Type::END_CMD){
                                String as = buffer.at(i2+1).value;
                                break;
                            } else {
                                goto disallowed;
                            }
                        }
                        else if ((last == lexer::Token::DOTDOT || last == lexer::Token::ID) && a.type == lexer::Token::Type::IN){
                            if (buffer.size() > i2 + 3 &&
                                buffer.at(i2 + 1).type == lexer::Token::Type::BLOCK_OPEN &&
                                buffer.at(buffer.size() - 1).type == lexer::Token::Type::END_CMD &&
                                buffer.at(buffer.size() - 2).type == lexer::Token::Type::BLOCK_CLOSE) {
                            
                                try {
                                    from = getImportList(
                                                subvector(buffer, i2 + 2, 1,
                                                        buffer.size() - 2))
                                                .value();
                                    break;
                                } catch (const std::bad_optional_access&) { // urgh!
                                    goto disallowed;
                                }
                            }
                            else {
                                goto disallowed;
                            }
                        }
                        else if (last == lexer::Token::SUBNS && a.type == lexer::Token::Type::MUL){
                            
                            if (buffer.size() > i2+1 && buffer.at(i2+1).type == lexer::Token::Type::END_CMD && new_module_name != ""){
                                importall = true;
                                new_module_name = new_module_name.substr(0, new_module_name.size()-2);
                                break;
                            } else {
                                goto disallowed;
                            }
                        }
                        else {
                            goto disallowed; // Yay, goto
                        }
                        last = a.type; i2++;
                        //std::cout << module_name << std::endl;
                    }
                    ////std::cout << new_module_name << std::endl;
                    bool is_stdlib = false;
                    if (new_module_name.size() >= 5 && new_module_name.substr(0,5) == "std::"){
                        new_module_name = new_module_name.substr(5);
                        is_stdlib = true;
                    }
                    Module* m = Module::create(new_module_name, directory.string(), cst_file, is_stdlib, buffer);
                    if (m != nullptr){
                        deps[m->module_name] = m;
                        for (String s : from) {
                            import_from[s] = m->module_name + "::" + s;
                        }
                        if (importall) include.push_back(m);
                        else if (as != "") add(as, m);
                        else contents[module_name] = {m};
                    }

                    if (!at_top && m != nullptr){
                        lexer::warn("Import not at top", buffer, "This import statement was not at the top of the module", 23);
                        lexer::note({first}, "because of this token of type "s + lexer::getTokenName(first.type), 0);
                    }
                }
                else {
                    at_top = false;
                    first = buffer.at(0);
                }
                disallowed:
                buffer = {};
            }
        }
        //std::cout << "end " << module_name << std::endl;
    f.close();
}

/**
 * @brief parse this module and create AST nodes
 */
void Module::parse(){
    sptr<AST> root = SubBlockAST::parse(tokens, 0, this);
    if (root != nullptr){
        int* i = new int;
        *i     = 0;
        //std::cout << str(root.get()) << std::endl;
        std::cout << root->emitCST() << std::endl;

        delete i;

        for (sptr<AST> a : std::dynamic_pointer_cast<SubBlockAST>(root)->contents) {
            if (instanceOf(a, FuncCallAST)) {
                auto r = std::dynamic_pointer_cast<FuncCallAST>(a);
                fsignal<void, String, std::vector<lexer::Token>, String, uint32, String> warn_error = parser::error;
                if (r->getCstType() == "void") continue;
                if (parser::isAtomic(r->getCstType())) {
                    warn_error = parser::warn;
                }
                warn_error("Type linearity violated",r->getTokens().tokens,"return values of this function are discarded",0,"");
            }
        }
    }
    // check variables for usage
    for (std::pair<String, std::vector<symbol::Reference*>> sr : contents){
        if (sr.second.at(0) == dynamic_cast<symbol::Variable*>(sr.second.at(0))){
            auto var = (symbol::Variable*)sr.second.at(0);
            // if (parser::isAtomic(var->getCstType())) continue;
            if (var->getVarName()[0] == '_') continue;
            if (var->used == symbol::Variable::PROVIDED){
                parser::warn("Unconsumed Variable", var->last, "This variable was provided, but never consumed.\nIf this was intended, prefix it with an '_'.", 0);
            }
            if (var->used == symbol::Variable::UNINITIALIZED){
                parser::warn("Unused Variable", var->tokens, "This variable was declared, but never used.\nIf this was intended, prefix it with an '_'.", 0);
            }
        }
    }

    std::cout << "\rParsing modules (" << ++parsed_modules << "/" << Module::modules.size() << ")";
}


