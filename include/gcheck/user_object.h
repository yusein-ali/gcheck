#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <sstream>
#include <utility>

#include "argument.h"
#include "json.h"
#include "sfinae.h"
#include "stringify.h"

namespace gcheck {
namespace {
namespace detail {
    template<class T>
    static auto has_begin(int) -> sfinae_true<decltype(std::declval<T>().begin())>;
    template<class T>
    static auto has_begin(long) -> sfinae_false<T>;
    template<class T>
    static auto has_end(int) -> sfinae_true<decltype(std::declval<T>().end())>;
    template<class T>
    static auto has_end(long) -> sfinae_false<T>;
} // detail

template<class T>
struct has_begin_end : decltype(detail::has_begin<T>(0) * detail::has_end<T>(0)){};

} // anonymous

/*
    Wrapper class for anything passed by users from tests.
    Includes a descriptor string constructed using operator std::string, to_string, std::to_string or "", 
        in that order by first available method.
 */
class UserObject {
public:
    UserObject() {}
    UserObject(const UserObject& v) = default;
    
    template<typename T>
    UserObject(const T& item) {
        as_json_ = item;
        as_string_ = toString(item);
        construct_ = toConstruct(item);
    }
    template<typename... Args>
    UserObject(const Args&... items) : UserObject(std::tuple<Args...>(items...)) {}
    
    JSON json() const { return as_json_; }
    std::string string() const { return as_string_; }
    std::string construct() const { return construct_; }
    
    template<typename T>
    UserObject& operator=(const T& v) {
        return *this = UserObject(v); 
    }
private:
    std::string as_string_;
    JSON as_json_;
    std::string construct_; // a string representation on how to construct the object e.g. std::vector<int>({0, 1, 2})
};

} // gcheck