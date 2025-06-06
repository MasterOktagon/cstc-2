#pragma once
#define share std::shared_ptr
#define unify std::unique_ptr

#include <cstdint>
#include <memory>

#include "../lib/segvcatch/lib/segvcatch.h"
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float float32;
typedef double float64;
typedef long double float80;

typedef std::size_t size;

#define sptr std::shared_ptr
#define uptr std::unique_ptr
#define Exception std::exception

#define abstract =0
#define nlambda []
#define lambda [&]

typedef std::string String;
typedef std::wstring WString;
typedef String LLType;
typedef String CstType;
typedef wchar_t wchar;

#include <string>
#include <vector>

extern String operator ""s (const char* a, size);

extern String fillup(String s, uint64 len, char with=' ');
template <typename T>
std::vector<T> subvector(std::vector<T> v, int start, int, int stop){
    auto s = v.begin() + start; auto end = v.begin() + stop;
    return std::vector<T>(s, end);
}

class Repr{
    friend String str(Repr*);

    public:
        Repr(){};
        virtual ~Repr(){};

    protected:
        virtual String _str(){return "Repr"s;};
};

#include <sstream>
template< typename T >
String int_to_hex( T i ){
  std::stringstream stream;
  stream << "0x"
         << std::hex << i;
  return stream.str();
}


inline String str(Repr* r){
    return r->_str();
}
inline String strp(Repr* r){
    return str(r) + " @ "s + int_to_hex((size) r);
}
inline String str(int32 i) {
    return std::to_string(i);
}
inline String str(int64 i) {
    return std::to_string(i);
}
inline String str(uint32 i) {
    return std::to_string(i);
}
inline String str(uint64 i) {
    return std::to_string(i);
}

inline String str(float32 i) {
    return std::to_string(i);
}
inline String str(float80 i) {
    return std::to_string(i);
}

class FPEException : public Exception {
    public:
    FPEException()= default;
    const char* what() const noexcept {
        //spdlog::critical("Zero Division Error!");
        return "\e[1;31mFATAL:\e[0m Zero division Error";
    }
};

class SegFException : public Exception {
    public:
    SegFException()= default;
    const char* what() const noexcept {
        //spdlog::critical("Access denied! (Segmentation fault)");
        return "\e[1;31mFATAL:\e[0m Access denied! (Segmentation fault)";
    }
};

#define instanceOf(el,of) ((of*) el.get() == dynamic_cast<of*>(el.get()))


