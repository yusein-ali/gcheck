#ifndef AALTO_ELEC_CPP_RESTRICTED_PTR_CLASS
#define AALTO_ELEC_CPP_RESTRICTED_PTR_CLASS

#include "restricted_ref_counter.hpp"

/*
    Implement a class RestrictedPtr that holds a pointer of any type with at least the following members:
        - default constructor (takes no parameters)
        - constructor with a raw pointer parameter
        - copy constructor
        - destructor
        - copy assignment operator
        - function GetData that takes no parameters and returns the data the raw pointer refers to.
            The data returned should be such that modifying it modifies the data the pointer refers to.
        - function GetPointer that takes no parameters and returns the raw pointer
        - function GetRefCount that takes no parameters and returns an int with the reference count
    
    At any time the objects should keep track of how many references there are to a pointer
    and not allow more than 3 references. If the reference count drops to 0, the referenced
    pointer's memory should be released. If the reference count is already 3 when copying,
    the copier should set it's pointer to nullptr and reference count to 1.
    NOTE: GetPointer, GetData and GetRefCount are needed for the tests to run
*/
// BEGIN SOLUTION
#include <algorithm>

template<typename T>
class RestrictedPtr {
public:
    // default constructor
    RestrictedPtr();

    // constructor when it is called with a pointer
    RestrictedPtr(T* pointer_val);

    // copy constructor
    RestrictedPtr(const RestrictedPtr<T>& restr_ptr);

    // destructor, only delete the object if the reference count drops to zero
    ~RestrictedPtr();

    // copy assignment
    RestrictedPtr<T>& operator=(const RestrictedPtr<T>& restr_ptr);

    // member function to get the data from pointer
    // this is given as a reference, so it can be modified
    T& GetData() { return *raw_pointer_; }

    // member function to get the pointer address
    T* GetPointer() { return raw_pointer_; }

    // get reference count
    int GetRefCount() { return ref_counter_->GetCount(); }
private:
    // swaps the data in the object with the parameter's
    void Swap(RestrictedPtr<T>& restr_ptr);

    T* raw_pointer_;
    RestrictedRefCounter* ref_counter_;
};

template<typename T>
RestrictedPtr<T>::RestrictedPtr() : RestrictedPtr(nullptr) {}

template<typename T>
RestrictedPtr<T>::RestrictedPtr(T* pointer_val)
        : raw_pointer_(pointer_val), ref_counter_(new RestrictedRefCounter()) {
    ref_counter_->Increment();
}

template<typename T>
RestrictedPtr<T>::RestrictedPtr(const RestrictedPtr<T>& restr_ptr) {
    if(restr_ptr.ref_counter_->Increment()) {
        // only one ref addition is needed, as all RestrictedPtr instances 
        // (that point to the same pointer) share the same ref_counter_
        raw_pointer_ = restr_ptr.raw_pointer_;
        ref_counter_ = restr_ptr.ref_counter_;
    } else {
        raw_pointer_ = nullptr;
        ref_counter_ = new RestrictedRefCounter();
        ref_counter_->Increment();
    }
}

template<typename T>
RestrictedPtr<T>::~RestrictedPtr() {
    // remove one reference 
    if(ref_counter_->Decrement() == 0) {
        delete raw_pointer_;
        delete ref_counter_;
    }
}

template<typename T>
RestrictedPtr<T>& RestrictedPtr<T>::operator=(const RestrictedPtr<T>& restr_ptr) {
    // using the copy and swap idiom
    /* This creates a new copy of the RestrictedPtr using the copy constuctor,
     * which takes care of incrementing the reference count and checking that
     * it doesn't exceed 3. After that, it swaps the data between the copy and
     * the object itself, effectively making itself a copy of the given pointer,
     * and sets the local pointer (nptr) to reference the original pointer.
     * When the function returns, nptr's destructor is called and the reference
     * count is decremented and memory is released if the count reaches 0.
     * More in-depth explanation of the copy and swap idiom: 
     * https://stackoverflow.com/a/3279550
     */
    RestrictedPtr<T> nptr(restr_ptr);
    Swap(nptr);
    
    return *this;
}

template<typename T>
void RestrictedPtr<T>::Swap(RestrictedPtr<T>& restr_ptr) {
    std::swap(raw_pointer_, restr_ptr.raw_pointer_);
    std::swap(ref_counter_, restr_ptr.ref_counter_);
}
// END SOLUTION

#endif