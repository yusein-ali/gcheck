#ifndef AALTO_ELEC_CPP_AVIARY_CLASS
#define AALTO_ELEC_CPP_AVIARY_CLASS

#include <iostream>

#include "bird.hpp"

/**
 * \brief Class for storing different birds and interacting with them.
 *
 * If somebody tries to copy an instance of this class, the compilation must
 * fail i.e. TODO: disallow copying
 */
class Aviary {
 public:
  Aviary() {}

  /**
   * \brief TODO: method Add adds the given bird to the aviary.
   *
   * If the pointer is NULL, std::logic_error is thrown. When a bird is added
   * to the aviary, it's ownership is transferred. When the aviary is
   * destroyed, all the birds are destroyed too.
   *
   * \param bird A pointer to a bird object.
   * \return nothing
   */
  // BEGIN SOLUTION
  void Add(Bird* bird);
  // END SOLUTION

  /**
   * \brief TODO: method SpeakAll calls the Speak method with the given
   * parameter on all birds in the aviary.
   *
   * Does not modify the object, thus it should be const.
   *
   * \param os A reference to the desired ostream.
   * \return nothing
   */
  // BEGIN SOLUTION
  void SpeakAll(std::ostream& os) const;
  // END SOLUTION

  /**
   * \brief TODO: method Size returns the number of birds in the aviary.
   *
   * Method takes no arguments and does not modify the object, thus it should
   * be const.
   *
   * \return How many birds there are in the aviary.
   */
  // BEGIN SOLUTION
  size_t Size() const;
  // END SOLUTION

  /**
   * \brief TODO: non-const version of the indexing operator [].
   *
   * If the index is out of bounds, you can optionally throw std::out_of_range exception. 
   *
   * \param idx desired index as an size_t integer value.
   * \return A pointer to the bird (Bird*) at the index given as an argument
   */
  // BEGIN SOLUTION
  Bird* operator[](size_t idx) { return birds_.at(idx); }
  // END SOLUTION

  /**
   * \brief TODO: const version of the indexing operator [].
   *
   * If the index is out of bounds, std::out_of_range is thrown.
   *
   * \param idx desired index as an size_t integer value.
   * \return A pointer to the bird (const Bird*) at the index given as an
   * argument
   */
  // BEGIN SOLUTION
  const Bird* operator[](size_t idx) const { return birds_.at(idx); }
  // END SOLUTION

  /**
   * \brief TODO: destructor. Takes no parameters.
   *
   * \return nothing
   */
  // BEGIN SOLUTION
  ~Aviary();
  // END SOLUTION
 private:
  // TODO: declare the required members
  /*BEGIN SOLUTION*/
  Aviary(const Aviary&);
  Aviary& operator=(const Aviary&);
  std::vector<Bird*> birds_;
  /*END SOLUTION*/
};

#endif