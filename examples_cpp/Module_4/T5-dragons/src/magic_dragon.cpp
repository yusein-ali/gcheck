#include "magic_dragon.hpp"

// Define MagicDragon's methods here
// BEGIN SOLUTION
MagicDragon::MagicDragon(const std::string& name, size_t age, size_t size) : Dragon(name, age, size) { }
    
void MagicDragon::Eat(std::list<Food>& food) {
    for (std::list<Food>::iterator it = food.begin(); it != food.end();)
        if(it->type == Herbs) {
            std::cout << "Magic dragon ate: " << it->name << std::endl;
            it = food.erase(it);
            size_++;
        } else 
            it++;
}

void MagicDragon::Hoard(std::list<Treasure>& treasures) {
    for (std::list<Treasure>::iterator it = treasures.begin(); it != treasures.end();)
        if(it->type == Potions) {
            std::cout << "Magic dragon received: " << it->name << std::endl;
            treasures_.push_back(*it);
            it = treasures.erase(it);
        } else 
            it++;
}
// END SOLUTION
