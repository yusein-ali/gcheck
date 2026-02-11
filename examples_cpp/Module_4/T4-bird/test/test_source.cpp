#include <gcheck/tests.h>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "aviary.hpp"
//#include "bird.hpp"
#include "duck.hpp"
#include "owl.hpp"

enum Type : int { DUCK, OWL, ERROR };

std::string to_string(Type t) {
  switch (t) {
    case DUCK:
      return "Duck";
    case OWL:
      return "Owl";
    default:
      return "ERROR";
  }
}
std::string to_string(const Owl& b) {
  return "\"" + b.GetName() + "\" the owl";
}
std::string to_string(const Duck& b) {
  return "\"" + b.GetName() + "\" the duck";
}
std::string to_string(const Bird* b) {
  if (auto c = dynamic_cast<const Owl*>(b)) {
    return to_string(*c);
  } else if (auto c = dynamic_cast<const Duck*>(b)) {
    return to_string(*c);
  }
  return "Unknown bird type named " + b->GetName();
}
std::string to_string(const Bird& b) { return to_string(&b); }

std::string to_construct(const Owl& b) {
  return "Owl(\"" + b.GetName() + "\")";
}
std::string to_construct(const Duck& b) {
  return "Duck(\"" + b.GetName() + "\")";
}
std::string to_construct(const Bird& b) {
  if (auto c = dynamic_cast<const Owl*>(&b)) {
    return to_construct(*c);
  } else if (auto c = dynamic_cast<const Duck*>(&b)) {
    return to_construct(*c);
  }
  throw std::runtime_error("Bird type not recognized");
}
std::string to_construct(const Bird* b) { return "new " + to_construct(*b); }

gcheck::Random<std::string> names({"Esko", "Orly", "Alfred", "Seppo",
                                   "Albert"});
gcheck::Random types(std::vector<Type>{DUCK, OWL});

// BEGIN SERVER INTERNAL
std::string CConstruct(const std::string& name) { return name; }
// END SERVER INTERNAL
std::string TConstructDuck(const std::string& name) {
  return Duck(name).GetName();
}
std::string TConstructOwl(const std::string& name) {
  return Duck(name).GetName();
}

TEST(Bird, constructor, 1) {
  CompareWithCallable(5, CConstruct, TConstructDuck, names);
  CompareWithCallable(5, CConstruct, TConstructOwl, names);
}

// BEGIN SERVER INTERNAL
std::string CSpeakDuck(const std::string& name) { return name + ": QUACK\n"; }
std::string CSpeakOwl(const std::string& name) { return name + ": WHUU\n"; }
// END SERVER INTERNAL
std::string TSpeakDuck(const std::string& name) {
  std::stringstream ss;
  Duck(name).Speak(ss);
  return ss.str();
}
std::string TSpeakOwl(const std::string& name) {
  std::stringstream ss;
  Owl(name).Speak(ss);
  return ss.str();
}

TEST(Birds, Speak, 2, "Bird.constructor") {
  CompareWithCallable(5, CSpeakDuck, TSpeakDuck, names);
  CompareWithCallable(5, CSpeakOwl, TSpeakOwl, names);
}

TEST(Aviary, Add_and_indexing_operator, 3, "Bird.constructor") {
  Aviary aviary;
  std::vector<Bird*> added;
  for (int i = 0; i < 10; i++) {
    Bird* b = types.Next() == DUCK ? (Bird*)new Duck(names.Next())
                                   : (Bird*)new Owl(names.Next());
    aviary.Add(b);
    added.push_back(b);
  }

  ASSERT_TRUE(10 == aviary.Size());
  for (int i = 0; i < 10; i++) {
    Bird* bird_that_was_added = added[i];
    EXPECT_EQ(bird_that_was_added, aviary[i]);
  }
}

