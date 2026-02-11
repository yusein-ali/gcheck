#ifndef ELEC_AS_CPP_LIBRARY
#define ELEC_AS_CPP_LIBRARY

#include <string>
#include <vector>

#include "book.hpp"
#include "customer.hpp"

class Library {
 public:
  /**
   * \brief Construct a new Library object
   *
   * \param name the library's name (const reference to string)
   */
  /*BEGIN SOLUTION*/
  Library(const std::string& name);
  /*END SOLUTION*/

  /**
   * \brief Get the library's name as a string, takes no parameters.
   *
   * This function should not alter the Library object's state, in other words
   * the function should be const.
   *
   * Function identifier: GetName
   *
   * \return std::string
   */
  /*BEGIN SOLUTION*/
  std::string GetName() const;
  /*END SOLUTION*/

  /**
   * \brief Get the Library's books as a reference to a vector<Book>, takes no
   * parameters.
   *
   * Function identifier: GetBooks
   *
   * \return std::vector<Book>&
   */
  /*BEGIN SOLUTION*/
  std::vector<Book>& GetBooks();
  /*END SOLUTION*/

  /**
   * \brief Get the Library's customers as a reference to a vector<Customer>,
   * takes no parameters.
   *
   * Function identifier: GetCustomers
   *
   * \return std::vector<Customer>&
   */
  /*BEGIN SOLUTION*/
  std::vector<Customer>& GetCustomers();
  /*END SOLUTION*/

  /**
   * \brief Searches for a book by its name.
   *
   * Function identifier: FindBookByName
   *
   * \param name the name of the book (a const reference to a string)
   *
   * \return a copy of the found Book object if it can be found. If a Book is
   * not found, a new book with an empty strings as parameters for the name,
   * author and isbn is returned.
   */
  /*BEGIN SOLUTION*/
  Book FindBookByName(const std::string& name);
  /*END SOLUTION*/

  /**
   * \brief Searches for books by their author.
   *
   * Function identifier: FindBooksByAuthor
   *
   * \param name the author name (a const reference to a string)
   *
   * \return a vector of Books with the specified author. If no books are found,
   * an empty vector is returned.
   */
  /*BEGIN SOLUTION*/
  std::vector<Book> FindBooksByAuthor(const std::string& name);
  /*END SOLUTION*/

  /**
   * \brief Returns  a vector of loaned Books, takes no parameters.
   *
   * Function identifier: FindAllLoanedBooks
   *
   * \return a vector of loaned Books. If no books are found,
   * an empty vector is returned.
   */
  /*BEGIN SOLUTION*/
  std::vector<Book> FindAllLoanedBooks();
  /*END SOLUTION*/

  /**
   * \brief Searches for a customer with specfied id
   *
   * Function identifier: FindCustomer
   *
   * \param id the id of the customer (a const reference to a string)
   *
   * \return A copy of the found customer. if a Customer is not found, a new
   * Customer with empty name and id is returned.
   */
  /*BEGIN SOLUTION*/
  Customer FindCustomer(const std::string& id);
  /*END SOLUTION*/

 private:
  /* Member variables:
   * name (string)
   * books (vector<Book>)
   * customers (vector<Customer>)
   */
  /*BEGIN SOLUTION*/
  std::string name_;
  std::vector<Book> books_;
  std::vector<Customer> customers_;
  /*END SOLUTION*/
};

#endif
