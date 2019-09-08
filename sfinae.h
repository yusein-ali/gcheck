#pragma once

namespace gcheck {
    template<class>
    struct sfinae_bool {};
    template<class T>
    struct sfinae_true : sfinae_bool<T>, std::true_type{};
    template<class T>
    struct sfinae_false : sfinae_bool<T>, std::false_type{};
    
    // AND operator
    template<class T, class S>
    sfinae_false<T> operator*(sfinae_bool<T>, sfinae_bool<S>);
    template<class T, class S>
    sfinae_true<T> operator*(sfinae_true<T>, sfinae_true<S>);
}