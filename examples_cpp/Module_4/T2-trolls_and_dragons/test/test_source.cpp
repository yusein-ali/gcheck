#include <gcheck/argument.h>
#include <gcheck/tests.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "creature.hpp"
#include "dragon.hpp"
#include "troll.hpp"

TEST(Troll, print) {
  gcheck::StdoutCapturer tout;

  Troll troll("Diiba", 10), troll2("Urkki", 15);
  std::cout << "Trolls are: " << troll << " and " << troll2 << std::endl;

  tout.Restore();
  std::string Trolls_out = tout.str();

  std::cout << "Testing output" << std::endl;
  std::cout << Trolls_out << std::endl;

  std::string correct_answer =
      "Trolls are: Troll Diiba with 10 HP and Troll Urkki with 15 HP\n";
  EXPECT_EQ(correct_answer, Trolls_out);
}

TEST(Dragon, print) {
  gcheck::StdoutCapturer tout;

  Dragon dragon("Rhaegal", 50), dragon2("Viserion", 55);
  std::cout << "Dragons are: " << dragon << " and " << dragon2 << std::endl;

  tout.Restore();
  std::string Dragons_out = tout.str();

  std::cout << "Testing output" << std::endl;
  std::cout << Dragons_out << std::endl;

  std::string correct_answer =
      "Dragons are: Dragon Rhaegal with 50 HP and Dragon Viserion with 55 HP\n";
  EXPECT_EQ(correct_answer, Dragons_out);
}