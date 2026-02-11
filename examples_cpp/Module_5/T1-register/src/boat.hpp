#ifndef AALTO_ELEC_CPP_BOAT_CLASS
#define AALTO_ELEC_CPP_BOAT_CLASS

#include <iostream>
#include <string>

#include "vehicle.hpp"

/**
 * @brief Inherits class Vehicle and implements functionalities required for
 * handling boats.
 */
class Boat : public Vehicle
{
public:
    /**
     * @brief Construct a new Boat object
     * 
     * @param register_number registration of the vehicle as a std::string
     * @param owner name of the owner as a std::string
     * @param name  name of the vehicle as a std::string
     * @param draft draft of the boat as a double
     * @param power power of the boat as a double
     */
    Boat(std::string register_number,
         std::string owner,
         std::string name,
         double draft,
         double power);

    /**
     * @brief Writes the vehicle to the stream given as a parameter in a
     * predefined serialized format.
     * 
     * Format for a boat:
     * B;<register number>;<owner>;<name>;<draft>;<power>\n
     * 
     * @param stream reference to the desired ostream 
     */
    void Write(std::ostream &stream);

    /**
     * @brief Writes the vehicle to the standard output in the given serialized
     * format (see method Write for details).
     */
    void Print();

    /**
     * @brief Reads a Boat object from a given istream.
     * 
     * Assumes that the vehicle is stored in a predefined serialized format (see
     * method Write for details). Reading is started immediately from the
     * current position of the stream.
     * 
     * @param stream reference to the desired istream 
     * @return a Boat* pointer refering to the newly created object if the input
     * can be read successfully. Otherwise, a NULL pointer.
     */
    static Boat *Read(std::istream &stream);

private:
    std::string name_;
    double draft_;
    double power_;
};

#endif