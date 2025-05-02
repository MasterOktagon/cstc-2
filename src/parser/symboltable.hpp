#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>
#include "../snippets.h"
#include "../lexer/lexer.hpp"

template<typename T, typename S>
using MultiMap = std::map<T, std::vector<S>>;

namespace symbol {

    class Reference : public Repr {
    
        Reference* parent = nullptr;

        protected:
        String loc;
        virtual String _str(){
            return "symbol::Reference"s;
        }

        public:
        std::vector<lexer::Token> tokens = {};

        virtual ~Reference();
        virtual LLType getLLType(){return "void"s;}
        virtual CstType getCstType(){return "void"s;}
        virtual String getLoc() const {
            if (parent == nullptr) return loc;
            return parent->getLoc() + "::"s + loc;
        }
        virtual String getLLLoc() const {
            if (parent == nullptr) return ""s;
            return parent->getLoc() + ".."s;
        }

        virtual void add(String loc, Reference* sr) abstract;
        virtual size sizeBytes(){return 0;}
    };

    class Variable : public Reference {
        /**
         * Reference that holds data about a Variable
         */

        CstType type;
        String name;

        public:
        bool isConst = false;
        bool isFinal = false;
        Variable(const String& name, LLType type, std::vector<lexer::Token> tokens) : type(std::move(type)), name(name)
            {loc = name; this->tokens = std::move(tokens);}
        virtual ~Variable(){};
        virtual String _str() {
            return "symbol::Variable "s + getLoc();
        }
        virtual LLType getLLType(){return "void"s;}
    };

    class Function : public Reference {
        /**
         * Reference that represents a function
         */

        String name;
        CstType type;
        std::vector<CstType> parameters;
        std::vector<CstType> name_parameters;
        bool is_method = false;

        protected:
        virtual String _str() {
            return "symbol::Function "s + getLoc();
        }
        public:
        LLType getLLType(){return "void"s;}
        String getLLName(){return getLLType() + (is_method ? "mthd."s : "fn."s) + name;}
        CstType getCstType();

        virtual ~Function(){};
        virtual size sizeBytes(){return 8;}
    };
    
    class Namespace : public Reference {
        /**
         * Reference that can hold other references
        */

        protected:
        MultiMap<String, Reference*> contents = {};
        virtual String _str(){
            return "symbol::Namespace "s + getLoc();
        }
        public:
        virtual void add(String loc, Reference* sr);
        Namespace() = default;
        virtual ~Namespace();

        bool ALLOWS_VAR_DECL     = false;
        // whether to allow variable declaration
        bool ALLOWS_VAR_SET      = false;
        // whether to allow variable setting
        bool ALLOWS_VISIBILITY   = false;
        // whether to allow variable visibility (public, private etc.)
        bool ALLOWS_VIRTUAL     = false;
        // whether to allow virtual variables & functions
        bool ALLOWS_EXPRESSIONS = false;
        // whether to allow expressions without body
        bool ALLOWS_FUNCTIONS   = false;
        // whether to allow function definitions
        bool ALLOWS_SUBBLOCKS   = false;
        // whether to allow subblocks
        bool ALLOWS_SUBCLASSES  = false;
        bool ALLOWS_INIT        = false;
        bool ALLOWS_INIT_CONST  = false;
        bool ALLOWS_CONST       = false;
        bool ALLOWS_STATIC      = false;
        bool ALLOWS_FINAL       = false;
        bool ALLOWS_ENUMS       = false;

        virtual std::vector<symbol::Reference*> operator [] (String subloc);
    };

    class Opaque {
        public:
            virtual ~Opaque(){}

        private:
            virtual String opaqueDecl() abstract;
    };

    class Struct : public Namespace, public Opaque {
        /**
         * Reference that holds data for a composite type
         */

        protected:
        virtual String _str() {
            return "symbol::Struct "s + getLoc();
        }
        public:
        virtual LLType getLLType() {return "@"s + getLLLoc() + "struct." + loc;}
        virtual String opaqueDecl() {
            return getLLType() + " = type opaque";
        }

        virtual ~Struct(){}
        Struct(const String& name, std::vector<lexer::Token> tokens) {
            loc = name;
            this->tokens = std::move(tokens);

            ALLOWS_INIT_CONST  = true;
            ALLOWS_VAR_DECL    = true;
            ALLOWS_STATIC      = true;
            ALLOWS_FINAL       = true;
        }

        size sizeBytes() {
            size s = 0;
            for (std::pair<String, std::vector<Reference*>> rs : contents) {
                for (Reference* r : rs.second) {
                    s += r->sizeBytes();
                }
            }
            return s;
        }
    };
}





