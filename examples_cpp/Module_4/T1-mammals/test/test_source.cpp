#include <gcheck/argument.h>
#include <gcheck/tests.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cat.hpp"
#include "dog.hpp"
#include "mammal.hpp"

struct Res {
  double weight;
  std::string name;
};

std::string to_string(const Res& res) {
  return std::string("Age: ") + std::to_string(res.weight) +
         ", Name: " + res.name;
}

// BEGIN SERVER INTERNAL
bool operator==(const Res& l, const Res& r) {
  return l.weight == r.weight && l.name == r.name;
}

Res CorrectDog(double weight, std::string name) { return Res{weight, name}; }

Res CorrectCat(double weight, std::string name) { return Res{weight, name}; }
// END SERVER INTERNAL

bool operator==(const Res& l, const std::string& r) {
  return to_string(l) == r;
}

Res TestConstructDog(double weight, std::string name) {
  Dog dog(name, weight);
  return Res({dog.GetWeight(), dog.GetName()});
}

Res TestConstructCat(double weight, std::string name) {
  Cat cat(name, weight);
  return Res({cat.GetWeight(), cat.GetName()});
}

TEST(Dog, Constructor) {
  gcheck::Random<double> rnd_weight(3.0, 25.0);
  gcheck::Random<std::string> rnd_name(
      std::vector<std::string>{"Rontti", "Mortti", "Rekku", "Kankkunen"});

  CompareWithCallable(5, CorrectDog, TestConstructDog, rnd_weight, rnd_name);
}

TEST(Cat, Constructor) {
  gcheck::Random<double> rnd_weight(3.0, 25.0);
  gcheck::Random<std::string> rnd_name(
      std::vector<std::string>{"Kissi", "Katti", "Matikainen", "Qwertyuiop"});

  CompareWithCallable(5, CorrectCat, TestConstructCat, rnd_weight, rnd_name);
}

TEST(Dog, MakeSound) { EXPECT_EQ("Wuff!", Dog::MakeSound()); }

TEST(Cat, MakeSound) { EXPECT_EQ("Meow", Cat::MakeSound()); }