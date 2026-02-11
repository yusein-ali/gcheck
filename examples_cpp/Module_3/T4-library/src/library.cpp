#include "library.hpp"

#include <iostream>

#include "book.hpp"
#include "customer.hpp"

/*BEGIN SOLUTION*/
Library::Library(const std::string& name) : name_(name) {}

std::string Library::GetName() const { return name_; }

std::vector<Book>& Library::GetBooks() { return books_; }

std::vector<Customer>& Library::GetCustomers() { return customers_; }

Book Library::FindBookByName(const std::string& name) {
  for (auto& b : books_)
    if (b.GetName() == name) return b;
  return Book("Not found!", "Not found!", "0000");
}

std::vector<Book> Library::FindBooksByAuthor(const std::string& name) {
  std::vector<Book> b;
  for (auto& book : books_)
    if (book.GetAuthor() == name) b.push_back(book);
  return b;
}

std::vector<Book> Library::FindAllLoanedBooks() {
  std::vector<Book> b;
  for (auto& book : books_)
    if (book.GetStatus() == true) b.push_back(book);
  return b;
}

Customer Library::FindCustomer(const std::string& id) {
  for (auto& c : customers_)
    if (c.GetID() == id) return c;
  return Customer("Not found!", "0000");
}
/*END SOLUTION*/
