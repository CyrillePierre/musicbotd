#pragma once

#include <utility>
#include <type_traits>
#include <iostream>

namespace util {

class Any {
    using Id = std::size_t;

    template <class T>
    struct TypeIdImpl { static void id() {} };

    struct Base {
        virtual ~Base() {}
        virtual bool   is(Id) const = 0;
        virtual Base * copy() const = 0;
    };

    template <class T>
    class Derived : public Base {
        T _inst;
    public:
        Derived(T const & t) : _inst(t) {}
        Derived(T && t) : _inst(std::move(t)) {}
        T       & get()       & { return _inst; }
        T const & get() const & { return _inst; }
        bool is(Id id) const { return id == typeId<T>(); }
        Base * copy() const { return new Derived<T>{get()}; }
    };

    Base * _data = nullptr;

    template <class T>
    static std::size_t typeId()
    { return reinterpret_cast<std::size_t>(&TypeIdImpl<T>::id); }

public:
    Any() {}
    Any(Any const & any) : _data{any._data->copy()} {}
    Any(Any && any)      : _data{any._data} { any._data = nullptr; }

    template <class T,
              class U = std::decay_t<T>,
              class = std::enable_if_t<!std::is_same<Any, U>{}>>
    Any(T && t) : _data{new Derived<U>{std::forward<T>(t)}} {}

    ~Any() { delete _data; }
    Any & operator = (Any && any) { swap(*this, any); return *this; }
    Any & operator = (Any const & any)
    { if (&any != this) delete _data, _data = any._data->copy(); }

    friend void swap(Any & a, Any & b) { std::swap(a._data, b._data); }
    bool has_value() const { return _data; }

    template <class T>
    bool is() const { return _data && _data->is(typeId<T>()); }

    template <class T>
    T & cast() { return static_cast<Derived<T> &>(*_data).get(); }

    template <class T>
    T const & cast() const { return static_cast<Derived<T> &>(*_data).get(); }

    template <class T> operator T && () && { return std::move(cast<T>()); }
    template <class T> operator T & () & { return cast<T>(); }
    template <class T> operator T const & () const & { return cast<T>(); }
};

}
