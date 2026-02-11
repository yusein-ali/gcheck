/*BEGIN SOLUTION*/
#include "printers.hpp"

#include <stdexcept>

Printers::Printers() { }

Printers::Printers(const Printers& other) {
    *this = other;
}

Printers::~Printers() {
    
    for (std::map<std::string, StringPrinter*>::iterator iter = index_.begin();
	        iter != index_.end(); iter++)
        delete iter->second;
}

void Printers::Add(const std::string& name, StringPrinter* printer) {
    
    if (!printer)
	    throw std::invalid_argument("Invalid printer");
        
    if (index_.find(name) != index_.end())
	    throw std::invalid_argument("Duplicate index");
        
    index_[name] = printer;
}

StringPrinter& Printers::operator[](const std::string& name) {
    
    if (index_.find(name) == index_.end())
	    throw std::invalid_argument("Unknown index");
        
    return *index_[name];
}

Printers& Printers::operator=(const Printers& other) {
    
    for (auto iter = index_.begin(); iter != index_.end(); iter++)
        delete iter->second;
    
    index_.clear();
    
    for (auto iter = other.index_.begin(); iter != other.index_.end(); iter++)
	    index_[iter->first] = iter->second->Clone();
        
    return *this;
}
/*END SOLUTION*/
