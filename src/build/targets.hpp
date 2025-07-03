#pragma once
#include "../snippets.h"
#include <vector>

namespace target {

    const std::vector<String> targets = {
        "linux:x86:64:llvm",
        "rv32i:as",
    };

    /**
     * @brief ckecks if a target is a valid target
    */
    bool isValid(String name);

    /**
     * @brief lists all available targets on the command line
    */
    void list();
}




