#pragma once
#include <type_traits>
#include <cstdint>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <utility>

/**
 * @file print_ip.hpp
 * @brief Перегруженные шаблонные функции print_ip с SFINAE.
 *
 * Варианты:
 * 1) Целочисленные типы — печать по байтам (MSB-LSB) в беззнаковом виде.
 * 2) Строка — печатается как есть.
 * 3) Контейнеры std::vector / std::list — элементы через '.'.
 * 4) (Опционально) std::tuple, если все типы одинаковы — элементы через '.'.
 */

template <class T> struct is_vector : std::false_type {};
template <class... Args> struct is_vector<std::vector<Args...>> : std::true_type {};
template <class T> inline constexpr bool is_vector_v = is_vector<std::decay_t<T>>::value;

template <class T> struct is_list : std::false_type {};
template <class... Args> struct is_list<std::list<Args...>> : std::true_type {};
template <class T> inline constexpr bool is_list_v = is_list<std::decay_t<T>>::value;

template <class...> struct are_same : std::true_type {};
template <class T, class U, class... R>
struct are_same<T, U, R...> : std::bool_constant<std::is_same_v<T, U> && are_same<U, R...>::value> {};
template <class... Ts>
inline constexpr bool are_same_v = are_same<Ts...>::value;

// 1) интегральные типы

/**
 * @brief Печать "IP" для любого целочисленного типа (кроме bool).
 * Вывод по байтам, беззнаково, со старшего к младшему, через '.'.
 * @tparam T целочисленный тип
 * @param value значение
 * @code
 * print_ip(int32_t{0x7F000001}); // 127.0.0.1
 * @endcode
 */
template <class T,
          std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
void print_ip(T value) {
    using U = std::make_unsigned_t<T>;
    U u = static_cast<U>(value);
    constexpr std::size_t B = sizeof(U);
    for (std::size_t i = 0; i < B; ++i) {
        std::size_t shift = 8 * (B - 1 - i);
        auto byte = static_cast<unsigned>((u >> shift) & static_cast<U>(0xFF));
        if (i) std::cout << '.';
        std::cout << byte;
    }
    std::cout << '\n';
}

// 2) строки 

/**
 * @brief Строка печатается как есть.
 */
inline void print_ip(const std::string& s) {
    std::cout << s << '\n';
}

// 3) контейнеры vector / list 

/**
 * @brief Печать содержимого std::vector или std::list через '.'.
 * Элементы выводятся как есть.
 */
template <class C,
          std::enable_if_t<is_vector_v<C> || is_list_v<C>, int> = 0>
void print_ip(const C& cont) {
    bool first = true;
    for (const auto& x : cont) {
        if (!first) std::cout << '.';
        first = false;
        std::cout << x;
    }
    std::cout << '\n';
}

// 4) std::tuple (все типы одинаковы) 

namespace detail {
    template <class Tuple, std::size_t... I>
    void print_tuple_impl(const Tuple& t, std::index_sequence<I...>) {
        bool first = true;
        auto print_one = [&](const auto& v) {
            if (!first) std::cout << '.';
            first = false;
            std::cout << v;
        };
        (print_one(std::get<I>(t)), ...);
        std::cout << '\n';
    }
}

/**
 * @brief Разрешённая перегрузка для std::tuple, если все типы равны.
 */
template <class... Ts,
          std::enable_if_t<(sizeof...(Ts) > 0) && are_same_v<Ts...>, int> = 0>
void print_ip(const std::tuple<Ts...>& t) {
    detail::print_tuple_impl(t, std::make_index_sequence<sizeof...(Ts)>{});
}

/**
 * @brief Запрещённая перегрузка: гетерогенный tuple - ошибка компиляции.
 */
template <class... Ts,
          std::enable_if_t<!((sizeof...(Ts) > 0) && are_same_v<Ts...>), int> = 0>
void print_ip(const std::tuple<Ts...>&) = delete;
