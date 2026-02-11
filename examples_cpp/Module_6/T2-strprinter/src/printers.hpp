/* Add include guards here */

/**
 * @brief: A storage for StringPrinter pointers associated with a name. The
 * class has ownership of the objects.
 * 
 * TODO: Implement the following:
 * ------------
 * Functions:
 * 
 * A default constructor that initializes the object with zero printers.
 * 
 * A copy constructor that copies the object.
 * 
 * Add: A function that adds a new StringPrinter object to the collection
 * and associates it with a name.
 * Takes two parameters:
 *  - a string that contains the name
 *  - the StringPrinter pointer to be added
 * Throws:
 *  - std::invalid_argument if an association for the name already exists ("Duplicate index").
 *  - std::invalid_argument if the pointer is null ("Invalid printer").
 * Note: The Printers object takes ownership of the pointer and is
 *  therefore responsible for eventually calling delete on the pointer.
 * 
 * Indexing operator: The collection is indexed using the
 * name (string) and returns a reference to the associated StringPrinter.
 * Throws:
 *  - std::invalid_argument if an association does not exist ("Unknown index").
 * 
 * Copy assignment operator: copies the object given to self.
 * ------------
 * Members:
 * Whatever you deem necessary.
 * ------------
 * Other:
 * Copying means copying the contained StringPrinters as well.
 * Remember to also implement the destructor.
 */
// BEGIN SOLUTION
#ifndef AALTO_ELEC_CPP_PRINTERS_CLASS
#define AALTO_ELEC_CPP_PRINTERS_CLASS

#include <string>
#include <map>
#include <iostream>

#include "string_printer.hpp"

class Printers {
public:
    /* Construct an empty Printers object */
    Printers();

    /* Copy construct a Printers object */
    Printers(const Printers&);

    /* Free all memory associated with the Printers object */
    ~Printers();

    /* Adds a new StringPrinter to the Printers object
     * Creates the association of name to StringPrinter*
     * Throws:
     *  std::invalid_argument if an association for the name already exists ("Duplicate index")
     *  std::invalid_argument if the pointer is null ("Invalid printer")
     * Note: The Printers object takes ownership of the pointer and is
     *  therefore responsible for eventually calling delete on the pointer
     */
    void Add(const std::string&, StringPrinter*);

    /* Retrieves a StringPrinter from the Printers object
     * The StringPrinter is obtained based on the association on the name
     * Returns:
     *  a reference to the StringPrinter object
     * Throws:
     *  std::invalid_argument if an association does not exist ("Unknown index")
     */
    StringPrinter& operator[](const std::string&);

    /* Assign a Printers object */
    Printers& operator=(const Printers&);

private:
    /* Internal representation of Printers
     * Associates std::string to StringPrinter*
     * Note: Due to the pointer content, this does not copy nicely
     */
    std::map<std::string, StringPrinter*> index_;
};

#endif
// END SOLUTION
