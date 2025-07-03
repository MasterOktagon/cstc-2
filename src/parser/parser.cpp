#include "parser.hpp"

#include <string>
#include <vector>
#include "../lexer/token.hpp"
#include "errors.hpp"
#include <cmath>
#include <regex>
#include "ast/ast.hpp"
#include "symboltable.hpp"
//#define DEBUG

#ifdef DEBUG
    #include <iostream>
#endif

#define INT_OPS(type)                                                       \
    if (type1 == type){                                                     \
        if (op == lexer::Token::Type::NOT) return "";                       \
        if (op == lexer::Token::Type::NEG) return type;                     \
        if(type2 == "bool"){                                                \
            if (op == lexer::Token::Type::AS) return "bool";                \
        }                                                                   \
        if(std::regex_match(type2, int_regex)){                             \
            if (op == lexer::Token::Type::AS) return type2;                 \
        }                                                                   \
        if(std::regex_match(type2, flt_regex)){                             \
            if (op == lexer::Token::Type::AS) return type2;                 \
        }                                                                   \
        if(type2 == "char"){                                                \
            if (op == lexer::Token::Type::AS) return "char";                \
        }                                                                   \
        if (type2 == type){                                                 \
            if (op == lexer::Token::Type::AND) return type;                 \
            if (op == lexer::Token::Type::OR) return type;                  \
            if (op == lexer::Token::Type::XOR) return type;                 \
            if (op == lexer::Token::Type::ADD) return type;                 \
            if (op == lexer::Token::Type::SUB) return type;                 \
            if (op == lexer::Token::Type::MUL) return type;                 \
            if (op == lexer::Token::Type::DIV) return type;                 \
            if (op == lexer::Token::Type::MOD) return type;                 \
            if (op == lexer::Token::Type::POW) return type;                 \
        } \
    }

#define FLT_OPS(type)                                                       \
    if (type1 == type){                                                     \
        if(type2 == "bool"){                                                \
            if (op == lexer::Token::Type::AS) return "bool";                \
        }                                                                   \
        if(std::regex_match(type2, int_regex)){                             \
            if (op == lexer::Token::Type::AS) return type2;                 \
        }                                                                   \
        if(std::regex_match(type2, flt_regex)){                             \
            if (op == lexer::Token::Type::AS) return type2;                 \
        }                                                                   \
        if(type2 == "char"){                                                \
            if (op == lexer::Token::Type::AS) return "char";                \
        }                                                                   \
        if (type2 == type){                                                 \
            if (op == lexer::Token::Type::ADD) return type;                 \
            if (op == lexer::Token::Type::SUB) return type;                 \
            if (op == lexer::Token::Type::MUL) return type;                 \
            if (op == lexer::Token::Type::DIV) return type;                 \
            if (op == lexer::Token::Type::MOD) return type;                 \
            if (op == lexer::Token::Type::POW) return type;                 \
        } \
    }


String parser::hasOp(String type1, String type2, lexer::Token::Type op){
    if(type1 == "@unknown" || type2 == "@unknown") return "@unknown";

    if (type2 == type1 + '?' && op == lexer::Token::Type::AS){
        return type2;
    }

    std::regex int_regex("u?int(8|16|32|64|128)");
    std::regex flt_regex("float(16|32|64|80)");
    if (type1 == "bool"){
        if (op == lexer::Token::Type::NOT) return "bool";
        if (op == lexer::Token::Type::NEG) return "";
        if(type2 == "bool"){
            if (op == lexer::Token::Type::LAND) return "bool";
            if (op == lexer::Token::Type::LOR) return "bool";
            if (op == lexer::Token::Type::NEQ) return "bool";
            if (op == lexer::Token::Type::EQ) return "bool";
            if (op == lexer::Token::Type::AND) return "bool";
            if (op == lexer::Token::Type::OR) return "bool";
            if (op == lexer::Token::Type::XOR) return "bool";

        }
        if(std::regex_match(type2, int_regex)){
            if (op == lexer::Token::Type::AS) return type2;
        }
    }
    INT_OPS("@int");
    INT_OPS("uint8");
    INT_OPS("uint16");
    INT_OPS("uint32");
    INT_OPS("uint64");
    INT_OPS("uint128");

    INT_OPS("int8");
    INT_OPS("int16");
    INT_OPS("int32");
    INT_OPS("int64");
    INT_OPS("int128");

    FLT_OPS("float16");
    FLT_OPS("float32");
    FLT_OPS("float64");
    FLT_OPS("float80");

    return "";
}