TEST(Aviary, SpeakAll, 2, "Add_and_indexing_operator") {
  std::cout << "local a" << std::endl;
  Aviary* a = new Aviary();
  a->Add(new Duck("Esko"));
  std::cout << "add a Esko" << std::endl;
  a->Add(new Owl("Orly"));
  a->Add(new Duck("Alfred"));

  std::stringstream out;

  a->SpeakAll(out);
  std::cout << "a spekAll" << std::endl;
  std::string SpeakAll_out = out.str();
  std::cout << SpeakAll_out << std::endl;

  std::string correct =
      "Esko: QUACK\n"
      "Orly: WHUU\n"
      "Alfred: QUACK\n";

  EXPECT_EQ(correct, SpeakAll_out);

  out.str("");
  a->Add(new Owl("Seppo"));
  a->Add(new Duck("Albert"));

  a->SpeakAll(out);
  SpeakAll_out = out.str();

  correct =
      "Esko: QUACK\n"
      "Orly: WHUU\n"
      "Alfred: QUACK\n"
      "Seppo: WHUU\n"
      "Albert: QUACK\n";

  EXPECT_EQ(correct, SpeakAll_out);
  delete a;
}

TEST(Aviary, Fly, 2, "Add_and_indexing_operator") {
  Aviary a;
  a.Add(new Duck("Esko"));
  a.Add(new Owl("Orly"));
  a.Add(new Duck("Alfred"));

  std::stringstream out;

  for (size_t i = 0; i < a.Size(); i++) a[i]->Fly(out);

  std::string SpeakAll_out = out.str();

  std::string correct =
      "Esko flies!\n"
      "Orly flies!\n"
      "Alfred flies!\n";

  EXPECT_EQ(correct, SpeakAll_out);

  out.str("");
  a.Add(new Owl("Seppo"));
  a.Add(new Duck("Albert"));

  for (size_t i = 0; i < a.Size(); i++) a[i]->Fly(out);

  SpeakAll_out = out.str();

  correct =
      "Esko flies!\n"
      "Orly flies!\n"
      "Alfred flies!\n"
      "Seppo flies!\n"
      "Albert flies!\n";

  EXPECT_EQ(correct, SpeakAll_out);
}

namespace {
template <class>
struct sfinae_bool {};
template <class T>
struct sfinae_true : sfinae_bool<T>, std::true_type {};
template <class T>
struct sfinae_false : sfinae_bool<T>, std::false_type {};
}  // namespace

namespace detail {
template <class T>
static auto is_const(int)
    -> sfinae_true<decltype(std::declval<const T&>().SpeakAll(std::cout))>;
template <class T>
static auto is_const(long) -> sfinae_false<T>;

template <class T>
static auto has_copy_constructor(int)
    -> sfinae_true<decltype(T(std::declval<T>()))>;
template <class T>
static auto has_copy_constructor(long) -> sfinae_false<T>;

template <class T>
static auto has_copy_assignment(int)
    -> sfinae_true<decltype(std::declval<T>() = std::declval<T>())>;
template <class T>
static auto has_copy_assignment(long) -> sfinae_false<T>;
}  // namespace detail

template <class T>
struct is_const : decltype(detail::is_const<T>(0)) {};
template <class T>
struct has_copy_constructor_t : decltype(detail::has_copy_constructor<T>(0)) {};
template <class T>
struct has_copy_assignment_t : decltype(detail::has_copy_assignment<T>(0)) {};

TEST(Aviary, copy_assignment_constructor, 2) {
  bool has_copy_constructor = has_copy_constructor_t<Aviary>();
  bool has_copy_assignment = has_copy_assignment_t<Aviary>();
  EXPECT_FALSE(has_copy_constructor)
      << "You might have to 'make clean' (ctrl + shift + b -> Clean test "
         "Bird)\nfor any changes to this to take effect when running locally";
  EXPECT_FALSE(has_copy_assignment)
      << "You might have to 'make clean' (ctrl + shift + b -> Clean test "
         "Bird)\nfor any changes to this to take effect when running locally";
}
