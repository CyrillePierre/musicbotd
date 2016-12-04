#include <iostream>
#include <string>
#include <cassert>
#include <util/any.hpp>

int main() {
    int i = 12;
    std::string s = "cheval";
    float f = 3.14;

    util::Any a1{i};
    util::Any a2 = s;
    util::Any a3 = std::move(f);
    util::Any a4 = a2;
    util::Any a5;
    util::Any a6 = "mouton";
    util::Any a7 = 8;
    util::Any a8;

    a5 = a1;

    // has_value()
    assert(a1.has_value());
    assert(a5.has_value());
    assert(!a8.has_value());

    a8 = a1;
    assert(a8.has_value());

    // is()
    assert(a1.is<int>());
    assert(!a1.is<std::string>());
    assert(!a1.is<short>());
    assert(a2.is<std::string>());
    assert(!a2.is<char const *>());
    assert(a3.is<float>());
    assert(a4.is<std::string>());
    assert(a5.is<int>());
    assert(a6.is<char const *>());
    assert(a7.is<int>());

    // cast
    assert(a1.cast<int>() == i);
    assert(static_cast<int>(a7) == 8);
    assert(3.14f == (float)a3);
    assert((int)a5 == (int)a1);
}
