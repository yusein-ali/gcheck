#include "cat.hpp"

/*BEGIN SOLUTION*/
Cat::Cat(const std::string& name, double weight) : Mammal::Mammal(weight), name_(name) { }

const std::string& Cat::GetName() const {
    return name_;
}

const std::string& Cat::MakeSound() {
    return sound_;
}

std::string Cat::sound_ = "Meow";
/*END SOLUTION*/
