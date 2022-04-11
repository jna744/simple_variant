#include <simple/variant.hpp>

#include <functional>
#include <iostream>
#include <string>

struct S {

  S() { std::cout << "S constructed!" << std::endl; }

  S(S const&) { std::cout << "S copied!" << std::endl; };
  S& operator=(S const&)
  {
    std::cout << "S copy assigned!" << std::endl;
    return *this;
  }

  S(S &&) noexcept { std::cout << "S moved!" << std::endl; };
  S& operator=(S &&)
  {
    std::cout << "S move assigned!" << std::endl;
    return *this;
  }

  ~S() { std::cout << "S destroyed" << std::endl; }

  std::string s1;
};

int main()
{

  using namespace simple;

  variant<S, S> var{in_place_index<1>};

  constexpr variant<int, unsigned char> var10{in_place_index<0>, 100};

  auto varcasdasd = var10;
  auto trold = varcasdasd;
  varcasdasd = std::move(trold);

  auto var2{var};

  var2 = var;

  var = std::move(var2);


  std::cout << var.index() << std::endl;
}
