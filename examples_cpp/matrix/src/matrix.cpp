#include "matrix.hpp"

#include <iostream>
#include <vector>

// Implement the functions here
// Remember to use auto and ranged for-loops when they can be used!

// Assumes 'm' is a square matrix
Matrix ReadMatrix(int n) {
  // BEGIN SOLUTION
  Matrix m;
  int y = n, x = n;
  while (y--) {
    std::vector<int> t;
    while (x--) {
      int i;
      std::cin >> i;
      t.push_back(i);
    }
    m.push_back(t);
    x = n;
  }
  return m;
  // END SOLUTION
}

Matrix Rotate90Deg(const Matrix &m) {
  // BEGIN SOLUTION
  Matrix v(m.size());

  for (size_t i = 0; i < m.size(); ++i) v[i] = std::vector<int>(m.size());

  for (size_t i = 0; i < m.size(); ++i)
    for (size_t j = 0; j < m.size(); ++j) v[j][m.size() - i - 1] = m[i][j];

  return v;
  // END SOLUTION
}

void Print(const Matrix &m) {
  // BEGIN SOLUTION
  std::cout << "Printing out a " << m.size() << " x " << m.size()
            << " matrix:" << std::endl;
  for (auto &i : m) {
    for (auto &j : i) std::cout << j << " ";
    std::cout << std::endl;
  }
  // END SOLUTION
}
