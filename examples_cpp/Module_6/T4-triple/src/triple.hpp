/* Add include guards here */

/**
 * @brief: The template class Triple is a container class, which holds three values.
 * All of the values can be of different type. All of the values should be
 * private.
 *
 * The constructor of the class takes three const references to values as a
 * parameter. The parameters are in order, in other words, after construction
 * the first parameter should be accessible through the public member function
 * First, the second parameter through the public member function Second, and
 * the third through the public member function Third.
 *
 * The default empty constructor should also work.
 *
 * The Triple class should also support the copy constructor, and assignment
 * operations.
 */

/* Public member functions:
 * ------------------------
 * The Triple class has three public member functions, which are used to
 * access the stored elements. All three should work with const instances
 * of Triple.
 *
 * Function: First
 * The function takes no parameters.
 * The function returns a const reference to the first value of the triple.
 *
 * Function: Second
 * The function takes no parameters.
 * The function returns a const reference to the second value of the triple.
 *
 * Function: Third
 * The function takes no parameters.
 * The function returns a const reference to the third value of the triple.
 */

/* Non-member functions:
 * ---------------------
 * Function: IsHomogenous
 * The function is a non-member function that takes a Triple as a parameter.
 * The function returns a boolean value indicating whether the triple is
 * homogenous or not. A triple is homogenous if all of its three values are of
 * the same type.
 * Hint: specialisation can help you here.
 */

/* Operator overloads:
 * -------------------
 * The triple class should support the following operators:
 *
 * Assignment operator, =
 * The default functionality should do here.
 *
 * Equality operator, ==
 * You can assume that all the values in the triple can be compared by the
 * == -operator. Two triples are equal if the corresponding values in both
 * of the triples are equal.
 *
 * Inequality operator, !=
 * You can assume that all the values in the triple can be compared by the
 * != -operator. Two triples are inequal if even one corresponding value in
 * the triples is different.
 *
 * The output stream operator, <<
 * You can assume that all the values in the triple support the << -operator.
 * The triple should be printed to the output stream in the following format:
[<first value>, <second value>, <third value>]
 * Note that there is NO new line at the end of the output.
 */

/* Remember the endif here to close the header guards */
/*BEGIN SOLUTION*/
#ifndef AALTO_ELEC_CPP_TRANSCRIPT_CLASS
#define AALTO_ELEC_CPP_TRANSCRIPT_CLASS

#include <ostream>

template <typename T1, typename T2, typename T3>
class Triple
{
public:
    // these aren't required but are a good thing to have
    typedef T1 Type1;
    typedef T2 Type2;
    typedef T3 Type3;

    // Constructor
    // Take the parameters as const references to avoid extra copies and
    // allow usage of consts as parameters
    Triple(const T1 &param1, const T2 &param2, const T3 &param3)
        : val1_(param1), val2_(param2), val3_(param3) {}

    // Default, empty- and copy constructors will do, so no need to
    // declare them

    // Accessors
    const T1 &First() const { return val1_; }
    const T2 &Second() const { return val2_; }
    const T3 &Third() const { return val3_; }

private:
    T1 val1_;
    T2 val2_;
    T3 val3_;
};

// Base version of IsHomogenous for any Triple
template <typename T1, typename T2, typename T3>
bool IsHomogenous(const Triple<T1, T2, T3> &)
{
    return false;
}

// Specialization where all the three types are the same
// i.e. if the types of the Triple are the same, this
// function will be called instead of the above one
template <typename T>
bool IsHomogenous(const Triple<T, T, T> &)
{
    return true;
}

/* An alternative implementation of IsHomogenous using std::is_same
#include <type_traits>
template <typename T1, typename T2, typename T3>
bool IsHomogenous(const Triple<T1, T2, T3>&) {
    return std::is_same<T1,T2>() && std::is_same<T1,T3>() && std::is_same<T2,T3>();
}
*/

// Overload !=, ==, and << -operators
template <typename T1, typename T2, typename T3>
bool operator==(const Triple<T1, T2, T3> &l, const Triple<T1, T2, T3> &r)
{
    return l.First() == r.First() && l.Second() == r.Second() && l.Third() == r.Third();
}

template <typename T1, typename T2, typename T3>
bool operator!=(const Triple<T1, T2, T3> &l, const Triple<T1, T2, T3> &r)
{
    return !(l == r);
}

template <typename T1, typename T2, typename T3>
std::ostream &operator<<(std::ostream &out, const Triple<T1, T2, T3> &triple)
{
    return out << "[" << triple.First() << ", " << triple.Second() << ", " << triple.Third() << "]";
}

#endif
/*END SOLUTION*/
