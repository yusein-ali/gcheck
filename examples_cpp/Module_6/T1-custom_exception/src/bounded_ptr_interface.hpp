/* Add include guards here */

/**
 * @brief a wrapper for class BoundedPtr
 *
 * TODO: Implement the following methods:
 *
 * MakeBounded: a template function that takes any type of
 * pointer as a parameter and returns a BoundedPtr holding
 * that pointer with the use set to "BoundedPtr instance".
 *
 * CopyBounded: a template function that takes a reference to
 * a BoundedPtr as a parameter and returns a copy of it. If
 * the copying fails, prints the error in the exception to
 * standard output with a newline appended and returns a
 * default constructed BoundedPtr.
 *
 * operator<<: a template function that takes in a ostream
 * reference and a BoundedPtr reference, prints the object
 * pointed to by the pointer to the stream using the << operator
 * and returns a reference to the stream. If the pointer is null,
 * prints the return value of GetError of the exception thrown by
 * GetData instead.
 */

#ifndef AALTO_ELEC_CPP_BOUNDED_INTERFACE_CLASS
#define AALTO_ELEC_CPP_BOUNDED_INTERFACE_CLASS

#include <iostream>

#include "bounded_ptr.hpp"
#include "bounded_ptr_ex.hpp"

using MyMemoryAllocator::BoundedPtr;


/**
 * \brief MakeBounded: a template function that takes any type of
 * pointer as a parameter and returns a BoundedPtr holding
 * that pointer with the use set to "BoundedPtr instance".
 *
 * \tparam T pointer type
 * \param ptr The pointer instance
 * \return BoundedPtr<T> Bounded smart pointer object 
 */ 
// BEGIN SOLUTION
template <typename T>
BoundedPtr<T> MakeBounded(T *ptr)
{
    return BoundedPtr<T>(ptr, "BoundedPtr instance");
}
// END SOLUTION
/**
 * \brief Creates a copy of a BoundedPointer object
 * 
 * \tparam T pointer type
 * \param bounded_ptr The object to be copied into a new object
 * \return A opy of the argument object
 */
// BEGIN SOLUTION
template <typename T>
BoundedPtr<T> CopyBounded(BoundedPtr<T> &bounded_ptr)
{
    try
    {
        return BoundedPtr<T>(bounded_ptr);
    }
    catch (MyMemoryAllocator::BoundedPtrException &ex)
    {
        std::cout << ex.GetError() << std::endl;
        return BoundedPtr<T>(); // return a nullptr
    }
}
// END SOLUTION
/**
 * \brief Outputs the data of a BoundedPtr object into the specfied output stream
 * 
 * \tparam T 
 * \param os 
 * \param bounded_ptr 
 * \return std::ostream& 
 */
template <typename T>
std::ostream &operator<<(std::ostream &os, BoundedPtr<T> &bounded_ptr)
{
    try
    {
        os << bounded_ptr.GetData() << std::endl;
    }
    catch (MyMemoryAllocator::BoundedPtrException &ex)
    {
        os << ex.GetError() << std::endl;
    }

    return os;
}

#endif

