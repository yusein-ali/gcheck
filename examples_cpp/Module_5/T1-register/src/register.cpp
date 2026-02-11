#include "register.hpp"
//TODO: implement Register's members
//BEGIN SOLUTION
#include <fstream>
#include <iostream>
#include <limits>

#include "aircraft.hpp"
#include "boat.hpp"

Register::~Register() {
    for(auto vehicle : vehicles_)
        delete vehicle;
}

void Register::Add(Vehicle* v) {
    vehicles_.push_back(v);
}

void Register::Save(const std::string& filename) const {
    std::ofstream file(filename);
    for(auto vehicle : vehicles_)
        vehicle->Write(file);
    file.close();
}

void Register::Print() {
    for(auto vehicle : vehicles_)
        vehicle->Print();
}

bool Register::ReadLine(std::istream& stream) {
    Vehicle* v = nullptr;
    char type = stream.peek();
    switch(type) {
    case 'A':
        v = Aircraft::Read(stream);
        break;
    case 'B':
        v = Boat::Read(stream);
        break;
    }
    
    if(v != nullptr) {
        Add(v);
        return true;
    }
    
    stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return false;
}

int Register::Load(const std::string& filename) {
    std::ifstream file(filename);
    if(!file.is_open())
        return -1;

    int read = 0;
    while(file.good()) 
        if(ReadLine(file))
            read++;

    file.close();

    return read;
}

size_t Register::Size() const {
    return vehicles_.size();
}
//END SOLUTION
