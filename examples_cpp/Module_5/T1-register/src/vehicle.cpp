#include "vehicle.hpp"
//TODO: implement Vehicle's members
//BEGIN SOLUTION
Vehicle::Vehicle(std::string register_number, std::string owner)
    : register_number_(register_number), owner_(owner) {
}

std::string Vehicle::GetRegisterNumber() const {
    return register_number_;
}

std::string Vehicle::GetOwner() const {
    return owner_;
}
//END SOLUTION