String match_token_clamp(lexer::Token::Type t){
    switch (t){
        case lexer::Token::Type::OPEN:  return "PARANTHESIS";
        case lexer::Token::Type::CLOSE: return "PARANTHESIS";

        case lexer::Token::Type::INDEX_OPEN:  return "INDEX";
        case lexer::Token::Type::INDEX_CLOSE: return "INDEX";

        case lexer::Token::Type::BLOCK_OPEN:  return "BLOCK";
        case lexer::Token::Type::BLOCK_CLOSE: return "BLOCK";

        default: return "UNKNOWN";
    }
}

uint64 parser::splitStack(std::vector<lexer::Token> t, std::initializer_list<lexer::Token::Type> delimiter, int, std::initializer_list<lexer::Token::Type>){
    std::stack<lexer::Token> s = {};

    uint64 i;
    for(i=t.size()-1; i>0 ; i--){
        if (t[i].type == lexer::Token::Type::CLOSE || t[i].type == lexer::Token::Type::INDEX_CLOSE || t[i].type == lexer::Token::Type::BLOCK_CLOSE) {
            if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end()) return i;
            s.push(t[i]);
        }

        else if (t[i].type == lexer::Token::Type::OPEN){
            if(s.size() == 0) parser::error("Unclosed PARANTHESIS", {t[i]}, "This PARANTHESIS was not closed" , 46);
            if(s.top().type != lexer::Token::Type::CLOSE) parser::error("Unopened " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not opened" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::INDEX_OPEN){
            if(s.size() == 0) parser::error("Unclosed INDEX", {t[i]}, "This INDEX was not closed" , 46);
            if(s.top().type != lexer::Token::Type::INDEX_CLOSE) parser::error("Unopened " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not opened" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::BLOCK_OPEN){
            if(s.size() == 0) parser::error("Unclosed BLOCK", {t[i]}, "This BLOCK was not closed" , 46);
            if(s.top().type != lexer::Token::Type::BLOCK_CLOSE) parser::error("Unopened " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not opened" , 47);
            s.pop();
        }
        if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end()) return i;
    
    }
    for(uint64 j=0; j<s.size(); j++){
        parser::error("Unopened " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not opened" , 47);
        s.pop();
    }
    return i;
}

uint64 parser::rsplitStack(std::vector<lexer::Token> t, std::initializer_list<lexer::Token::Type> delimiter, int, std::initializer_list<lexer::Token::Type>){
    std::stack<lexer::Token> s = {};

    uint64 i;
    for(i=0; i<t.size(); i++){
        if (t[i].type == lexer::Token::Type::OPEN || t[i].type == lexer::Token::Type::INDEX_OPEN || t[i].type == lexer::Token::Type::BLOCK_OPEN) {
            if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end()) return i;
            s.push(t[i]);
        }

        else if (t[i].type == lexer::Token::Type::CLOSE){
            if(s.size() == 0) parser::error("Unopened PARANTHESIS", {t[i]}, "This PARANTHESIS was not opened" , 46);
            if(s.top().type != lexer::Token::Type::OPEN) parser::error("Unclosed " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not closed" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::INDEX_CLOSE){
            if(s.size() == 0) parser::error("Unopened INDEX", {t[i]}, "This INDEX was not opened" , 46);
            if(s.top().type != lexer::Token::Type::INDEX_OPEN) parser::error("Unclosed " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not closed" , 47);
            s.pop();
        }

        else if (t[i].type == lexer::Token::Type::BLOCK_CLOSE){
            if(s.size() == 0) parser::error("Unopened BLOCK", {t[i]}, "This BLOCK was not opened" , 46);
            if(s.top().type != lexer::Token::Type::BLOCK_OPEN) parser::error("Unclosed " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not closed" , 47);
            s.pop();
        }
        if (s.size() == 0 && std::find(delimiter.begin(), delimiter.end(), t[i].type) != delimiter.end()) return i;
    
    }
    for(uint64 j=0; j<s.size(); j++){
        parser::error("Unclosed " + match_token_clamp(s.top().type), {s.top()}, "This " + match_token_clamp(s.top().type) + " was not closed" , 47);
        s.pop();
    }
    return i;
}

sptr<AST> parser::parseOneOf(std::vector<lexer::Token> tokens, std::vector<PARSER_FN_NO_DEFAULT> functions, int local, symbol::Namespace* sr, String expected_type){
    for (auto fn : functions){
        sptr<AST> r = fn(tokens, local, sr, expected_type);
        if (r != nullptr){
            #ifdef DEBUG
                std::cout << "parse_one_of: " << r->emitCST()  << std::endl;
            #endif
            return r;
        }
    }
    return nullptr;
}

bool parser::typeEq(String a, String b){
    if (a == "@unknown" || b == "@unknown") return true; // @unknown should ONLY be used for temporarily unknown types
    if (a == b) return true;
    if (a == "@int"){
        return (b == "int8" || b == "int16" || b == "int32" || b == "int64" || b == "int128" || b == "int256" || b == "int512" || b == "int1024" || 
            b == "uint8" || b == "uint16" || b == "uint32" || b == "uint64" || b == "uint128" || b == "uint256" || b == "uint512" || b == "uint1024" || b == "@uint");
    }
    if (a == "@float"){return b == "float32" || b == "float64";}
    if (b == "@int"){
        return (a == "int8" || a == "int16" || a == "int32" || a == "int64" || a == "int128" || a == "int256" || a == "int512" || a == "int1024" || 
            a == "uint8" || a == "uint16" || a == "uint32" || a == "uint64" || a == "uint128" || a == "uint256" || a == "uint512" || a == "uint1024" || a == "@uint");
    }
    if (a == "@uint"){
        return b == "uint8" || b == "uint16" || b == "uint32" || b == "uint64" || b == "uint128" || b == "uint256" || b == "uint512" || b == "uint1024" || b == "@int";
    }
    if (b == "@uint"){
        return a == "uint8" || a == "uint16" || a == "uint32" || a == "uint64" || a == "uint128" || a == "uint256" || a == "uint512" || a == "uint1024" || a == "@int";
    }
    if (b == "@float"){return a == "float32" || a == "float64";}
    
    return false;
}

bool parser::isAtomic(String type){
    if(type == "uint8") return true;
    if(type == "uint16") return true;
    if(type == "uint32") return true;
    if(type == "uint64") return true;
    if(type == "uint128") return true;

    if(type == "int8") return true;
    if(type == "int16") return true;
    if(type == "int32") return true;
    if(type == "int64") return true;
    if(type == "int128") return true;

    if(type == "char") return true;
    if(type == "float16") return true;
    if(type == "float32") return true;
    if(type == "float64") return true;
    if(type == "float80") return true;
    if (type[type.size() - 1] == '&') return true;
    if(type.size() > 1 && type.substr(type.size()-2) == "&!") return true;
    //if(type[type.size()-1] == '?') return true;
    return false;
}

bool parser::is_snake_case(String text){
    const std::regex rx("[a-z\\_][a-z0-9\\_]*");
    std::cmatch m;
    return std::regex_match(text.c_str(), m, rx);
}
bool parser::isPascalCase(String text){
    const std::regex rx("\\_?[a-z][a-z0-9A-Z]*");
    std::cmatch m;
    return std::regex_match(text.c_str(), m, rx);
}
bool parser::IsCamelCase(String text){
    const std::regex rx("\\_?[A-Z][a-z0-9A-Z]*");
    std::cmatch m;
    return std::regex_match(text.c_str(), m, rx);
}
bool parser::IS_UPPER_CASE(String text){
    const std::regex rx("[A-Z\\_][A-Z0-9\\_]*");
    std::cmatch m;
    return std::regex_match(text.c_str(), m, rx);
}

LLType parser::LLType(CstType name, symbol::Reference* sr){
    if (name == "int8") return "i8";
    if (name == "int16") return "i16";
    if (name == "int32") return "i32";
    if (name == "int64") return "i64";

    if (name == "uint8") return "i8";
    if (name == "uint16") return "i16";
    if (name == "uint32") return "i32";
    if (name == "uint64") return "i64";

    if (name == "float16") return "half";
    if (name == "float32") return "float";
    if (name == "float64") return "double";

    if (name == "char") return "i16";
    if (name == "bool") return "i1";

    if(name[name.size()-1] == '?') return "{ "s + LLType(name.substr(0, name.size()-1)) + " , i1 }";

    if (sr != nullptr){
        if ((*sr)[name].size() > 0){
            return (*sr)[name][0]->getLLType();
        }
    }

    return name;
}

parser::Modifier parser::getModifier(std::vector<lexer::Token>& tokens){
    Modifier m = Modifier::NONE;

    uint64 i;
    for (i=0; i<tokens.size(); i++){
        if (tokens.at(i).type == lexer::Token::Type::CONST)
            m = Modifier(m | Modifier::CONST);
        else if (tokens.at(i).type == lexer::Token::Type::MUT)
            m = Modifier(m | Modifier::MUTABLE);
        else break;
    }
    #ifdef DEBUG
        std::cout << "parser::getModifier: " << i << "/" << tokens.size() << std::endl;
    #endif
    if (i == tokens.size()) tokens = {};
    else if (tokens.size() > 0) tokens = parser::subvector(tokens, i,1,tokens.size());
    return m;
}
