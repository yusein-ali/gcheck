#include "customer.hpp"

/*BEGIN SOLUTION*/
#include <iostream>

Customer::Customer(const std::string& name, const std::string& id)
    : name_(name), id_(id), loans_() {}

std::string Customer::GetName() const { return name_; }

std::string Customer::GetID() const { return id_; }

int Customer::GetLoanAmount() const { return loans_.size(); }

std::vector<Book> Customer::GetLoans() const { return loans_; }

bool Customer::LoanBook(Book& b) {
  if (b.Loan()) {
    loans_.push_back(b);
    return true;
  }
  return false;
}

void Customer::ReturnBook(Book& b) {
  for (auto it = loans_.begin(); it != loans_.end(); ++it)
    if (it->GetISBN() == b.GetISBN()) {
      it->Restore();
      loans_.erase(it);
      break;
    }
}

void Customer::Print() const {
  std::cout << "Customer: " << name_ << ", " << id_ << ", has "
            << GetLoanAmount() << " books on loan:" << std::endl;
  for (auto& b : loans_) {
    std::cout << "- ";
    b.Print();
  }
}

/*END SOLUTION*/
