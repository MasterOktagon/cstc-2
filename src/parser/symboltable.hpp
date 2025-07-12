#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>
#include "../snippets.h"
#include "../lexer/token.hpp"

template<typename T, typename S>
using MultiMap = std::map<T, std::vector<S>>;

namespace symbol {
    class Namespace;
    class Reference : public Repr {
    
        protected:
        String loc;
        virtual String _str() const {
            return "symbol::Reference"s;
        }

        public:
        std::vector<lexer::Token> tokens = {};
        std::vector<lexer::Token> last = {};
        Reference* parent = nullptr;

        virtual ~Reference();
        virtual LLType getLLType(){return "void"s;}
        virtual CstType getCstType(){return "void"s;}
        String getLoc() const {
            if (parent == nullptr) return loc;
            return parent->getLoc() + "::"s + loc;
        }
        String getLLLoc() const {
            if (parent == nullptr) return "%"s + loc;
            return parent->getLLLoc() + ".."s + loc;
        }
        virtual String getRawLoc(){return loc;}

        virtual void add(String loc, Reference* sr) abstract;
        virtual size sizeBytes(){return 0;}

        virtual const String getName() const abstract;

        virtual std::vector<symbol::Reference*> operator [] (String) {return {};}
    };

    class Variable : public Reference {
        /**
         * Reference that holds data about a Variable
         */

        CstType type;
        String  name;

        public:

        enum Status {
            UNINITIALIZED = 0,
            PROVIDED = 1,
            CONSUMED = 2,
        };

        bool isConst   = false;
        bool isMutable = false;
        Status used = UNINITIALIZED;
        bool   isFree  = false;
        String  const_value;
        Variable(String name, LLType type, std::vector<lexer::Token> tokens, symbol::Reference* parent)
            {loc = name; this->tokens = tokens; last=tokens; this->parent=parent; this->name=name; this->type=type;}
        virtual ~Variable(){};
        virtual String _str() const { return "symbol::Variable "s + getLoc(); }
        String getVarName() const { return name;}
        virtual LLType getLLType(){return "void"s;}
        virtual void add(String, Reference*){}
        virtual CstType getCstType(){return type;}
        const String getName() const {return "Variable";};
    };

    class Function : public Reference {
        /**
         * Reference that represents a function
         */

        String name;
        CstType type;

        protected:
        virtual String _str() const {
            return "symbol::Function "s + getLoc();
        }
        public:
        std::vector<CstType> parameters;
        std::vector<CstType> name_parameters;
        bool is_method = false;
        LLType getLLType(){return "void"s;}
        String getLLName(){return getLLType() + (is_method ? "mthd."s : "fn."s) + name;}
        CstType getCstType();

        virtual ~Function(){};
        virtual size sizeBytes(){return 8;}
        const String getName() const {return "Function";};
    };
    
    class Namespace : public Reference {
        /**
         * Reference that can hold other references
        */

        protected:
          std::vector<Namespace *> include{};
          std::map<String, String> import_from = {}; //> import-from map

        virtual String _str() const {
            return "symbol::Namespace "s + getLoc();
        }
        public:
        MultiMap<String, Reference*> contents = {};
        std::vector<String> unknown_vars = {};
        virtual void add(String loc, Reference* sr);
        Namespace() = default;
        Namespace(String loc){this->loc = loc;};
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
        bool ALLOWS_SUBCLASSES  = true ;
        bool ALLOWS_INIT        = false;
        bool ALLOWS_INIT_CONST  = false;
        bool ALLOWS_CONST       = false;
        bool ALLOWS_STATIC      = false;
        bool ALLOWS_FINAL       = false;
        bool ALLOWS_ENUMS       = false;

        virtual std::vector<symbol::Reference*> operator [] (String subloc);
        const String getName() const {return "Namespace";}
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
        virtual String _str() const {
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

        const String getName() const {return "Struct";}
    };

    class EnumEntry : public Reference {
        
        virtual ~EnumEntry() = default;
        EnumEntry(String name){loc = name;}
    };

    class Enum : public Namespace {
        /**
         * Reference that holds data for an Enumeration type
         */

        protected:
        uint64 default_value = 0;
        bool has_default = false;
        uint32 bits = 8;

        public:
        bool needs_stringify = false;
        bool needs_from_string = false;
        LLType getLLType(){return "i"s + std::to_string(bits);}
        CstType getCstType(){return getLoc();}

        const String getName() const {return "Enumeration";}

        virtual ~Enum(){
            ALLOWS_ENUMS = true;
            ALLOWS_SUBCLASSES = false;
        }
        Enum(String name) {loc=name;}
    };

    class EnumGroup : public Enum {
        /**
         * Reference that holds data for an Enumeration type
         */
    };
}





