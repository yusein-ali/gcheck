#include "vector_it.hpp"

#include <iostream>
#include <vector>

// Implement the functions here

std::vector<int> ReadVector() {
  std::vector<int> v;
  /*BEGIN SOLUTION*/
  int t = 0;
  while (std::cin >> t) v.push_back(t);
  /*END SOLUTION*/
  return v;
}

void PrintSum1(const std::vector<int>& v) {
  /*BEGIN SOLUTION*/
  for (auto it = v.begin(); it < v.end() - 1; it++)
    std::cout << *it + *(it + 1) << " ";

  std::cout << std::endl;
  /*END SOLUTION*/
}

void PrintSum2(const std::vector<int>& v) {
  /*BEGIN SOLUTION*/
  auto b = v.begin(), e = v.end() - 1;
  for (; e > b; ++b, --e) std::cout << *b + *e << " ";
  std::cout << std::endl;
  /*END SOLUTION*/
}
