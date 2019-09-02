
#include "user_object.h"

namespace gcheck {
    
UserObject::UserObject(std::any item) {
    SetString(item);
    any_ = item;
}

UserObject::UserObject(std::vector<UserObject> cont) {
    as_string_ = "[";
    for(auto it = cont.begin(); it != cont.end();) {
        as_string_ += it->as_string_;
        if(++it != cont.end()) {
            as_string_ += ",";
        } else {
            break;
        }
    }
    as_string_ += "]";
    any_ = std::make_any<std::vector<UserObject>>(cont);
}

UserObject MakeUserObject(const char* v) {
    return UserObject(std::string(v));
}

UserObject MakeUserObject(const UserObject& v) {
    return v;
}

UserObject MakeUserObject(const std::vector<UserObject>& v) {
    return UserObject(v);
}

void AddToUserObjectList(std::vector<UserObject>& container, const std::string& str) {
    container.push_back(MakeUserObject(str));
}

template UserObject MakeUserObject(const int& v);
template UserObject MakeUserObject(const float& v);
template UserObject MakeUserObject(const double& v);
template UserObject MakeUserObject(const std::string& v);
template UserObject MakeUserObject(const std::vector<std::string>& v);

}