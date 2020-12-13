//
// Created by jwscoggins on 12/12/20.
//

#ifndef SIM960_GENERICNUMBER_H
#define SIM960_GENERICNUMBER_H
#include <type_traits>

template<typename T, std::size_t width>
class GenericNumber {
public:
    GenericNumber(T val = static_cast<T>(0)) : _val(val) { }
    [[nodiscard]] [[maybe_unused]] constexpr auto get() const noexcept { return _val; }
    [[maybe_unused]] void set(T value) noexcept { _val = value; }
private:
    T _val : width;
};
template<typename T, std::size_t width>
bool
operator==(const GenericNumber<T, width>& a, const GenericNumber<T, width>& b) noexcept {
    return  a.get() == b.get();
}

template<typename T, std::size_t width>
bool
operator!=(const GenericNumber<T, width>& a, const GenericNumber<T, width>& b) noexcept {
    return  a.get() != b.get();
}

template<typename T, std::size_t width>
GenericNumber<T, width>
operator+(const GenericNumber<T, width>& a, const GenericNumber<T, width>& b) noexcept {
    return GenericNumber<T, width>(a.get() + b.get());
}
template<typename T, std::size_t width>
GenericNumber<T, width>
operator-(const GenericNumber<T, width>& a, const GenericNumber<T, width>& b) noexcept {
    return GenericNumber<T, width>(a.get() - b.get());
}
template<typename T, std::size_t width>
GenericNumber<T, width>
operator*(const GenericNumber<T, width>& a, const GenericNumber<T, width>& b) noexcept {
    return GenericNumber<T, width>(a.get() * b.get());
}

template<typename T, std::size_t width>
GenericNumber<T, width>
operator/(const GenericNumber<T, width>& a, const GenericNumber<T, width>& b) noexcept {
    return GenericNumber<T, width>(a.get() / b.get());
}
#endif //SIM960_GENERICNUMBER_H
