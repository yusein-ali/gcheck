#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

#include "book.hpp"
#include "customer.hpp"
#include "library.hpp"
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

std::string GenerateISBN()
{
    srand(time(0));
    std::random_device rd;                             // obtain a random number from hardware
    std::mt19937 eng(rd());                            // seed the generator
    std::uniform_int_distribution<> distr1(100, 999); // define the range
    std::uniform_int_distribution<> distr2(100, 999); // define the range
    std::string s = "978-0-" + std::to_string(distr1(eng)) + "-" + std::to_string(distr2(eng)) + "-2";
    return s;
}

Date GetCurrentDate()
{
    Date d;
    std::time_t t = time(0);
    struct tm *now = localtime(&t);
    d.day = now->tm_mday;
    d.month = now->tm_mon + 1;
    d.year = now->tm_year + 1900;
    return d;
}

Date GetCurrentDatePlusMonth()
{
    Date d = GetCurrentDate();
    d.month++;
    return d;
}

bool operator==(const Date &d, const Date &d2)
{
    return d.day == d2.day && d.month == d2.month && d.year == d2.year;
}

bool operator==(const Book &b, const Book &b2)
{
    return b.GetAuthor() == b2.GetAuthor() && b.GetDueDate() == b2.GetDueDate() && b.GetISBN() == b2.GetISBN() && b.GetName() == b2.GetName() && b.GetStatus() == b2.GetStatus();
}

std::string to_string(const Date &d)
{

    return std::to_string(d.day) + "." + std::to_string(d.month) + "." + std::to_string(d.year);
}
std::string to_string(const Book &b)
{
    return b.GetAuthor() + ": " + b.GetName() + ", due: " + to_string(b.GetDueDate()) + ", isbn: " + b.GetISBN() + ", status: " + (b.GetStatus() ? "true" : "false");
}

