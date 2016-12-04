#pragma once

#include <type_traits>

namespace util {

template <class T>
class Optional {
    T _t;
    bool _hasValue;

public:
    Optional() : _hasValue{false} {}
    Optional(T const & t) : _t{t}, _hasValue{true} {}
    Optional(T && t) : _t{t}, _hasValue{true} {}

    T       && get()       && { return std::move(_t); }
    T       &  get()       &  { return _t; }
    T const &  get() const &  { return _t; }

    operator bool () const { return _hasValue; }
    operator T () { return _t; }
    T * operator * () { return &_t; }
    T * operator -> () { return &_t; }
};

template <class T>
Optional<typename std::decay<T>::type> makeOptional(T && t) {
    return Optional<typename std::decay<T>::type>{std::forward<T>(t)};
}

}
