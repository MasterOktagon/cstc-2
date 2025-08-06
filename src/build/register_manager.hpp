#pragma once

//
// REGISTER_MANAGER.hpp
//
// layouts the register manager
//

#include "../snippets.h"
#include <cstdint>
#include <vector>

class RegisterState {
    uint32 id = 0; //> if you have more than 2^32 registers you may have to much money
    fsignal<String, uint32> naming_scheme = nullptr; //> used to convert a register into a name
    
};

class RegisterManager {

};