namespace {

class BookConstructorTest : public gcheck::CustomTest {
public:
    BookConstructorTest() : gcheck::CustomTest(gcheck::TestInfo("Book", "constructor", 0.5)) {}
private:
    void TheTest() override {
        Book book("name", "author", "isbn");
        Book book2("name", "author", "isbn", true, {1, 1, 1});
        std::cout << "Testing Book class constructor" << std::endl;
        ExpectTrue(true, "Two objects can be constructed using different constructors");
    }
};
BookConstructorTest book_constructor_test;

class BookGettersTest : public gcheck::CustomTest {
public:
    BookGettersTest() : gcheck::CustomTest(gcheck::TestInfo("Book", "getters", 2, "constructor")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2", false, {1, 1, 1});
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4", true, {2, 2, 2});

        EXPECT_EQ("Stroustrup, Bjarne", book.GetAuthor());
        EXPECT_EQ("Lippman, Stanley B.", book2.GetAuthor());

        EXPECT_EQ("978-0-321-56384-2", book.GetISBN());
        EXPECT_EQ("978-0-321-71411-4", book2.GetISBN());

        EXPECT_FALSE(book.GetStatus());
        EXPECT_TRUE(book2.GetStatus());

        EXPECT_EQ(Date({1, 1, 1}), book.GetDueDate());
        EXPECT_EQ(Date({2, 2, 2}), book2.GetDueDate());
    }
};
BookGettersTest book_getters_test;

class BookLoanRestoreTest : public gcheck::CustomTest {
public:
    BookLoanRestoreTest() : gcheck::CustomTest(gcheck::TestInfo("Book", "Loan_Restore", 2, "getters")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");

        book.Loan();
        EXPECT_EQ(GetCurrentDatePlusMonth(), book.GetDueDate());
        EXPECT_TRUE(book.GetStatus());

        book.Restore();
        EXPECT_EQ(Date({0, 0, 0}), book.GetDueDate());
        EXPECT_FALSE(book.GetStatus());
    }
};
BookLoanRestoreTest book_loan_restore_test;

class BookPrintTest : public gcheck::CustomTest {
public:
    BookPrintTest() : gcheck::CustomTest(gcheck::TestInfo("Book", "Print", 1, "getters")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4", true, {1, 10, 2019});

        gcheck::StdoutCapturer tout;

        book.Print();
        std::string book1_Print = tout.str();

        book2.Print();
        std::string book2_Print = tout.str();

        std::string correct1 = "Book: The C++ programming language, author: Stroustrup, Bjarne, ISBN: 978-0-321-56384-2, loaned false, due date: 0.0.0\n"; // TODO: date
        std::string correct2 = "Book: C++ Primer, author: Lippman, Stanley B., ISBN: 978-0-321-71411-4, loaned true, due date: 1.10.2019\n";

        EXPECT_EQ(correct1, book1_Print);
        EXPECT_EQ(correct2, book2_Print);
    }
};
BookPrintTest book_print_test;

class CustomerConstructorTest : public gcheck::CustomTest {
public:
    CustomerConstructorTest() : gcheck::CustomTest(gcheck::TestInfo("Customer", "constructor", 0.5)) {}
private:
    void TheTest() override {
        Customer("Pertti", "10");
        std::cout << "Testing Customer class constructor" << std::endl;
        ExpectTrue(true, "One object can be constructed using the defined constructor");
    }
};
CustomerConstructorTest customer_constructor_test;

class CustomerGettersTest : public gcheck::CustomTest {
public:
    CustomerGettersTest() : gcheck::CustomTest(gcheck::TestInfo("Customer", "getters", 2, "constructor")) {}
private:
    void TheTest() override {
        Customer customer("Pertti Kovanen", "45647658657");
        Customer customer2("Satu Kivinen", "95871034857");

        EXPECT_EQ("Pertti Kovanen", customer.GetName());
        EXPECT_EQ("Satu Kivinen", customer2.GetName());

        EXPECT_EQ("45647658657", customer.GetID());
        EXPECT_EQ("95871034857", customer2.GetID());

        EXPECT_EQ(0u, customer.GetLoans().size());
        EXPECT_EQ(0u, customer2.GetLoans().size());

        EXPECT_EQ(0, customer.GetLoanAmount());
        EXPECT_EQ(0, customer2.GetLoanAmount());
    }
};
CustomerGettersTest customer_getters_test;

class CustomerLoanBookTest : public gcheck::CustomTest {
public:
    CustomerLoanBookTest() : gcheck::CustomTest(gcheck::TestInfo("Customer", "LoanBook", 1, "getters")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        Customer customer("Pertti Kovanen", "45647658657");

        EXPECT_EQ(0u, customer.GetLoans().size());
        EXPECT_EQ(0, customer.GetLoanAmount());

        customer.LoanBook(book);
        ASSERT_TRUE(1 == customer.GetLoanAmount());
        EXPECT_EQ(book, customer.GetLoans()[0]);

        customer.LoanBook(book2);
        ASSERT_TRUE(2 == customer.GetLoanAmount());
        EXPECT_EQ(book2, customer.GetLoans()[1]);
    }
};
CustomerLoanBookTest customer_loanbook_test;

class CustomerReturnBookTest : public gcheck::CustomTest {
public:
    CustomerReturnBookTest() : gcheck::CustomTest(gcheck::TestInfo("Customer", "ReturnBook", 1, "LoanBook")) {}
private:
    void TheTest() override {
        SetGradingMethod(gcheck::AllOrNothing);

        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        Customer customer("Pertti Kovanen", "45647658657");

        customer.LoanBook(book);
        customer.LoanBook(book2);

        customer.ReturnBook(book);
        ASSERT_TRUE(1 == customer.GetLoanAmount());
        EXPECT_EQ(book2, customer.GetLoans()[0]);

        customer.ReturnBook(book2);
        EXPECT_EQ(0, customer.GetLoanAmount());
    }
};
CustomerReturnBookTest customer_returnbook_test;

class CustomerPrintTest : public gcheck::CustomTest {
public:
    CustomerPrintTest() : gcheck::CustomTest(gcheck::TestInfo("Customer", "Print", 1, "LoanBook")) {}
private:
    void TheTest() override {
        Customer customer("Pertti Kovanen", "45647658657");
        Customer customer2("Satu Kivinen", "95871034857");
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        Book book3("C++ For Dummies", "Stephen Randy Davis", "978-1-118-82377-4");

        gcheck::StdoutCapturer tout;

        customer.Print();
        std::string customer1_Print = tout.str();

        customer.LoanBook(book);
        customer.Print();
        std::string customer1_Print2 = tout.str();

        customer2.LoanBook(book);
        customer2.LoanBook(book2);
        customer2.LoanBook(book3);
        customer2.Print();
        std::string customer2_Print = tout.str();

        std::string loan_date = to_string(GetCurrentDatePlusMonth());
        std::string book1str = "Book: The C++ programming language, author: Stroustrup, Bjarne, ISBN: 978-0-321-56384-2, loaned true, due date: " + loan_date + "\n";
        std::string book2str = "Book: C++ Primer, author: Lippman, Stanley B., ISBN: 978-0-321-71411-4, loaned true, due date: " + loan_date + "\n";
        std::string book3str = "Book: C++ For Dummies, author: Stephen Randy Davis, ISBN: 978-1-118-82377-4, loaned true, due date: " + loan_date + "\n";

        std::string correct1 = "Customer: Pertti Kovanen, 45647658657, has 0 books on loan:\n";
        std::string correct1_2 = "Customer: Pertti Kovanen, 45647658657, has 1 books on loan:\n"
                                 "- " +
                                 book1str;
        std::string correct2 = "Customer: Satu Kivinen, 95871034857, has 2 books on loan:\n"
                               "- " +
                               book2str + "- " + book3str;

        EXPECT_EQ(correct1, customer1_Print);
        EXPECT_EQ(correct1_2, customer1_Print2);
        EXPECT_EQ(correct2, customer2_Print);
    }
};
CustomerPrintTest customer_print_test;

class LibraryConstructorTest : public gcheck::CustomTest {
public:
    LibraryConstructorTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "constructor", 0.5, "Book.constructor Customer.constructor")) {}
