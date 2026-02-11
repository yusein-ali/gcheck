#include "dragon.hpp"

/*BEGIN SOLUTION*/
Dragon::Dragon(const std::string& name, size_t age, size_t size) : name_(name), age_(age), size_(size) { }
Dragon::~Dragon() { }
    
const std::string& Dragon::GetName() const {
    return name_;
}

size_t Dragon::GetAge() const {
    return age_;
}

size_t Dragon::GetSize() const {
    return size_;
}

const std::list<Treasure>& Dragon::GetTreasures() const {
    return treasures_;
}

std::ostream& operator<<(std::ostream& os, const Dragon& dragon) {
    os << "Dragon named: " << dragon.GetName() << ", age: " << dragon.GetAge() << ", size: " << dragon.GetSize() << std::endl;
    os << "Treasures:" << std::endl;
    for (auto& t : dragon.GetTreasures())
        os << t.name << std::endl;
        
    return os;
}
/*END SOLUTION*/