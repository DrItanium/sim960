//
// Created by jwscoggins on 12/12/20.
//

#ifndef SIM960_GENERICNUMBER_H
#define SIM960_GENERICNUMBER_H
#include <type_traits>

template<typename T, size_t width>
class GenericNumber {
public:
    GenericNumber(T val = static_cast<T>(0)) : _val(val) { }
    [[nodiscard]] constexpr auto get() const noexcept { return _val; }
    void set(T value) noexcept { _val = value; }
private:
    T _val : width;
};
#endif //SIM960_GENERICNUMBER_H
