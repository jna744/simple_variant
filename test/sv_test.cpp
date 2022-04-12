#include <functional>
#include <iostream>
#include <simple/mp/type_name.hpp>
#include <simple/variant.hpp>
#include <string>

struct S {

  S() { std::cout << "S constructed!" << std::endl; }

  S(S const&) { std::cout << "S copied!" << std::endl; };
  S& operator=(S const&)
  {
    std::cout << "S copy assigned!" << std::endl;
    return *this;
  }

  S(S&&) noexcept { std::cout << "S moved!" << std::endl; };
  S& operator=(S&&)
  {
    std::cout << "S move assigned!" << std::endl;
    return *this;
  }

  ~S() { std::cout << "S destroyed" << std::endl; }

  std::string s1;
};

struct L1 {
  template <typename T>
  constexpr auto operator()(T t) const
  {
    return t + 50;
  }
};

template <typename T>
void print()
{
  std::cout << simple::mp::m_type_name<T>() << std::endl;
}

int main()
{

  using namespace simple;

  variant<std::string> v("abc");  // OK
  // // variant<std::string, std::string> w("abc");  // ill-formed
  variant<std::string, const char*> x("abc");                // OK, chooses const char*
  variant<std::string, bool> y(in_place_type<bool>, false);  // OK, chooses string; bool
                                                             // is not a candidate
  variant<float, long, double> z = 0;                        // OK, holds long
  //                                         // float and double are not candidates
  //
  //
  std::cout << y.index() << std::endl;
  y = "Hello";
  std::cout << y.index() << std::endl;
}
