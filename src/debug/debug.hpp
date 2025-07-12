#pragma once

/**
 * @file that has some debug macros
*/

#define DBG_LVL 5

#ifdef DEBUG_ON
#include <iostream>
#define DEBUG(lvl, msg)                                                                                                \
    if (DBG_LVL >= lvl)                                                                                                \
        std::cout << "\e[35;1mDEBUG> \e[0m\e[35m" << lvl << "\e[1m ]\e[0m " << msg << std::endl;
#define DEBUGT(lvl, msg, tokens)                                                                                       \
    if (DBG_LVL >= lvl)                                                                                                \
        std::cout << "\e[35;1mDEBUG> \e[0m\e[35m" << lvl << "\e[1m ]\e[0m " << fillup(msg, 70) << str(tokens)          \
                  << std::endl;
#else
#define DEBUG(lvl, msg)
#define DEBUGT(lvl, msg, tokens)
#endif

