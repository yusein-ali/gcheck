
#include "user_object.h"

namespace gcheck {

void AddToUserObjectVector(std::vector<UserObject>& container, const std::string& str) {
    container.push_back(UserObject(str));
}

} // gcheck