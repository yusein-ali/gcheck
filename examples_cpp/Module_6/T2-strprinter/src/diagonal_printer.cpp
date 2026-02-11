// BEGIN SOLUTION
#include "diagonal_printer.hpp"

DiagonalPrinter::DiagonalPrinter(const std::string& first, const std::string& last,
        std::ostream& os) : StringPrinter(os), first_(first), last_(last) { }

DiagonalPrinter* DiagonalPrinter::Clone() const {
    return new DiagonalPrinter(first_, last_, os_);
}

DiagonalPrinter& DiagonalPrinter::operator()(const std::string& str) {
    if (first_.size())
        os_ << first_ << std::endl;
        
    for(size_t i = 0; i < str.size(); i++) {
        for (size_t j = 0; j < str.size(); j++)
            os_ << ((i == j) ? str[i] : ' ');
            
        os_ << std::endl;
    }
    
    if (last_.size())
        os_ << last_ << std::endl;
        
    return *this;
}
// END SOLUTION