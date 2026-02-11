#include "dog.hpp"

/*BEGIN SOLUTION*/
Dog::Dog(const std::string& name, double weight) : Mammal::Mammal(weight), name_(name) { }
    
const std::string& Dog::GetName() const {
    return name_;
}

const std::string& Dog::MakeSound() {
    return sound_;
}

std::string Dog::sound_ = "Wuff!";
/*END SOLUTION*/
