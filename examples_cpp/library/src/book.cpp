#include "book.hpp"

#include <ctime>
#include <iostream>

/*BEGIN SOLUTION*/
Book::Book(const std::string& name, const std::string& author,
           const std::string& isbn, bool loaned, Date due_date)
    : name_(name),
      author_(author),
      isbn_(isbn),
      loaned_(loaned),
      due_date_(due_date) {}

std::string Book::GetName() const { return name_; }

std::string Book::GetAuthor() const { return author_; }

std::string Book::GetISBN() const { return isbn_; }

bool Book::GetStatus() const { return loaned_; }

Date Book::GetDueDate() const { return due_date_; }

bool Book::Loan() {
  if (loaned_) return false;
  loaned_ = true;

  due_date_ = Date::Today();
  due_date_.month++;

  return true;
}

void Book::Restore() {
  loaned_ = false;
  due_date_.day = 0;
  due_date_.month = 0;
  due_date_.year = 0;
}

void Book::Print() const {
  std::cout << "Book: " << name_ << ", author: " << author_
            << ", ISBN: " << isbn_ << ", loaned "
            << (loaned_ ? "true" : "false") << ", due date: " << due_date_.day
            << "." << due_date_.month << "." << due_date_.year << std::endl;
}

/*END SOLUTION*/
