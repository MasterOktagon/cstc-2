#pragma once

//
// IMPORT.hpp
//
// import ingoring & debugging AST
//

#include "../../lexer/token.hpp"
#include "../symboltable.hpp"
#include "ast.hpp"

namespace ImportAST {
/**
 * @namespace that implements the Import helper parsing (W.I.P.)
 */

    extern AST* parse(std::vector<lexer::Token>, int local, symbol::Namespace* sr,
                  String expected_type = "@unknown");
    /**
     * @brief parses (and ignores) Import statements to allow debugging import statements
     */
}




