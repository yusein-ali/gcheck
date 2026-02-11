#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

#include "bounded_ptr.hpp"
#include "bounded_ptr_ex.hpp"
#include "bounded_ptr_interface.hpp"
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

gcheck::Random rnd_int(4, 20);

std::string random_string(std::string::size_type length)
{
    static auto &chrs = "0123456789"
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::default_random_engine rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;

    s.reserve(length);

    while (length--)
        s += chrs[pick(rg)];

    return s;
}

TEST(BoundedPtr, GetData_exception, 1)
{
    bool GetData_threw_with_non_nullptr = false;
    try
    {
        MyMemoryAllocator::BoundedPtr<int> test_int(new int, "not null");
        test_int.GetData();
        MyMemoryAllocator::BoundedPtr<double> test_double(new double, "not null");
        test_double.GetData();
    }
    catch (MyMemoryAllocator::BoundedNullException &ex)
    {
        GetData_threw_with_non_nullptr = true;
    }
    EXPECT_FALSE(GetData_threw_with_non_nullptr) << "GetData shouldn't throw when the pointer is not null";

    bool GetData_threw_with_nullptr = false;
    try
    {
        MyMemoryAllocator::BoundedPtr<int> test_int(nullptr, "null");
        test_int.GetData();
    }
    catch (MyMemoryAllocator::BoundedNullException &ex)
    {
        GetData_threw_with_nullptr = true;
    }
    EXPECT_TRUE(GetData_threw_with_nullptr) << "GetData() should throw an exception when the raw pointer is nullptr.";
}

TEST(BoundedPtr, copy_exception, 1)
{
    bool copy_constructor_or_assignment_threw = false;
    MyMemoryAllocator::BoundedPtr<int> test_int(new int, "Just testing storage of integer.");
    try
    {
        MyMemoryAllocator::BoundedPtr<int> i2(test_int);

        MyMemoryAllocator::BoundedPtr<int> i3;

        i3 = i2;
    }
    catch (MyMemoryAllocator::BoundedPtrException &ex)
    {
        copy_constructor_or_assignment_threw = true;
    }
    EXPECT_FALSE(copy_constructor_or_assignment_threw) << "Copy constructor or assignment operator threw an exception where it should have not";

    MyMemoryAllocator::BoundedPtr<int> i2(test_int);
    MyMemoryAllocator::BoundedPtr<int> i3;
    i3 = i2;

    bool copy_constructor_threw = false;
    try
    {
        MyMemoryAllocator::BoundedPtr<int> i4(i2);
    }
    catch (MyMemoryAllocator::BoundedCopyException &ex)
    {
        copy_constructor_threw = true;
    }
    EXPECT_TRUE(copy_constructor_threw) << "Copy constructor should throw at the 4th copy";

    bool copy_assignment_threw = false;
    try
    {
        MyMemoryAllocator::BoundedPtr<int> i4;
        i4 = i3;
    }
    catch (MyMemoryAllocator::BoundedCopyException &ex)
    {
        copy_assignment_threw = true;
    }
    EXPECT_TRUE(copy_assignment_threw) << "Copy assignment operator should throw at the 4th copy";
}

TEST(BoundedCopyException, GetError, 1, "BoundedPtr.copy_exception")
{
    SetGradingMethod(gcheck::AllOrNothing);

    std::string expect = random_string(6);

    std::string correct = expect + ": Too many copies of BoundedPtr!";

    MyMemoryAllocator::BoundedPtr<int> test_int(new int, expect);

    MyMemoryAllocator::BoundedPtr<int> i2(test_int);

    MyMemoryAllocator::BoundedPtr<int> i3;

    i3 = i2;

    std::string GetError_after_copy_construct;
    try
    {
        MyMemoryAllocator::BoundedPtr<int> i4(i3);
    }
    catch (MyMemoryAllocator::BoundedCopyException &ex)
    {
        GetError_after_copy_construct = ex.GetError();
    }
    EXPECT_EQ(correct, GetError_after_copy_construct) << "Testing GetError of BoundedCopyException after copy constructor";

    std::string GetError_after_copy_assignment;
    try
    {
        MyMemoryAllocator::BoundedPtr<int> i4 = i3;
    }
    catch (MyMemoryAllocator::BoundedCopyException &ex)
    {
        GetError_after_copy_assignment = ex.GetError();
    }
    EXPECT_EQ(correct, GetError_after_copy_assignment) << "Testing GetError of BoundedCopyException after copy assignment";
}

TEST(BoundedPtrInterface, MakeBounded, 1, "BoundedPtr.GetData_exception")
{
    SetGradingMethod(gcheck::AllOrNothing);
    int *tester = new int;

    *tester = rnd_int.Next();
    int correct_int = rnd_int();

    MyMemoryAllocator::BoundedPtr<int> test_ptr = MakeBounded(tester);

    EXPECT_EQ(tester, test_ptr.GetPointer()) << "After MakeBounded with the pointer 'tester'";
    EXPECT_EQ(correct_int, test_ptr.GetData()) << "After MakeBounded with a pointer containing " << correct_int;

    std::string *another = new std::string;
    std::string correct = "test string" + random_string(3);
    *another = correct;

    std::string expectString = *another;

    MyMemoryAllocator::BoundedPtr<std::string> test_ptr2 = MakeBounded(another);

    EXPECT_EQ(another, test_ptr2.GetPointer()) << "After MakeBounded with the pointer 'another'";
    EXPECT_EQ(correct, test_ptr2.GetData()) << "After MakeBounded with a pointer containing the string: " << correct;
}

TEST(BoundedPtrInterface, CopyBounded)
{
    int *tester = new int;

    *tester = 6;

    MyMemoryAllocator::BoundedPtr<int> test_ptr(tester, "tester");

    MyMemoryAllocator::BoundedPtr<int> test_ptr2 = CopyBounded(test_ptr);
    EXPECT_EQ(tester, test_ptr.GetPointer()) << "After CopyBounded(test_ptr) with test_ptr containing the pointer 'tester'";
    EXPECT_EQ(tester, test_ptr2.GetPointer()) << "After CopyBounded(test_ptr) with test_ptr containing the pointer 'tester'";

    // test that the 4th copy fails
    MyMemoryAllocator::BoundedPtr<int> test_ptr3(test_ptr);

    gcheck::StdoutCapturer tout;

    MyMemoryAllocator::BoundedPtr<int> test_ptr4 = CopyBounded(test_ptr);

    std::string CopyBounded_out = tout.str();

    std::string correct = "tester: Too many copies of BoundedPtr!\n";

    EXPECT_EQ(correct, CopyBounded_out) << "On the 4th copy";
}

TEST(BoundedPtrInterface, write_operator, 1, "MakeBounded")
{
    int *tester = new int;
    int correct_int = rnd_int.Next();
    *tester = correct_int;

    MyMemoryAllocator::BoundedPtr<int> test_ptr = MakeBounded(tester);
    MyMemoryAllocator::BoundedPtr<int> null_ptr;

    gcheck::StdoutCapturer tout;

    std::cout << test_ptr;
    std::cout << null_ptr;

    // read output from function
    std::string write_operator_out = tout.str();

    std::string correct = std::to_string(correct_int) + "\n"
                                                        "nullptr: nullptr exception!\n";

    EXPECT_EQ(correct, write_operator_out) << "Output of writing two pointers; one not null and the other null";
}