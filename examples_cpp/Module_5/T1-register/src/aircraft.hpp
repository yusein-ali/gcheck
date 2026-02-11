#ifndef AALTO_ELEC_CPP_AIRCRAFT_CLASS
#define AALTO_ELEC_CPP_AIRCRAFT_CLASS

#include <string>
#include <iostream>

#include "vehicle.hpp"

/**
 * @brief Inherits class Vehicle and implements functionalities required for
 * handling aircraft.
 */
class Aircraft : public Vehicle {
public:
    /**
     * @brief Construct a new Aircraft object
     * 
     * @param register_number registration of the vehicle as a std::string
     * @param owner name of the owner as a std::string
     * @param model name of the vehicle model as a std::string
     * @param wingspan wingspan of the aircraft as a double
     * @param cruise_speed cruise speed of the aircraft as an unsigned integer
     */
    Aircraft(std::string register_number,
             std::string owner,
             std::string model,
             double wingspan,
             unsigned int cruise_speed);

    /**
     * @brief Writes the vehicle to the stream given as a parameter in a
     * predefined serialized format.
     * 
     * Format for an aircraft:
     * A;<register number>;<owner>;<model>;<wingspan>;<cruise_speed>\n
     * 
     * @param stream reference to the desired ostream 
     */
    void Write(std::ostream& stream);
    
    /**
     * @brief Writes the vehicle to the standard output in the given serialized
     * format (see method Write for details).
     */
    void Print();

    /**
     * @brief Reads an Aircraft object from a given istream.
     * 
     * Assumes that the vehicle is stored in a predefined serialized format (see
     * method Write for details). Reading is started immediately from the
     * current position of the stream.
     * 
     * @param stream reference to the desired istream 
     * @return a Aircraft* pointer refering to the newly created object if the
     * input can be read successfully
     * @return a NULL pointer otherwise
     */
    static Aircraft* Read(std::istream& stream);

private:
    std::string model_;
    double wingspan_;
    unsigned int cruise_speed_;
};

#endif