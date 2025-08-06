#pragma once
#include "../parser/symboltable.hpp"
#include "../snippets.h"

#include <vector>

namespace target {

    extern symbol::Function*   entry;
    extern std::vector<String> target;

    const std::vector<String> targets = {
        "linux:x86:64:llvm",
        "rv32i:as",
    };

    /**
     * @brief ckecks if a target is a valid target
     */
    extern bool isValid(String name);

    /**
     * @brief ckecks if a (sub)target is active
     *
     * when target is "linux:x86:64:llvm" all of these are active: <br>
     *     linux
     *     linux:x86
     *     linux:x86:64
     *     linux:x86:64:llvm
     *     llvm
     */
    extern bool is(String subtarget);

    /**
     * @brief set a target. @see @function is
     */
    extern void set(String t);

    /**
     * @brief lists all available targets on the command line
     */
    extern void list();
} // namespace target

