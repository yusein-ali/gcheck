/*
 * Declare and define the class in MyMemoryAllocator namespace.
 *
 * Changes to the class:
 *
 * BoundedPtr has a string parameter to store the use of the pointer.
 * If the pointer is automatically set to
 * a nullptr (i.e. not given in the constructor), the use is "nullptr".
 *
 * The copy constructor and assignment operators should throw a BoundedCopyException
 * if the operations fail (as they in some cases should). When the copying
 * fails, only the exception should be thrown and no changes should be made
 * to the existing pointer.
 *
 * The GetData function should throw a BoundedNullException with the use
 * of the BoundedPtr given to it if the pointer is null.
 *
 * The class shouldn't leak any memory.
 */

#ifndef AALTO_ELEC_CPP_RESTRICTED_PTR_CLASS
#define AALTO_ELEC_CPP_RESTRICTED_PTR_CLASS

#include "bounded_ptr_ex.hpp"
// BEGIN SOLUTION
namespace MyMemoryAllocator
{
    // END SOLUTION
    const int kMaxReferenceCount = 3;
    template <typename T>
    class BoundedPtr
    {
    public:
        // default constructor
        /**
         * \brief Construct a new Bounded Ptr object with null pointer and use_ initialized with "nullptr" string.
         *
         */
        BoundedPtr();

        // constructor when it is called with a pointer

        /**
         * \brief Construct a new Bounded Ptr object
         *
         * \param pointer_val the pointer to be managed by this object
         * \param use The usage of the pointer
         */
        BoundedPtr(T *pointer_val, const std::string &use);

        // copy constructor
        /**
         * \brief Construct a new Bounded Ptr object by copying from a source object
         *
         * \param bounded_ptr the source object to be copied into this one
         */
        BoundedPtr(const BoundedPtr<T> &bounded_ptr);

        // destructor, only delete the object if the reference count drops to zero
        ~BoundedPtr();

        // copy assignment
        BoundedPtr<T> &operator=(const BoundedPtr<T> &bounded_ptr);

        // member function to get the data from pointer
        // this is given as a reference, so it can be modified
        T &GetData();

        // member function to get the pointer address
        T *GetPointer()
        {
            return raw_pointer_;
        }

        // get ref count
        int GetRefCount()
        {
            return *ref_count_;
        }

    private:
        /**
         * \brief The raw pointer to be managed by this object
         *
         */
        T *raw_pointer_;
        /**
         * \brief Reference counter
         *
         */
        int *ref_count_;
        /**
         * \brief The use of the pointer
         *
         */
        std::string use_;

        /**
         * \brief Decrements reference count
         *
         * \return true if the counter has a value less than kMaxReferenceCount
         * \return false otherwise
         */
        bool IncrementRefCount()
        {
            if (*ref_count_ < kMaxReferenceCount)
            {
                (*ref_count_)++;
                return true;
            }
            return false;
        }

        /**
         * \brief Decrements the reference counter
         *
         * \return The value of the counter after decrementing it
         */
        int DecrementRefCount()
        {
            if ((*ref_count_) > 0)
            {
                (*ref_count_)--;
            }
            return *ref_count_;
        }
    };

    // BEGIN SOLUTION
    template <typename T>
    BoundedPtr<T>::BoundedPtr() : BoundedPtr(nullptr, "nullptr")
    {
    }

    template <typename T>
    BoundedPtr<T>::BoundedPtr(T *pointer_val, const std::string &use)
        : raw_pointer_(pointer_val), ref_count_(new int), use_(use)
    {
        *ref_count_ = (pointer_val == nullptr) ? 0 : 1;
    }

    template <typename T>
    BoundedPtr<T>::BoundedPtr(const BoundedPtr<T> &bounded_ptr)
    {
        if (*(bounded_ptr.ref_count_) >= kMaxReferenceCount)
        {
            throw BoundedCopyException(bounded_ptr.use_);
        }
        // only one ref addition is needed, as all BoundedPtr instances (that point to the same pointer) share the same ref_count_
        raw_pointer_ = bounded_ptr.raw_pointer_;
        ref_count_ = bounded_ptr.ref_count_;
        use_ = bounded_ptr.use_;
        IncrementRefCount();
    }

    template <typename T>
    BoundedPtr<T>::~BoundedPtr()
    {
        // remove one reference
        if (DecrementRefCount() == 0)
        {
            delete raw_pointer_;
            delete ref_count_;
        }
    }

    template <typename T>
    T &BoundedPtr<T>::GetData()
    {
        if (raw_pointer_)
            return *raw_pointer_;
        else
            throw BoundedNullException(use_);
    }

    template <typename T>
    BoundedPtr<T> &BoundedPtr<T>::operator=(const BoundedPtr<T> &bounded_ptr)
    {
        if (*(bounded_ptr.ref_count_) >= kMaxReferenceCount)
        {
            throw BoundedCopyException(bounded_ptr.use_);
        }
        if (DecrementRefCount() == 0)
        {
            delete ref_count_;
            delete raw_pointer_;
        }
        // only one ref addition is needed, as all BoundedPtr instances (that point to the same pointer) share the same ref_count_
        raw_pointer_ = bounded_ptr.raw_pointer_;
        ref_count_ = bounded_ptr.ref_count_;
        use_ = bounded_ptr.use_;

        // increment the reference count of the source object
        IncrementRefCount();
        return *this;
    }
} // namespace MyMemoryAllocator
// END SOLUTION

#endif