private:
    void TheTest() override {
        Library library("Test Library");
        std::cout << "Testing Library class constructor" << std::endl;
        ExpectTrue(true, "One object can be constructed using the defined constructor");
    }
};
LibraryConstructorTest library_constructor_test;

class LibraryGetBooksTest : public gcheck::CustomTest {
public:
    LibraryGetBooksTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "GetBooks", 1, "constructor")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        auto i = GenerateISBN();
        auto i2 = GenerateISBN();
        Book book3("Test book", "Test author", i);
        Book book4("Test book2", "Test author", i2);

        Library library("Test Library");
        auto &books = library.GetBooks();
        books.push_back(book);
        books.push_back(book2);
        books.push_back(book3);
        books.push_back(book4);

        EXPECT_EQ(book.GetName(), books[0].GetName());
        EXPECT_EQ(book2.GetName(), books[1].GetName());
        EXPECT_EQ(book3.GetName(), books[2].GetName());
        EXPECT_EQ(book4.GetName(), books[3].GetName());
    }
};
LibraryGetBooksTest library_getbooks_test;

class LibraryGetCustomersTest : public gcheck::CustomTest {
public:
    LibraryGetCustomersTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "GetCustomers", 1, "constructor")) {}
private:
    void TheTest() override {
        Customer customer("Pertti Kovanen", "45647658657");
        Customer customer2("Satu Kivinen", "95871034857");
        Library library("Test Library");
        auto &customers = library.GetCustomers();
        customers.push_back(customer);
        customers.push_back(customer2);

        EXPECT_EQ(customer.GetName(), customers[0].GetName());
        EXPECT_EQ(customer2.GetName(), customers[1].GetName());
    }
};
LibraryGetCustomersTest library_getcustomers_test;

