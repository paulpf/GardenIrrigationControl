#pragma once
// ---------------------------------------------------------------------------
// ArduinoFake.h
// Minimal host-side stubs so helper/domain code can be compiled and tested
// without an Arduino/ESP32 toolchain.
// ---------------------------------------------------------------------------

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>

// ---- Arduino String stub --------------------------------------------------
class String {
public:
    String() {}
    String(const char* s) : _s(s ? s : "") {}
    String(int  v) : _s(std::to_string(v)) {}
    String(long v) : _s(std::to_string(v)) {}
    String(unsigned long v) : _s(std::to_string(v)) {}
    String(float  v, int dec = 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", dec, (double)v);
        _s = buf;
    }
    String(double v, int dec = 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", dec, v);
        _s = buf;
    }

    const char* c_str()    const { return _s.c_str(); }
    size_t      length()   const { return _s.size(); }
    bool        isEmpty()  const { return _s.empty(); }

    String operator+(const String& o) const { return String((_s + o._s).c_str()); }
    String operator+(const char* o)   const { return String((_s + o).c_str()); }
    String& operator+=(const String& o)     { _s += o._s; return *this; }
    String& operator+=(const char* o)       { _s += o;    return *this; }
    bool operator==(const String& o) const  { return _s == o._s; }
    bool operator==(const char*   o) const  { return _s == o; }
    bool operator!=(const String& o) const  { return !(*this == o); }

    bool startsWith(const String& prefix) const {
        return _s.rfind(prefix._s, 0) == 0;
    }

    int toInt() const { return std::stoi(_s); }

    void replace(char from, char to) {
        for (auto& c : _s) if (c == from) c = to;
    }

private:
    std::string _s;
};

inline String operator+(const char* lhs, const String& rhs) {
    return String((std::string(lhs) + rhs.c_str()).c_str());
}
