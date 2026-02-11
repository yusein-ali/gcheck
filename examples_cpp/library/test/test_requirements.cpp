#include <concepts>
#include <string>
#include <vector>

#include "library.hpp"

namespace {

template <typename T>
concept HasRequiredBookApi =
    requires(const std::string& name,
             const std::string& author,
             const std::string& isbn,
             bool loaned,
             Date due_date) {
      { T{name, author, isbn, loaned, due_date} } -> std::same_as<T>;
      { T{name, author, isbn, loaned} } -> std::same_as<T>;
      { T{name, author, isbn} } -> std::same_as<T>;

      static_cast<std::string (T::*)() const>(&T::GetName);
      static_cast<std::string (T::*)() const>(&T::GetAuthor);
      static_cast<std::string (T::*)() const>(&T::GetISBN);
      static_cast<bool (T::*)() const>(&T::GetStatus);
      static_cast<Date (T::*)() const>(&T::GetDueDate);
      static_cast<bool (T::*)()>(&T::Loan);
      static_cast<void (T::*)()>(&T::Restore);
      static_cast<void (T::*)() const>(&T::Print);
    };

template <typename T>
concept HasRequiredCustomerApi =
    requires(const std::string& name, const std::string& id, Book& b) {
      { T{name, id} } -> std::same_as<T>;

      static_cast<std::string (T::*)() const>(&T::GetName);
      static_cast<std::string (T::*)() const>(&T::GetID);
      static_cast<int (T::*)() const>(&T::GetLoanAmount);
      static_cast<std::vector<Book> (T::*)() const>(&T::GetLoans);
      static_cast<bool (T::*)(Book&)>(&T::LoanBook);
      static_cast<void (T::*)(Book&)>(&T::ReturnBook);
      static_cast<void (T::*)() const>(&T::Print);
    };

template <typename T>
concept HasRequiredLibraryApi =
    requires(const std::string& name, const std::string& key) {
      { T{name} } -> std::same_as<T>;

      static_cast<std::string (T::*)() const>(&T::GetName);
      static_cast<std::vector<Book>& (T::*)()>(&T::GetBooks);
      static_cast<std::vector<Customer>& (T::*)()>(&T::GetCustomers);
      static_cast<Book (T::*)(const std::string&)>(&T::FindBookByName);
      static_cast<std::vector<Book> (T::*)(const std::string&)>(&T::FindBooksByAuthor);
      static_cast<std::vector<Book> (T::*)()>(&T::FindAllLoanedBooks);
      static_cast<Customer (T::*)(const std::string&)>(&T::FindCustomer);
    };

static_assert(HasRequiredBookApi<Book>,
              "Book must provide the required constructor overload set and member signatures");
static_assert(HasRequiredCustomerApi<Customer>,
              "Customer must provide required constructor and member signatures");
static_assert(HasRequiredLibraryApi<Library>,
              "Library must provide required constructor and member signatures");

}  // namespace
