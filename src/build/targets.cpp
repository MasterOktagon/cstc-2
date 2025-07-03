
#include "targets.hpp"
#include "../snippets.h"
#include <iostream>

void target::list() {
    std::cout << "\e[1;36mINFO: Available targets:\e[0m" << std::endl << std::endl;
    std::cout << "\t\e[1;31m[BE]\e[0m - Bleeding Edge\t\e[1;33m[Beta]\e[0m - Beta" << std::endl << std::endl;

    for (String target : targets) {
        std::cout << "\t" << fillup(target, 30) << "\e[1;31m[BE]\e[0m" << std::endl; // TODO: change this
    }

}
