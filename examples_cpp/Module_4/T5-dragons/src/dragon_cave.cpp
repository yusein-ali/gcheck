#include "dragon_cave.hpp"

// Define DragonCave's methods here
/*BEGIN SOLUTION*/
DragonCave::DragonCave() { }
   
DragonCave::~DragonCave() { 
    for (auto it : dragons_)
        delete it;
}
    
const std::list<Dragon*>& DragonCave::GetDragons() const {
    return dragons_;
}
    
void DragonCave::Accommodate(Dragon* dragon) {
    dragons_.push_back(dragon);
}
   
void DragonCave::Evict(const std::string& name) {
    for (auto it = dragons_.begin(); it != dragons_.end(); it++)
        if((*it)->GetName() == name) {
            dragons_.erase(it);
            break;
        }
}

std::ostream& operator<<(std::ostream& os, const DragonCave& cave) {
    os << "DragonCave dwellers:" << std::endl;
    for (auto& it : cave.GetDragons())
        os << std::endl << *it;
    
    return os.flush();
}
/*END SOLUTION*/