#ifndef AALTO_ELEC_CPP_VEHICLE_CLASS
#define AALTO_ELEC_CPP_VEHICLE_CLASS

#include <string>

/**
 * @brief Parent class for different vehicle types.
 */
class Vehicle {
public:
    /**
     * @brief Construct a new Vehicle object
     * 
     * @param register_number registration of the vehicle as a std::string
     * @param owner name of the owner as a std::string
     */
    explicit Vehicle(std::string register_number,
                     std::string owner);

    /**
     * @brief Destroy the Vehicle object.
     */
    virtual ~Vehicle() {};

    /**
     * @brief Get the register number of the vehicle
     * 
     * @return registration of the vehicle as a std::string 
     */
    std::string GetRegisterNumber() const;

    /**
     * @brief Get the owner of the vehicle
     * 
     * @return name of the owner as a std::string 
     */
    std::string GetOwner() const;

    /**
     * @brief Writes the vehicle to the stream given as a parameter in the
     * serialized format for that vehicle. Note: this is a pure virtual method.
     * 
     * @param stream reference to the desired ostream 
     */
    virtual void Write(std::ostream& stream) = 0;

    /**
     * @brief Writes the vehicle to the standard output in the serialized format
     * for that vehicle. Note: this is a pure virtual method.
     */
    virtual void Print() = 0;
private:
    std::string register_number_;
    std::string owner_;
};

#endif
