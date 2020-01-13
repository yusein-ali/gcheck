
#include "user_object.h"

namespace gcheck {
    
UserObject::UserObject(const std::vector<UserObject>& cont) {
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
    as_json_ = toJSON(cont);
}

void AddToUserObjectVector(std::vector<UserObject>& container, const std::string& str) {
    container.push_back(UserObject(str));
}

} // gcheck