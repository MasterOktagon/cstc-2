//
// MAIN.cpp
//
// main porgram file
//

#include "lexer/lexer.hpp"
#include "module.hpp"
#include "parser/errors.hpp"
#include "snippets.h"
#include "../lib/argparse/include/argparse/argparse.hpp"
#include <filesystem>
#include <iostream>
#include <ostream>

// EXIT CODE NAMES
#define PROGRAM_EXIT 0
#define EXIT_ARG_FAILURE 1
#define EXIT_NO_MAIN_FILE 2

int32 main(int32 argc, const char** argv){
/**
 * @brief main function
 */
    // setup segfault catching for easier debugging
    segvcatch::init_fpe (nlambda () {throw FPEException(); });
    segvcatch::init_segv(nlambda () {throw SegFException();});

    // setup argument parsing
    argparse::ArgumentParser argparser("cstc"s, "c0.01"s, argparse::default_arguments::help);
    argparser.add_argument("file")
        .help("main file of your program")
    ;
    argparser.add_argument("-l", "--list-modules")
        .help("list all loaded modules")
        .flag()    
    ;
    argparser.add_argument("-1", "--one-error")
        .help("exit at first error")
        .flag()    
    ;
    argparser.add_argument("--version")
        .help("display version info and exit")
        .flag()    
    ;
    argparser.add_argument("--max-line-len")
        .help("maximum line length before LTL warning (-1 to disable)")
        .scan<'d', int32>()
        .default_value<int32>(100)
    ;

    // try to parse arguments
    try {
        argparser.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << argparser;
        return EXIT_ARG_FAILURE;
    }

    // check for std environment variable
    if (Module::stdLibLocation() == ""){
        std::cerr << "\e[1;33mWARNING: \e[0m\e[1mCSTC_STD\e[0m environment variable could not be found.\n"
                     "This may cause problems with std::* modules.\n" << std::endl;
    }

    if (argparser["--version"] == true){
        std::cout << "CSTC v0.01 - cst25" << std::endl;

        std::exit(0);
    }

    parser::one_error = argparser["-1"] == true;
    lexer::pretty_size = argparser.get<int32>("--max-line-len");
    if (lexer::pretty_size < -1) lexer::pretty_size = -1;

    // try to load the main file
    String main_file = argparser.get("file");
    if (!std::filesystem::exists(std::filesystem::u8path(main_file))){
        std::cout << "\e[1;31mERROR:\e[0m main file at \e[1m"s + main_file + "\e[0m not found!" << std::endl;
        return EXIT_NO_MAIN_FILE;
    }
    // Load the main module (which autoloads all necessary modules)
    std::cout << "Fetching modules: (" << 0 << "/?)";
    Module::create("lang", "", "", true);
    Module::create(main_file, std::fs::current_path().string(), "", false, {}, true, true);

    // sort modules for parsing
    Module::modules.sort(Module::loadOrder);
    std::cout << "\r\e[32mFetching modules: (" << Module::known_modules.size() << "/" << Module::known_modules.size() + Module::unknown_modules.size() << ")\e[0m" << std::endl;

    if(argparser["-l"] == true){
        // display a list of Modules loaded
        std::cout << "\e[36;1mINFO: " << Module::known_modules.size() << " Module" << (Module::known_modules.size() == 1 ? ""s : "s"s) << " loaded\e[0m" << std::endl;
        std::cout << "\t\e[36;1m[h]\e[0m - Header        \e[32;1m[m]\e[0m - Main file        \e[33;1m[s]\e[0m - STDlib        \e[31;1m[t]\e[0m - target depending        \e[1m[l]\e[0m - autoload" << std::endl << std::endl;
        for (Module* m : Module::modules){
            std::cout << "\t" << str(m) << std::endl;
        }
        for (String m : Module::unknown_modules){
            std::cout << "\t\e[31m" << fillup(m, 60) << "missing" << "\e[0m" << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "Parsing modules (" << 0 << "/" << Module::modules.size() << ")";

    for (Module* m : Module::modules){
        m->parse();
    }

    std::cout << "\r\e[32mParsing modules (" << Module::modules.size() << "/" << Module::modules.size() << ")\e[0m" << std::endl;

    return PROGRAM_EXIT;
}



