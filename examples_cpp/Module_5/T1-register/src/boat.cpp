#include "boat.hpp"
//TODO: implement Boat's members
//BEGIN SOLUTION
Boat::Boat(std::string register_number, std::string owner, std::string name, double draft, double power)
    : Vehicle(register_number, owner),
    name_(name),
    draft_(draft),
    power_(power) {
}

void Boat::Write(std::ostream& stream) {
    stream << "B;"
            << GetRegisterNumber() << ";"
            << GetOwner() << ";"
            << name_ << ";"
            << draft_ << ";"
            << power_ << std::endl;
}

void Boat::Print() {
    Write(std::cout);
}

Boat* Boat::Read(std::istream& stream) {
    std::string type, register_number, owner, name, draft, power;

    std::getline(stream, type, ';');
    if(type != "B")
        return nullptr;
    
    std::getline(stream, register_number, ';');
    std::getline(stream, owner, ';');
    std::getline(stream, name, ';');
    std::getline(stream, draft, ';');
    std::getline(stream, power, '\n');

    return new Boat(register_number, owner, name, std::stod(draft), std::stod(power));
}
//END SOLUTION