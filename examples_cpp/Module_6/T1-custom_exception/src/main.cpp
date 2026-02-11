#include "bounded_ptr_ex.hpp"
#include "bounded_ptr_interface.hpp"
#include <iostream>

int main(void)
{
    int *tester = new int;

    *tester = 6;

    MyMemoryAllocator::BoundedPtr<int> testi_ptr = MakeBounded(tester);

    MyMemoryAllocator::BoundedPtr<int> testi_ptr2 = CopyBounded(testi_ptr);

    std::cout << "Next should print 6" << std::endl;
    std::cout << testi_ptr2.GetData() << std::endl;

    MyMemoryAllocator::BoundedPtr<int> testi_ptr3 = CopyBounded(testi_ptr);

    std::cout << "next should fail and print out: BoundedPtr instance: Too many copies of BoundedPtr!" << std::endl;

    MyMemoryAllocator::BoundedPtr<int> testi_ptr4 = CopyBounded(testi_ptr);

    // calling GetData with this null_ptr instance should throw a
    // BoundedNullException
    MyMemoryAllocator::BoundedPtr<int> null_ptr(nullptr, "nullptr instance");

    std::cout << "This should print out: nullptr instance: nullptr exception!" << std::endl;

    std::cout << null_ptr;

    return 0;
}