#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <type_traits>
#include "argument.h"

class JSON {
    std::unordered_map<std::string, std::any> items_;

public:
    static std::string AsString(const std::any& item);

    template <typename A>
    JSON& Set(const std::string& name, const A& value) {
        items_.insert_or_assign(name, value);
        return *this;
    }
    JSON& Set(const std::string& name, std::vector<std::any> value) {
        items_.insert_or_assign(name, value);
        //for(auto it = std::any_cast<std::vector<std::any>>(&items_[name])->begin(); it != std::any_cast<std::vector<std::any>>(&items_[name])->end(); ++it)
            //std::cout << JSON::AsString(*it) << " ";
        return *this;
    }
    JSON& Set(const std::string& name, const char value[]) {
        items_.insert_or_assign(name, std::string(value));
        return *this;
    }

    JSON& Set(JSON json);
    JSON& Remove(std::string key);

    bool Contains(std::string key) { return items_.find(key) != items_.end(); }
    std::any& Get(std::string key);
    std::any Get(std::string key) const;
    
    template<typename A>
    A& Get(std::string key) {
        auto out = std::any_cast<A>(&Get(key));

        if(out == NULL) {
            throw; //TODO: an actual exception
        }

        return *out;
    }

    template<typename A>
    A Get(std::string key) const {
        auto item = Get(key);
        auto out = std::any_cast<A>(&item);

        if(out == NULL) {
            throw; //TODO: an actual exception
        }

        return *out;
    }

    std::string AsString(bool strip_ends = false) const;
    std::string AsString(const char* key) const;
    std::string AsString(std::string key) const;
    static std::string Escape(std::string str);

    auto begin() { return items_.begin(); }
    auto end() { return items_.end(); }
};

template<typename T>
typename std::enable_if<is_Argument<T>::value>::type
MakeAnyList(std::vector<std::any>& container, T first) {
    container.push_back(std::any(first()));
}

template<typename T>
typename std::enable_if<!is_Argument<T>::value>::type
MakeAnyList(std::vector<std::any>& container, T first) {
    container.push_back(std::any(first));
}

template<typename T, typename... Args>
void MakeAnyList(std::vector<std::any>& container, T first, Args... rest) {
    MakeAnyList(container, first);
    MakeAnyList(container, rest...);
}

template<typename... Args>
std::vector<std::any> MakeAnyList(Args... args) {
    std::vector<std::any> out;
    MakeAnyList(out, args...);
    return out;
}
