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

  S(S&&) noexcept { std::cout << "S moved!" << std::endl; };
  S& operator=(S&&)
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

  variant<S, S>                    var{in_place_index<1>};
  constexpr variant<int, int, int> var2{in_place_index<0>, 100};

  get<1>(var).s1 = "Hello";

  auto value = visit(
      [](int const&& i)
      {
        int x = static_cast<int>(50 + i);
        return x;
      },
      std::move(var2));
}
