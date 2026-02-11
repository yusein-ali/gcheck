/* Add include guards here */

/**
 * @brief A class that is used to print strings diagonally (see below). The
 * class inherits (public inheritance) class StringPrinter.
 * 
 * TODO: Implement the following:
 * ------------
 * Functions:
 * 
 * A constructor that takes three parameters:
 *  - The first line, which is "" by default, i.e. it is not printed at all
 *  - The last line, which is "" by default, i.e. it is not printed at all either
 *  - A reference to the output stream, which is std::cout by default
 * 
 * Overrides the necessary members from the base class.
 * ------------
 * Other:
 * DiagonalPrinter prints a string diagonally. For the string "like this":
<first line>\n
l        \n
 i       \n
  k      \n
   e     \n
         \n
     t   \n
      h  \n
       i \n
        s\n
<last line>\n
 * where <first line> and <last line> are replaced by the strings given in the
 * constructor. If the first line is empty, it is not printed. The same applies
 * to the last line.
 */

/*BEGIN SOLUTION*/
#ifndef AALTO_ELEC_CPP_DIAGONAL_CLASS
#define AALTO_ELEC_CPP_DIAGONAL_CLASS

#include "string_printer.hpp"

/**
 * @brief A class that is used to print strings diagonally (see below). The
 * class inherits (public inheritance) class StringPrinter.
 */
class DiagonalPrinter : public StringPrinter {
public:

    /**
     * @brief Construct a new Diagonal Printer object
     * 
     * @param first The first line, which is "" by default, i.e. it is not printed at all
     * @param last The last line, which is "" by default, i.e. it is not printed at all either
     * @param os A reference to the output stream, which is std::cout by default
     */
    DiagonalPrinter(const std::string& first = "", const std::string& last = "",
            std::ostream& os = std::cout);

    /**
     * @brief 
     * 
     * @return DiagonalPrinter* 
     */
    DiagonalPrinter* Clone() const;
    DiagonalPrinter& operator()(const std::string& str);
private:
    std::string first_;
    std::string last_;
};

#endif
/*END SOLUTION*/