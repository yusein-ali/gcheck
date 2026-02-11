#ifndef AALTO_ELEC_CPP_CAT_CLASS
#define AALTO_ELEC_CPP_CAT_CLASS

#include "mammal.hpp"

/**
 * \brief Inherits class Mammal.
 *
 * Class Cat implements additionally the following features:
 *
 * 1. a constructor with two arguments, first being the name of the animal and
 * the second its weight.
 *
 * 2. an accessor function **GetName** that returns the name of the creature.
 *
 * 3. a static function **MakeSound** that returns a string that contains the
 * sound that the creature makes. For a dog, the function should return the
 * string `Meow`.
 *
 */
class Cat : public Mammal {
  /*BEGIN SOLUTION*/
 public:
  Cat(const std::string &name, double weight);
  const std::string &GetName() const;
  static const std::string &MakeSound();

 private:
  const std::string name_;
  static std::string sound_;
  /*END SOLUTION*/
};

#endif
