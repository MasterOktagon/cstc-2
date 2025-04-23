#include "snippets.h"
#include "../lib/argparse/include/argparse/argparse.hpp"
#define PROGRAM_EXIT 0
#define EXIT_ARG_FAILURE 1

int32 main(int32 argc, const char** argv){
    segvcatch::init_fpe (nlambda () {throw FPEException(); });
    segvcatch::init_segv(nlambda () {throw SegFException();});

    argparse::ArgumentParser argparser("cstc"s, "c0.01"s, argparse::default_arguments::help);

    try {
        argparser.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << argparser;
        return EXIT_ARG_FAILURE;
    }

    return PROGRAM_EXIT;
}



