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

  variant<std::string, int> v = "abc";
  variant<std::string, int> v2 = 0;

  if (v == v2) {
    std::cout << "yay!" << std::endl;
  } else {
    std::cout << "nay!" << std::endl;
  }
}
