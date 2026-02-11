#include "list.hpp"

#include <iostream>

// Implement the functions here

std::istream& GetLines(std::istream& is, std::list<std::string>& list) {
  /*BEGIN SOLUTION*/
  list.clear();
  for (std::string line; std::getline(is, line); list.push_back(line)) {
  }
  return is;
  /*END SOLUTION*/
}

void Print(const std::list<std::string>& list) {
  /*BEGIN SOLUTION*/
  for (std::list<std::string>::const_iterator it = list.begin();
       it != list.end(); ++it) {
    std::cout << *it << std::endl;
  }
  /*END SOLUTION*/
}

void SortAndUnique(std::list<std::string>& list) {
  /*BEGIN SOLUTION*/
  list.sort();
  list.unique();
  /*END SOLUTION*/
}