class LibraryFindBookByNameTest : public gcheck::CustomTest {
public:
    LibraryFindBookByNameTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "FindBookByName", 1, "GetBooks Book.getters")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        auto i = GenerateISBN();
        auto i2 = GenerateISBN();
        Book book3("Test book", "Test author", i);
        Book book4("Test book2", "Test author", i2);

        Library library("Test Library");
        auto &books = library.GetBooks();
        books.push_back(book);
        books.push_back(book2);
        books.push_back(book3);
        books.push_back(book4);

        EXPECT_EQ(book2.GetISBN(), library.FindBookByName("C++ Primer").GetISBN());
        EXPECT_EQ(book3.GetISBN(), library.FindBookByName("Test book").GetISBN());
    }
};
LibraryFindBookByNameTest library_findbookbyname_test;

class LibraryFindBooksByAuthorTest : public gcheck::CustomTest {
public:
    LibraryFindBooksByAuthorTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "FindBooksByAuthor", 1, "GetBooks Book.getters")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        auto i = GenerateISBN();
        auto i2 = GenerateISBN();
        Book book3("Test book", "Test author", i);
        Book book4("Test book2", "Test author", i2);

        Library library("Test Library");
        auto &books = library.GetBooks();
        books.push_back(book);
        books.push_back(book2);
        books.push_back(book3);
        books.push_back(book4);

        EXPECT_EQ(book3.GetISBN(), library.FindBooksByAuthor("Test author")[0].GetISBN());
        EXPECT_EQ(book4.GetISBN(), library.FindBooksByAuthor("Test author")[1].GetISBN());
    }
};
LibraryFindBooksByAuthorTest library_findbooksbyauthor_test;

class LibraryFindAllLoanedBooksTest : public gcheck::CustomTest {
public:
    LibraryFindAllLoanedBooksTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "FindAllLoanedBooks", 1, "GetBooks Customer.LoanBook Book.getters")) {}
private:
    void TheTest() override {
        Book book("The C++ programming language", "Stroustrup, Bjarne", "978-0-321-56384-2");
        Book book2("C++ Primer", "Lippman, Stanley B.", "978-0-321-71411-4");
        auto i = GenerateISBN();
        auto i2 = GenerateISBN();
        Book book3("Test book", "Test author", i);
        Book book4("Test book2", "Test author", i2);

        Customer customer("Pertti Kovanen", "45647658657");
        Customer customer2("Satu Kivinen", "95871034857");

        customer.LoanBook(book2);
        customer2.LoanBook(book4);

        Library library("Test Library");
        auto &books = library.GetBooks();
        books.push_back(book);
        books.push_back(book2);
        books.push_back(book3);
        books.push_back(book4);

        std::vector<Customer> &customers = library.GetCustomers();
        customers.push_back(customer);
        customers.push_back(customer2);

        ASSERT_TRUE(2 == library.FindAllLoanedBooks().size());

        EXPECT_EQ(book2.GetISBN(), library.FindAllLoanedBooks()[0].GetISBN());
        EXPECT_EQ(book4.GetISBN(), library.FindAllLoanedBooks()[1].GetISBN());
    }
};
LibraryFindAllLoanedBooksTest library_findallloanedbooks_test;

class LibraryFindCustomerTest : public gcheck::CustomTest {
public:
    LibraryFindCustomerTest() : gcheck::CustomTest(gcheck::TestInfo("Library", "FindCustomer", 1, "GetCustomers Customer.getters")) {}
private:
    void TheTest() override {
        Customer customer("Pertti Kovanen", "45647658657");
        Customer customer2("Satu Kivinen", "95871034857");
        Customer customer3("Santeri Sankala", "95576031857");
        Library library("Test Library");
        std::vector<Customer> &v = library.GetCustomers();
        v.push_back(customer);
        v.push_back(customer2);
        v.push_back(customer3);

        EXPECT_EQ(customer.GetName(), library.FindCustomer("45647658657").GetName());
        EXPECT_EQ(customer3.GetName(), library.FindCustomer("95576031857").GetName());
        EXPECT_EQ(customer2.GetName(), library.FindCustomer("95871034857").GetName());
    }
};
LibraryFindCustomerTest library_findcustomer_test;

} // namespace
