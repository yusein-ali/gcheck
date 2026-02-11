#include "fantasy_dragon.hpp"

// Define FantasyDragon's methods here
// BEGIN SOLUTION
FantasyDragon::FantasyDragon(const std::string& name, size_t age, size_t size) : Dragon(name, age, size) { }

/* Fantasy dragon eats: People and PeopleFood */
void FantasyDragon::Eat(std::list<Food>& food) {
    for (auto it = food.begin(); it != food.end();)
        if(it->type == People || it->type == PeopleFood) {
            std::cout << "Fantasy dragon ate: " << it->name << std::endl;
            it = food.erase(it);
            size_++;
        } else 
            it++;
}

void FantasyDragon::Hoard(std::list<Treasure>& treasures) {
    for (auto it = treasures.begin(); it != treasures.end();)
        if(it->type == Jewellery) {
            std::cout << "Fantasy dragon received: " << it->name << std::endl;
            treasures_.push_back(*it);
            it = treasures.erase(it);
        } else 
            it++;
}
// END SOLUTION
