#ifndef AALTO_ELEC_CPP_REGISTER_CLASS
#define AALTO_ELEC_CPP_REGISTER_CLASS

#include <string>
#include <vector>

#include "vehicle.hpp"

/**
 * @brief Class for storing and interacting with Vehicle objects.
 */
class Register {
public:
    /**
     * @brief Destroy the Register object
     */
    ~Register();
    
    /**
     * @brief Adds a new vehicle to the Vehicle* vector vehicles_
     * 
     * @param v Vehicle pointer to be added
     */
    void Add(Vehicle* v);

    /**
     * @brief Saves the vehicles to the file named by the parameter with each
     * vehicle on its own line in the serialized format.
     * 
     * @param filename Reference to the desired filename as std::string 
     */
    void Save(const std::string& filename) const;

    /**
     * @brief Reads a vehicle from the stream given as a parameter assuming a
     * serialized format, and adds it to the register.
     * 
     * Reads a whole line from the stream whether or not constructing the
     * vehicle was succesful.
     * 
     * @param stream 
     * @return true if a vehicle was added
     * @return false otherwise
     */
    bool ReadLine(std::istream& stream);
   
    /**
     * @brief Reads all vehicles from a file and adds them to the register.
     * 
     * Each vehicle is on a different line. On an error the line is discarded
     * and reading is continued from the next line.
     * 
     * @param filename 
     * @return the number of vehicles added as an integer
     * @return -1 if opening the file fails
     */
    int Load(const std::string& filename);

    /**
     * @brief Prints all the vehicles in the register to the standard output
     * each on a different line in the serialized format.
     */
    void Print();

    /**
     * @brief Get the number of vehicles in the register.
     * 
     * @return the number of vehicles in the register as a size_t integer. 
     */
    size_t Size() const;

private:
    std::vector<Vehicle*> vehicles_;
};

#endif