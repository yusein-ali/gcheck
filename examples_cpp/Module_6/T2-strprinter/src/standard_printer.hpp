/* Add include guards here */

/**
 * @brief: A minimal StringPrinter implementation with no special functionality.
 * 
 * TODO: Implement the following:
 * ------------
 * Functions:
 * A constructor that takes a single parameter:
 *  - A reference to the output stream, which is std::cout by default
 * 
 * Overrides the necessary members from the base class.
 * ------------
 * Other:
 * StandardPrinter prints a string like one normally would but with a 
 * newline at the end. For the string "like this":
 * like this\n
*/
// BEGIN SOLUTION
#ifndef AALTO_ELEC_CPP_STANDARD_PRINTER_CLASS
#define AALTO_ELEC_CPP_STANDARD_PRINTER_CLASS

#include "string_printer.hpp"

class StandardPrinter : public StringPrinter {
public:
    StandardPrinter(std::ostream& os = std::cout);
    
    StandardPrinter* Clone() const;
    virtual StringPrinter& operator()(const std::string& str);
private:
};

#endif
// END SOLUTION
