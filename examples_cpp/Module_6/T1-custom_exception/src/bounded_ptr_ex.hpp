
/* TODO: classes BoundedPtrException, BoundedCopyException
 * and BoundedNullException are parts of MyMemoryAllocator namespace.
 * ------------
 * Description:
 * Exception classes used by the BoundedPtr class.
 * BoundedPtrException is a abstract base class of the other two and inherits
 * std::exception.
 * CopyException signifies a copying error.
 * NullException signifies a nullptr error.
 * ------------
 * Functions:
 *
 * All of them should have a constructor taking a single const
 * reference to a string which is the use of the BoundedPtr that
 * caused the exception.
 *
 * GetError: A function that takes no arguments and returns
 * a string description of the error (see "Other" section for details).
 * Calling GetError on BoundedPtrException should return the
 * error description of the actual exception type (Null- or
 * CopyException). Consequently, base class BoundedPtrException
 * must declare this function as pure virtual.
 *
 * GetUse: takes no parameters and returns the string given as a
 * parameter to the constructor.
 * ------------
 * Other:
 * As always, any function that doesn't modify the object or allow
 * modifying the object's members from the outside should be const.
 *
 * Wrap everything inside the MyMemoryAllocator namespace.
 *
 * GetError format:
 * BoundedCopyException:
<use>: Too many copies of BoundedPtr!
 * BoundedNullException:
<use>: nullptr exception!
 * where <use> is replaced by the string given in to the constructor.
*/

#ifndef AALTO_ELEC_CPP_PTR_EX_CLASS
#define AALTO_ELEC_CPP_PTR_EX_CLASS

#include <exception>
#include <string>

// BEGIN SOLUTION
namespace MyMemoryAllocator
{

    class BoundedPtrException : public std::exception
    {
    public:
        BoundedPtrException(const std::string &name) : use_(name) {}
        virtual std::string GetError() const = 0;
        std::string GetUse() const
        {
            return use_;
        }

    private:
        std::string use_;
    };

    class BoundedCopyException : public BoundedPtrException
    {
    public:
        BoundedCopyException(const std::string &name) : BoundedPtrException(name) {}

        std::string GetError() const
        {
            return GetUse() + ": Too many copies of BoundedPtr!";
        }
    };

    class BoundedNullException : public BoundedPtrException
    {
    public:
        BoundedNullException(const std::string &name) : BoundedPtrException(name) {}

        std::string GetError() const
        {
            return GetUse() + ": nullptr exception!";
        }
    };

} // namespace MyMemoryAllocator

// END SOLUTION
#endif
