//BEGIN SOLUTION
#include "standard_printer.hpp"

/* Construct a new object - pass the std::ostream& to the base class */
StandardPrinter::StandardPrinter(std::ostream& os) : StringPrinter(os) { }

/* Copy this object */
StandardPrinter* StandardPrinter::Clone() const {
    return new StandardPrinter(os_);
}

StringPrinter& StandardPrinter::operator()(const std::string& str) {
    /* os is a protected member of the base class */
    os_ << str << std::endl;
    return *this;
}
//END SOLUTION