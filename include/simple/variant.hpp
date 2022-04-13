#ifndef SIMPLE_VARIANT_HPP
#define SIMPLE_VARIANT_HPP

#if __cplusplus >= 201703L

#define SVAR_INLINE_17 inline
#define SVAR_INLINE_14

#elif __cplusplus >= 201402L

#define SVAR_INLINE_17
#define SVAR_INLINE_14 inline

#else

#error "simple::variant requires at least c++14"

#endif

#define SVAR_INLINE(version) SVAR_INLINE_##version

#include <simple/mp.hpp>

#include <exception>
#include <type_traits>

namespace simple
{

template <typename... Ts>
class variant;

struct monostate {
};

inline constexpr bool operator==(monostate, monostate) noexcept
{
  return true;
}

inline constexpr bool operator!=(monostate, monostate) noexcept
{
  return false;
}

inline constexpr bool operator<(monostate, monostate) noexcept
{
  return false;
}

inline constexpr bool operator>(monostate, monostate) noexcept
{
  return false;
}

inline constexpr bool operator<=(monostate, monostate) noexcept
{
  return true;
}

inline constexpr bool operator>=(monostate, monostate) noexcept
{
  return true;
}

class bad_variant_access : public std::exception
{
public:

  explicit bad_variant_access(char const* what) : what_{what} {}
  virtual ~bad_variant_access() = default;

  char const* what() const noexcept override { return what_; }

private:

  char const* what_;
};

#define SVAR_BAD_ACCESS(what) throw ::simple::bad_variant_access(what)

template <typename T>
struct variant_size;

template <typename... Ts>
struct variant_size<variant<Ts...>> : mp::m_size_t<sizeof...(Ts)> {
};

template <typename T>
struct variant_size<T const> : variant_size<T> {
};

template <typename T>
struct variant_size<T volatile> : variant_size<T> {
};

template <typename T>
struct variant_size<T const volatile> : variant_size<T> {
};

template <typename T>
SVAR_INLINE(17)
constexpr std::size_t variant_size_v = variant_size<T>::value;

template <std::size_t I, typename T>
struct variant_alternative;

template <std::size_t I, typename... Ts>
struct variant_alternative<I, variant<Ts...>> {
  using type = mp::m_at_c<variant<Ts...>, I>;
};

template <std::size_t I, typename T>
struct variant_alternative<I, T const>
  : std::add_const<mp::m_t_<variant_alternative<I, T>>> {
};

template <std::size_t I, typename T>
struct variant_alternative<I, T volatile>
  : std::add_volatile<mp::m_t_<variant_alternative<I, T>>> {
};

template <std::size_t I, typename T>
struct variant_alternative<I, T const volatile>
  : std::add_cv<mp::m_t_<variant_alternative<I, T>>> {
};

template <std::size_t I, typename T>
using variant_alternative_t = mp::m_t_<variant_alternative<I, T>>;

SVAR_INLINE(17) constexpr std::size_t variant_npos = std::size_t(-1);

template <typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template <typename T>
SVAR_INLINE(17)
constexpr in_place_type_t<T> in_place_type{};

template <std::size_t I>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

template <std::size_t I>
SVAR_INLINE(17)
constexpr in_place_index_t<I> in_place_index{};

namespace variant_ns
{

template <typename T>
struct is_in_place_type : mp::m_false {
};

template <typename T>
struct is_in_place_type<in_place_type_t<T>> : mp::m_true {
};

template <typename>
struct is_in_place_index : mp::m_false {
};

template <std::size_t I>
struct is_in_place_index<in_place_index_t<I>> : mp::m_true {
};

template <typename T>
struct is_variant : mp::m_false {
};

template <typename... Ts>
struct is_variant<variant<Ts...>> : mp::m_true {
};

namespace swap_ns
{

using std::swap;

template <typename T>
using swap_detected_t = decltype(swap(mp::m_declval<T>(), mp::m_declval<T>()));

template <typename T>
using is_swappable = mp::m_is_valid<swap_detected_t, T&>;

template <typename T>
using nothrow_swap_detected_t =
    mp::m_bool<noexcept(swap(mp::m_declval<T>(), mp::m_declval<T>()))>;

template <typename T>
using is_nothrow_swappable = mp::m_eval_or<mp::m_false, nothrow_swap_detected_t, T&>;

}  // namespace swap_ns

template <typename T>
struct is_swappable : swap_ns::is_swappable<T> {
};

template <typename T>
struct is_nothrow_swappable : swap_ns::is_nothrow_swappable<T> {
};

template <bool, typename... Ts>
union variant_storage_impl {
};

template <typename... Ts>
using variant_storage =
    variant_storage_impl<mp::m_all<std::is_trivially_destructible<Ts>...>::value, Ts...>;

template <typename T, typename... Ts>
union variant_storage_impl<true, T, Ts...> {

  template <std::size_t I, typename... Args>
  constexpr variant_storage_impl(mp::m_size_t<I>, Args&&... args)
    : rest_{mp::m_size_t<I - 1>{}, std::forward<Args>(args)...}
  {
  }

  template <typename... Args>
  constexpr variant_storage_impl(mp::m_size_t<0>, Args&&... args)
    : first_{std::forward<Args>(args)...}
  {
  }

  // trivial destructor
  // ~variant_storage_impl() = default;

  template <std::size_t I, typename... Args>
  constexpr auto& emplace(mp::m_size_t<I> index, Args&&... args)
  {
    return emplace_impl(
        mp::m_all<
            std::is_trivially_move_assignable<T>,
            std::is_trivially_move_assignable<Ts>...>{},
        index,
        std::forward<Args>(args)...);
  }

  template <std::size_t I>
  constexpr auto& get(mp::m_size_t<I>) & noexcept
  {
    return rest_.get(mp::m_size_t<I - 1>{});
  }

  constexpr T& get(mp::m_size_t<0>) & noexcept { return first_; }

  template <std::size_t I>
  constexpr auto const& get(mp::m_size_t<I>) const& noexcept
  {
    return rest_.get(mp::m_size_t<I - 1>{});
  }

  constexpr T const& get(mp::m_size_t<0>) const& noexcept { return first_; }

  template <std::size_t I>
  constexpr auto&& get(mp::m_size_t<I>) && noexcept
  {
    return std::move(rest_).get(mp::m_size_t<I - 1>{});
  }

  constexpr T&& get(mp::m_size_t<0>) && noexcept { return std::move(first_); }

  template <std::size_t I>
  constexpr auto const&& get(mp::m_size_t<I>) const&& noexcept
  {
    return std::move(rest_).get(mp::m_size_t<I - 1>{});
  }

  constexpr T const&& get(mp::m_size_t<0>) const&& noexcept { return std::move(first_); }

private:

  template <std::size_t I, typename... Args>
  constexpr auto& emplace_impl(mp::m_false, mp::m_size_t<I>, Args&&... args)
  {
    return rest_.emplace(mp::m_size_t<I - 1>{}, std::forward<Args>(args)...);
  }

  template <typename... Args>
  constexpr T& emplace_impl(mp::m_false, mp::m_size_t<0>, Args&&... args)
  {
    new (&first_) T(std::forward<Args>(args)...);
    return first_;
  }

  template <std::size_t I, typename... Args>
  constexpr auto& emplace_impl(mp::m_true, mp::m_size_t<I> index, Args&&... args)
  {
    *this = variant_storage_impl{index, std::forward<Args>(args)...};
    return get(index);
  }

  T                      first_;
  variant_storage<Ts...> rest_;
};

template <typename T, typename... Ts>
union variant_storage_impl<false, T, Ts...> {

public:

  template <std::size_t I, typename... Args>
  variant_storage_impl(mp::m_size_t<I>, Args&&... args)
    : rest_{mp::m_size_t<I - 1>{}, std::forward<Args>(args)...}
  {
  }

  template <typename... Args>
  variant_storage_impl(mp::m_size_t<0>, Args&&... args)
    : first_{std::forward<Args>(args)...}
  {
  }

  // non-trivial destructor
  ~variant_storage_impl() {}

  template <std::size_t I, typename... Args>
  auto& emplace(mp::m_size_t<I>, Args&&... args)
  {
    return rest_.emplace(mp::m_size_t<I - 1>{}, std::forward<Args>(args)...);
  }

  template <typename... Args>
  T& emplace(mp::m_size_t<0>, Args&&... args)
  {
    new (&first_) T(std::forward<Args>(args)...);
    return first_;
  }

  template <std::size_t I>
  auto& get(mp::m_size_t<I>) & noexcept
  {
    return rest_.get(mp::m_size_t<I - 1>{});
  }

  T& get(mp::m_size_t<0>) & noexcept { return first_; }

  template <std::size_t I>
  auto const& get(mp::m_size_t<I>) const& noexcept
  {
    return rest_.get(mp::m_size_t<I - 1>{});
  }

  T const& get(mp::m_size_t<0>) const& noexcept { return first_; }

  template <std::size_t I>
  auto&& get(mp::m_size_t<I>) && noexcept
  {
    return std::move(rest_).get(mp::m_size_t<I - 1>{});
  }

  T&& get(mp::m_size_t<0>) && noexcept { return std::move(first_); }

  template <std::size_t I>
  auto const&& get(mp::m_size_t<I>) const&& noexcept
  {
    return std::move(rest_).get(mp::m_size_t<I - 1>{});
  }

  T const&& get(mp::m_size_t<0>) const&& noexcept { return std::move(first_); }

private:

  T                      first_;
  variant_storage<Ts...> rest_;
};

struct dummy_type {
};

template <bool, typename... Ts>
class variant_base_impl;

template <typename... Ts>
using variant_base =
    variant_base_impl<mp::m_all<std::is_trivially_destructible<Ts>...>::value, Ts...>;

template <typename... Ts>
class variant_base_impl<true, Ts...>
{

  using storage_type = variant_storage<dummy_type, Ts...>;

public:

  constexpr variant_base_impl() : index_{variant_npos}, storage_{mp::m_size_t<0>{}} {}

  template <std::size_t I, typename... Args>
  constexpr variant_base_impl(mp::m_size_t<I>, Args&&... args)
    : index_{I + 1}, storage_{mp::m_size_t<I + 1>{}, std::forward<Args>(args)...}
  {
  }

  ~variant_base_impl() = default;

  constexpr std::size_t index() const noexcept { return this->index_ - 1; }

  constexpr bool valueless_by_exception() const noexcept
  {
    return index_ == variant_npos;
  }

  template <
      std::size_t I,
      typename... Args,
      typename T = variant_alternative_t<I, variant<Ts...>>,
      typename = mp::m_if<std::is_constructible<T, Args...>, void>>
  constexpr T&
  emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>{})
  {
    static_assert(I < sizeof...(Ts), "");
    return emplace_impl<I + 1>(
        std::is_nothrow_constructible<T, Args...>{}, std::forward<Args>(args)...);
  }

protected:

  template <std::size_t I, typename... Args>
  constexpr auto& emplace_impl(mp::m_true, Args&&... args)
  {
    index_ = I;
    return storage_.emplace(mp::m_size_t<I>{}, std::forward<Args>(args)...);
  }

  template <std::size_t I, typename... Args>
  constexpr auto& emplace_impl(mp::m_false, Args&&... args)
  {
    index_ = variant_npos;
    auto& value = storage_.emplace(mp::m_size_t<I>{}, std::forward<Args>(args)...);
    index_ = I;
    return value;
  }

  constexpr void destroy() noexcept { index_ = variant_npos; }

  template <std::size_t I, typename... Args>
  constexpr void init(mp::m_size_t<I>, Args&&... args)
  {
    new (&storage_) storage_type{mp::m_size_t<I + 1>{}, std::forward<Args>(args)...};
    index_ = I + 1;
  }

  template <std::size_t I>
  constexpr auto& get(mp::m_size_t<I>) & noexcept
  {
    // assert(I + 1 == index_);
    return storage_.get(mp::m_size_t<I + 1>{});
  }

  template <std::size_t I>
  constexpr auto const& get(mp::m_size_t<I>) const& noexcept
  {
    // assert(I + 1 == index_);
    return storage_.get(mp::m_size_t<I + 1>{});
  }

  template <std::size_t I>
  constexpr auto&& get(mp::m_size_t<I>) && noexcept
  {
    // assert(I + 1 == index_);
    return std::move(storage_).get(mp::m_size_t<I + 1>{});
  }

  template <std::size_t I>
  constexpr auto const&& get(mp::m_size_t<I>) const&& noexcept
  {
    // assert(I + 1 == index_);
    return std::move(storage_).get(mp::m_size_t<I + 1>{});
  }

private:

  std::size_t  index_;
  storage_type storage_;
};

template <typename... Ts>
class variant_base_impl<false, Ts...>
{

  using storage_type = variant_storage<dummy_type, Ts...>;

public:

  variant_base_impl() : index_{variant_npos}, storage_{mp::m_size_t<0>{}} {}

  template <std::size_t I, typename... Args>
  variant_base_impl(mp::m_size_t<I>, Args&&... args)
    : index_{I + 1}, storage_{mp::m_size_t<I + 1>{}, std::forward<Args>(args)...}
  {
  }

  ~variant_base_impl()
  {
    if (index_ != variant_npos)
      destroy();
  }

  std::size_t index() const noexcept { return index_ - 1; }

  bool valueless_by_exception() const noexcept { return index_ == variant_npos; }

  template <
      std::size_t I,
      typename... Args,
      typename T = variant_alternative_t<I, variant<Ts...>>,
      typename = mp::m_if<std::is_constructible<T, Args...>, void>>
  T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>{})
  {
    static_assert(I < sizeof...(Ts), "");
    if (index_ != variant_npos)
      destroy();
    auto& value = storage_.emplace(mp::m_size_t<I + 1>{}, std::forward<Args>(args)...);
    index_ = I + 1;
    return value;
  }

protected:

  struct destroy_fn {
    template <typename I, typename Variant>
    void operator()(I index, Variant* this_) const
    {
      using type = variant_alternative_t<I::value, variant<dummy_type, Ts...>>;
      this_->storage_.get(index).~type();
    }
  };

  void destroy() noexcept
  {
    mp::m_invoke_with_index<sizeof...(Ts) + 1>(index_, destroy_fn{}, this);
    index_ = variant_npos;
  }

  template <std::size_t I, typename... Args>
  void init(mp::m_size_t<I>, Args&&... args)
  {
    new (&storage_) storage_type{mp::m_size_t<I + 1>{}, std::forward<Args>(args)...};
    index_ = I + 1;
  }

  template <std::size_t I>
  auto& get(mp::m_size_t<I>) & noexcept
  {
    // assert(I + 1 == index_);
    return storage_.get(mp::m_size_t<I + 1>{});
  }

  template <std::size_t I>
  auto const& get(mp::m_size_t<I>) const& noexcept
  {
    // assert(I + 1 == index_);
    return storage_.get(mp::m_size_t<I + 1>{});
  }

  template <std::size_t I>
  auto&& get(mp::m_size_t<I>) && noexcept
  {
    // assert(I + 1 == index_);
    return std::move(storage_).get(mp::m_size_t<I + 1>{});
  }

  template <std::size_t I>
  auto const&& get(mp::m_size_t<I>) const&& noexcept
  {
    // assert(I + 1 == index_);
    return std::move(storage_).get(mp::m_size_t<I + 1>{});
  }

private:

  std::size_t  index_;
  storage_type storage_;
};

template <bool, bool, typename... Ts>
class variant_cc_base_impl;

template <typename... Ts>
using variant_cc_base = variant_cc_base_impl<
    mp::m_all<std::is_copy_constructible<Ts>...>::value,
    mp::m_all<std::is_trivially_copy_constructible<Ts>...>::value,
    Ts...>;

template <bool TriviallyCopyConstructible, typename... Ts>
class variant_cc_base_impl<false, TriviallyCopyConstructible, Ts...>
  : public variant_base<Ts...>
{
public:

  using variant_base<Ts...>::variant_base;

  variant_cc_base_impl(variant_cc_base_impl const&) = delete;
  variant_cc_base_impl& operator=(variant_cc_base_impl const&) = default;

  variant_cc_base_impl(variant_cc_base_impl&&) = default;
  variant_cc_base_impl& operator=(variant_cc_base_impl&&) = default;
};

template <typename... Ts>
class variant_cc_base_impl<true, true, Ts...> : public variant_base<Ts...>
{
public:

  using variant_base<Ts...>::variant_base;

  variant_cc_base_impl(variant_cc_base_impl const&) = default;
  variant_cc_base_impl& operator=(variant_cc_base_impl const&) = default;

  variant_cc_base_impl(variant_cc_base_impl&&) = default;
  variant_cc_base_impl& operator=(variant_cc_base_impl&&) = default;
};

template <typename... Ts>
class variant_cc_base_impl<true, false, Ts...> : public variant_base<Ts...>
{

private:

  struct copy_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_cc_base_impl& this_, variant_cc_base_impl const& other) const
    {
      this_.init(I{}, other.get(I{}));
    }
  };

public:

  using variant_base<Ts...>::variant_base;

  constexpr variant_cc_base_impl(variant_cc_base_impl const& other) noexcept(
      mp::m_all<std::is_nothrow_copy_constructible<Ts>...>::value)
    : variant_base<Ts...>{}
  {
    if (!other.valueless_by_exception())
      mp::m_invoke_with_index<sizeof...(Ts)>(other.index(), copy_fn{}, *this, other);
  }

  variant_cc_base_impl& operator=(variant_cc_base_impl const&) = default;

  variant_cc_base_impl(variant_cc_base_impl&&) = default;
  variant_cc_base_impl& operator=(variant_cc_base_impl&&) = default;
};

template <bool, bool, typename... Ts>
struct variant_ca_base_impl;

template <typename... Ts>
using variant_ca_base = variant_ca_base_impl<
    mp::m_all<std::is_copy_constructible<Ts>..., std::is_copy_assignable<Ts>...>::value,
    mp::m_all<
        std::is_trivially_copy_constructible<Ts>...,
        std::is_trivially_copy_assignable<Ts>...,
        std::is_trivially_destructible<Ts>...>::value,
    Ts...>;

template <bool TriviallyCopyAssignable, typename... Ts>
class variant_ca_base_impl<false, TriviallyCopyAssignable, Ts...>
  : public variant_cc_base<Ts...>
{
public:

  using variant_cc_base<Ts...>::variant_cc_base;

  variant_ca_base_impl(variant_ca_base_impl const&) = default;
  variant_ca_base_impl& operator=(variant_ca_base_impl const&) = delete;

  variant_ca_base_impl(variant_ca_base_impl&&) = default;
  variant_ca_base_impl& operator=(variant_ca_base_impl&&) = default;
};

template <typename... Ts>
class variant_ca_base_impl<true, true, Ts...> : public variant_cc_base<Ts...>
{
public:

  using variant_cc_base<Ts...>::variant_cc_base;

  variant_ca_base_impl(variant_ca_base_impl const&) = default;
  variant_ca_base_impl& operator=(variant_ca_base_impl const&) = default;

  variant_ca_base_impl(variant_ca_base_impl&&) = default;
  variant_ca_base_impl& operator=(variant_ca_base_impl&&) = default;
};

template <typename... Ts>
class variant_ca_base_impl<true, false, Ts...> : public variant_cc_base<Ts...>
{

  using is_valid_emplace = mp::m_any<
      mp::m_all<std::is_nothrow_copy_constructible<Ts>...>,
      mp::m_not<mp::m_all<std::is_nothrow_move_constructible<Ts>...>>>;

  struct copy_assign_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_ca_base_impl& this_, variant_ca_base_impl const& other) const
    {
      this_.get(I{}) = other.get(I{});
    }
  };

  struct copy_emplace_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_ca_base_impl& this_, variant_ca_base_impl const& other) const
    {
      this_.template emplace<I::value>(other.get(I{}));
    }
  };

  template <typename = void>
  constexpr static void
  do_emplace(mp::m_true, variant_ca_base_impl& this_, variant_ca_base_impl const& other)
  {
    mp::m_invoke_with_index<sizeof...(Ts)>(
        other.index(), copy_emplace_fn{}, this_, other);
  }

  template <typename = void>
  constexpr static void
  do_emplace(mp::m_false, variant_ca_base_impl& this_, variant_ca_base_impl const& other)
  {
    static_cast<variant<Ts...>&>(this_) =
        variant<Ts...>{static_cast<variant<Ts...> const&>(other)};
  }

public:

  using variant_cc_base<Ts...>::variant_cc_base;

  variant_ca_base_impl(variant_ca_base_impl const&) = default;

  variant_ca_base_impl& operator=(variant_ca_base_impl const& other) noexcept(
      mp::m_all<mp::m_all<
          std::is_nothrow_copy_constructible<Ts>,
          std::is_nothrow_copy_assignable<Ts>>...>::value)
  {
    if (this != &other) {
      if (this->index() == other.index()) {
        if (!this->valueless_by_exception()) {
          mp::m_invoke_with_index<sizeof...(Ts)>(
              this->index(), copy_assign_fn{}, *this, other);
        }
      } else {
        if (other.valueless_by_exception()) {
          this->destroy();
        } else {
          do_emplace(is_valid_emplace{}, *this, other);
        }
      }
    }
    return *this;
  }

  variant_ca_base_impl(variant_ca_base_impl&&) = default;
  variant_ca_base_impl& operator=(variant_ca_base_impl&&) = default;
};

template <bool, bool, typename... Ts>
class variant_mc_base_impl;

template <typename... Ts>
using variant_mc_base = variant_mc_base_impl<
    mp::m_all<std::is_move_constructible<Ts>...>::value,
    mp::m_all<std::is_trivially_move_constructible<Ts>...>::value,
    Ts...>;

template <bool TriviallyMoveConstructible, typename... Ts>
class variant_mc_base_impl<false, TriviallyMoveConstructible, Ts...>
  : public variant_ca_base<Ts...>
{
public:

  using variant_ca_base<Ts...>::variant_ca_base;

  variant_mc_base_impl(variant_mc_base_impl const&) = default;
  variant_mc_base_impl& operator=(variant_mc_base_impl const&) = default;

  variant_mc_base_impl(variant_mc_base_impl&&) = delete;
  variant_mc_base_impl& operator=(variant_mc_base_impl&&) = default;
};

template <typename... Ts>
class variant_mc_base_impl<true, true, Ts...> : public variant_ca_base<Ts...>
{
public:

  using variant_ca_base<Ts...>::variant_ca_base;

  variant_mc_base_impl(variant_mc_base_impl const&) = default;
  variant_mc_base_impl& operator=(variant_mc_base_impl const&) = default;

  variant_mc_base_impl(variant_mc_base_impl&&) = default;
  variant_mc_base_impl& operator=(variant_mc_base_impl&&) = default;
};

template <typename... Ts>
class variant_mc_base_impl<true, false, Ts...> : public variant_ca_base<Ts...>
{

private:

  struct move_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_mc_base_impl& this_, variant_mc_base_impl&& other) const
    {
      this_.init(I{}, std::move(other).get(I{}));
    }
  };

public:

  using variant_ca_base<Ts...>::variant_ca_base;

  variant_mc_base_impl(variant_mc_base_impl const&) = default;
  variant_mc_base_impl& operator=(variant_mc_base_impl const&) = default;

  constexpr variant_mc_base_impl(variant_mc_base_impl&& other) noexcept(
      mp::m_all<std::is_nothrow_move_constructible<Ts>...>::value)
    : variant_ca_base<Ts...>{}
  {
    if (!other.valueless_by_exception())
      mp::m_invoke_with_index<sizeof...(Ts)>(
          other.index(), move_fn{}, *this, std::move(other));
  }
  variant_mc_base_impl& operator=(variant_mc_base_impl&&) = default;
};

template <bool, bool, typename... Ts>
class variant_ma_base_impl;

template <typename... Ts>
using variant_ma_base = variant_ma_base_impl<
    mp::m_all<std::is_move_constructible<Ts>..., std::is_move_assignable<Ts>...>::value,
    mp::m_all<
        std::is_trivially_move_constructible<Ts>...,
        std::is_trivially_move_assignable<Ts>...,
        std::is_trivially_destructible<Ts>...>::value,
    Ts...>;

template <bool TriviallyMoveAssignable, typename... Ts>
class variant_ma_base_impl<false, TriviallyMoveAssignable, Ts...>
  : public variant_mc_base<Ts...>
{

public:

  using variant_mc_base<Ts...>::variant_mc_base;

  variant_ma_base_impl(variant_ma_base_impl const&) = default;
  variant_ma_base_impl& operator=(variant_ma_base_impl const&) = default;

  variant_ma_base_impl(variant_ma_base_impl&&) = default;
  variant_ma_base_impl& operator=(variant_ma_base_impl&&) = delete;
};

template <typename... Ts>
class variant_ma_base_impl<true, true, Ts...> : public variant_mc_base<Ts...>
{

public:

  using variant_mc_base<Ts...>::variant_mc_base;

  variant_ma_base_impl(variant_ma_base_impl const&) = default;
  variant_ma_base_impl& operator=(variant_ma_base_impl const&) = default;

  variant_ma_base_impl(variant_ma_base_impl&&) = default;
  variant_ma_base_impl& operator=(variant_ma_base_impl&&) = default;
};

template <typename... Ts>
class variant_ma_base_impl<true, false, Ts...> : public variant_mc_base<Ts...>
{

public:

  struct move_assign_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_ma_base_impl& this_, variant_ma_base_impl&& other) const
    {
      this_.get(I{}) = std::move(other).get(I{});
    }
  };

  struct move_emplace_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_ma_base_impl& this_, variant_ma_base_impl&& other) const
    {
      this_.template emplace<I::value>(std::move(other).get(I{}));
    }
  };

public:

  using variant_mc_base<Ts...>::variant_mc_base;

  variant_ma_base_impl(variant_ma_base_impl const&) = default;
  variant_ma_base_impl& operator=(variant_ma_base_impl const&) = default;

  variant_ma_base_impl(variant_ma_base_impl&&) = default;

  variant_ma_base_impl& operator=(variant_ma_base_impl&& other) noexcept(
      mp::m_all<mp::m_all<
          std::is_nothrow_move_constructible<Ts>,
          std::is_nothrow_move_assignable<Ts>>...>::value)
  {
    if (this != &other) {
      if (this->index() == other.index()) {
        if (!this->valueless_by_exception()) {
          mp::m_invoke_with_index<sizeof...(Ts)>(
              this->index(), move_assign_fn{}, *this, std::move(other));
        }
      } else {
        if (other.valueless_by_exception()) {
          this->destroy();
        } else {
          mp::m_invoke_with_index<sizeof...(Ts)>(
              other.index(), move_emplace_fn{}, *this, std::move(other));
        }
      }
    }
    return *this;
  }
};

template <typename T>
struct anti_narrow_type {
  T x[1];
};

template <typename T, typename V, typename = void>
struct variant_conversion_overload {
  constexpr void call();
};

template <typename T, typename V>
struct variant_conversion_overload<
    T,
    V,
    mp::m_void<decltype(anti_narrow_type<T>{mp::m_declval<V>()})>> {
  static constexpr auto call(T) -> mp::m_identity<T>;
};

template <typename V, typename T, typename... Ts>
struct variant_conversion_overloads : variant_conversion_overload<T, V>,
                                      variant_conversion_overloads<V, Ts...> {
  using variant_conversion_overload<T, V>::call;
  using variant_conversion_overloads<V, Ts...>::call;
};

template <typename V, typename T>
struct variant_conversion_overloads<V, T> : variant_conversion_overload<T, V> {
  using variant_conversion_overload<T, V>::call;
};

template <typename Variant, typename Value>
using variant_conversion_t =
    typename decltype(mp::m_apply<
                      variant_conversion_overloads,
                      mp::m_push_front<Variant, Value>>::call(mp::m_declval<Value>()))::
        type;

template <typename Variant, typename Value>
using confirm_conversion_t =
    std::is_constructible<variant_conversion_t<Variant, Value>, Value>;

template <typename Variant, typename Value, typename V = mp::m_remove_cvref<Value>>
using is_valid_variant_conversion = mp::m_all<
    mp::m_not<is_variant<V>>,
    mp::m_not<mp::m_empty<Variant>>,
    mp::m_not<is_in_place_type<V>>,
    mp::m_not<is_in_place_index<V>>,
    mp::m_eval_or<mp::m_false, confirm_conversion_t, Variant, Value>>;

template <typename Variant, typename Value>
using enable_if_is_valid_variant_conversion =
    mp::m_if<is_valid_variant_conversion<Variant, Value>, void>;

struct variant_friend;

}  // namespace variant_ns

template <typename... Ts>
class variant : public variant_ns::variant_ma_base<Ts...>
{

  friend struct variant_ns::variant_friend;

  using base_type = variant_ns::variant_ma_base<Ts...>;

  using type_list = mp::m_list<Ts...>;

  template <std::size_t I>
  using type_at = variant_alternative_t<I, variant<Ts...>>;

  template <typename T>
  using index_of = mp::m_find<variant<Ts...>, T>;

  template <typename T>
  using is_unique = mp::m_bool<mp::m_count<variant<Ts...>, T>{} == 1>;

public:

  constexpr variant() noexcept(std::is_nothrow_default_constructible<type_at<0>>::value)
    : base_type{mp::m_size_t<0>{}}
  {
  }

  variant(variant const&) = default;

  variant(variant&&) = default;

  template <
      typename T,
      typename = variant_ns::enable_if_is_valid_variant_conversion<variant<Ts...>, T>,
      typename I = index_of<variant_ns::variant_conversion_t<variant<Ts...>, T>>>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible<
                                    variant_alternative_t<I::value, variant<Ts...>>,
                                    T>::value)
    : base_type{I{}, std::forward<T>(t)}
  {
  }

  template <
      typename T,
      typename... Args,
      typename = mp::m_if<is_unique<T>, void>,
      typename I = index_of<T>,
      typename = mp::m_if<std::is_constructible<type_at<I::value>, Args...>, void>>
  constexpr explicit variant(in_place_type_t<T>, Args&&... args) noexcept(
      std::is_nothrow_constructible<type_at<I::value>, Args...>::value)
    : base_type{I{}, std::forward<Args>(args)...}
  {
  }

  template <
      typename T,
      typename U,
      typename... Args,
      typename = mp::m_if<is_unique<T>, void>,
      typename I = index_of<T>,
      typename = mp::m_if<
          std::is_constructible<type_at<I::value>, std::initializer_list<U>&, Args...>,
          void>>
  constexpr explicit variant(
      in_place_type_t<T>,
      std::initializer_list<U> il,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible<
                                       type_at<I::value>,
                                       std::initializer_list<U>&,
                                       Args...>::value)
    : base_type{I{}, il, std::forward<Args>(args)...}
  {
  }

  template <
      std::size_t I,
      typename... Args,
      typename = mp::m_if_c<
          (sizeof...(Ts) > I) && std::is_constructible<type_at<I>, Args...>::value,
          void>>
  constexpr explicit variant(in_place_index_t<I>, Args&&... args) noexcept(
      std::is_nothrow_constructible<type_at<I>, Args...>::value)
    : base_type{mp::m_size_t<I>{}, std::forward<Args>(args)...}
  {
  }

  template <
      std::size_t I,
      typename U,
      typename... Args,
      typename = mp::m_if_c<
          (sizeof...(Ts) > I) &&
              std::is_constructible<type_at<I>, std::initializer_list<U>&, Args...>::
                  value,
          void>>
  constexpr explicit variant(
      in_place_index_t<I>,
      std::initializer_list<U> il,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible<
                                       type_at<I>,
                                       std::initializer_list<U>&,
                                       Args...>::value)
    : base_type{mp::m_size_t<I>{}, il, std::forward<Args>(args)...}
  {
  }

  ~variant() = default;

  variant& operator=(variant const&) = default;

  variant& operator=(variant&&) = default;

  template <
      typename T,
      typename = variant_ns::enable_if_is_valid_variant_conversion<variant<Ts...>, T>,
      typename I = index_of<variant_ns::variant_conversion_t<variant<Ts...>, T>>>
  constexpr variant&
  operator=(T&& v) noexcept(std::is_nothrow_constructible<type_at<I::value>, T>::value&&
                                std::is_nothrow_assignable<type_at<I::value>&, T>::value)
  {
    if (this->index() == I::value) {
      this->get(I{}) = std::forward<T>(v);
    } else {
      convert_assign(
          mp::m_any<
              std::is_nothrow_constructible<type_at<I::value>, T>,
              mp::m_not<std::is_nothrow_move_constructible<type_at<I::value>>>>{},
          I{},
          std::forward<T>(v));
    }
    return *this;
  }

  using base_type::index;

  using base_type::valueless_by_exception;

  template <
      typename T,
      typename... Args,
      typename =
          mp::m_if<mp::m_all<is_unique<T>, std::is_constructible<T, Args...>>, void>>
  constexpr T&
  emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>{})
  {
    return this->template emplace<index_of<T>::value>(std::forward<Args>(args)...);
  }

  template <
      typename T,
      typename U,
      typename... Args,
      typename = mp::m_if<
          mp::m_all<
              is_unique<T>,
              std::is_constructible<T, std::initializer_list<U>&, Args...>>,
          void>>
  constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, Args...>{})
  {
    return this->template emplace<index_of<T>::value>(il, std::forward<Args>(args)...);
  }

  // template<std::size_t, typename ... Args>
  using base_type::emplace;

  template <
      std::size_t I,
      typename U,
      typename... Args,
      typename T = type_at<I>,
      typename =
          mp::m_if<std::is_constructible<T, std::initializer_list<U>&, Args...>, void>>
  constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>{})
  {
    return base_type::template emplace<I>(il, std::forward<Args>(args)...);
  }

  constexpr void
  swap(variant& other) noexcept(mp::m_all<
                                std::is_nothrow_move_constructible<Ts>...,
                                variant_ns::is_nothrow_swappable<Ts>...>::value)
  {
    auto const I = index();
    if (I == other.index()) {
      if (valueless_by_exception())
        return;
      mp::m_invoke_with_index<sizeof...(Ts)>(I, swap_fn{}, *this, other);
    } else {
      variant tmp{std::move(*this)};
      *this = std::move(other);
      other = std::move(tmp);
    }
  }

private:

  template <typename I, typename V>
  constexpr void convert_assign(mp::m_true, I, V&& v)
  {
    base_type::template emplace<I::value>(std::forward<V>(v));
  }

  template <typename I, typename V>
  constexpr void convert_assign(mp::m_false, I, V&& v)
  {
    using T = type_at<I::value>;
    base_type::template emplace<I::value>(T(std::forward<V>(v)));
  }

  struct swap_fn {
    template <typename I>
    constexpr void operator()(I, variant& this_, variant& other)
    {
      using std::swap;
      swap(this_.get(I{}), other.get(I{}));
    }
  };
};

namespace variant_ns
{

struct variant_friend {
  template <typename Variant, typename I>
  static constexpr decltype(auto) get(I, Variant&& variant)
  {
    return std::forward<Variant>(variant).get(I{});
  }
};

template <std::size_t I, typename Variant>
inline constexpr decltype(auto) unsafe_get(mp::m_size_t<I> index, Variant&& variant)
{
  return variant_friend::get(index, std::forward<Variant>(variant));
}

template <typename Fn, typename Arg>
struct bind_front_arg {
  Fn&&  fn;
  Arg&& arg;
  template <typename... Args>
  constexpr decltype(auto) operator()(Args&&... args) const
  {
    return std::forward<Fn>(fn)(std::forward<Arg>(arg), std::forward<Args>(args)...);
  }
};

template <typename Fn, typename Arg>
inline constexpr bind_front_arg<Fn, Arg> bind_front(Fn&& fn, Arg&& arg)
{
  return {std::forward<Fn>(fn), std::forward<Arg>(arg)};
}

struct visit_impl {

  template <
      typename I,
      typename Function,
      typename Variant,
      typename Variant2,
      typename... Variants>
  constexpr decltype(auto) operator()(
      I,
      Function&& function,
      Variant&&  variant,
      Variant2&& variant2,
      Variants&&... variants) const
  {
    auto const bound = bind_front(
        std::forward<Function>(function),
        unsafe_get(I{}, std::forward<Variant>(variant)));
    return variant2.valueless_by_exception()
               ? SVAR_BAD_ACCESS("bad_variant_access: invalid variant in visit")
               : mp::m_invoke_with_index<mp::m_size<mp::m_remove_cvref<Variant>>::value>(
                     variant2.index(),
                     visit_impl{},
                     bound,
                     std::forward<Variant2>(variant2),
                     std::forward<Variants>(variants)...);
  }

  template <typename I, typename Function, typename Variant>
  constexpr decltype(auto) operator()(I, Function&& function, Variant&& variant) const
  {
    return std::forward<Function>(function)(
        unsafe_get(I{}, std::forward<Variant>(variant)));
  }
};

struct deduced_visit {
};

template <typename T>
using require_deduced_visit = mp::m_if<mp::m_same<T, deduced_visit>, void>;

}  // namespace variant_ns

template <
    typename R = variant_ns::deduced_visit,
    typename Visitor,
    typename = variant_ns::require_deduced_visit<R>>
inline constexpr decltype(auto) visit(Visitor&& visitor)
{
  return std::forward<Visitor>(visitor)();
}

template <
    typename R = variant_ns::deduced_visit,
    typename Visitor,
    typename Variant,
    typename... Variants,
    typename = variant_ns::require_deduced_visit<R>>
inline constexpr decltype(auto)
visit(Visitor&& visitor, Variant&& variant, Variants&&... variants)
{
  return variant.valueless_by_exception()
             ? SVAR_BAD_ACCESS("bad_variant_access: invalid variant in visit")
             : mp::m_invoke_with_index<mp::m_size<mp::m_remove_cvref<Variant>>::value>(
                   variant.index(),
                   variant_ns::visit_impl{},
                   std::forward<Visitor>(visitor),
                   std::forward<Variant>(variant),
                   std::forward<Variants>(variants)...);
}

template <
    typename R,
    typename Visitor,
    typename... Variants,
    typename = mp::m_if<mp::m_not<mp::m_same<mp::m_remove_cvref<R>, void>>, void>>
inline constexpr R visit(Visitor&& visitor, Variants&&... variants)
{
  return visit(std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);
}

template <
    typename R,
    typename Visitor,
    typename... Variants,
    typename = mp::m_if<mp::m_same<mp::m_remove_cvref<R>, void>, void>>
inline constexpr void visit(Visitor&& visitor, Variants&&... variants)
{
  visit(std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);
}

template <typename T, typename... Ts>
inline constexpr bool holds_alternative(variant<Ts...> const& variant) noexcept
{
  static_assert(
      mp::m_count<simple::variant<Ts...>, T>::value == 1,
      "type T must be unique in variant");
  return variant.index() == mp::m_find<simple::variant<Ts...>, T>::value;
}

#define SVAR_BAD_INDEX() SVAR_BAD_ACCESS("bad_variant_access: index not active")

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>>& get(variant<Ts...>& v)
{
  using variant_ns::unsafe_get;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, v) : SVAR_BAD_INDEX();
}

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>>&& get(variant<Ts...>&& v)
{
  using variant_ns::unsafe_get;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, std::move(v)) : SVAR_BAD_INDEX();
}

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>> const&
get(variant<Ts...> const& v)
{
  using variant_ns::unsafe_get;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, v) : SVAR_BAD_INDEX();
}

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>> const&&
get(variant<Ts...> const&& v)
{
  using variant_ns::unsafe_get;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, std::move(v)) : SVAR_BAD_INDEX();
}

#define SVAR_BAD_TYPE() SVAR_BAD_ACCESS("bad_variant_access: type T not active")

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T& get(variant<Ts...>& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");

  using variant_ns::unsafe_get;

  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, v) : SVAR_BAD_TYPE();
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T&& get(variant<Ts...>&& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");

  using variant_ns::unsafe_get;

  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, std::move(v)) : SVAR_BAD_TYPE();
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T const& get(variant<Ts...> const& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");

  using variant_ns::unsafe_get;

  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, v) : SVAR_BAD_TYPE();
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T const&& get(variant<Ts...> const&& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");

  using variant_ns::unsafe_get;

  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? unsafe_get(mp::m_size_t<I>{}, std::move(v)) : SVAR_BAD_TYPE();
}

template <
    std::size_t I,
    typename... Ts,
    typename = mp::m_if_c<(I < (sizeof...(Ts))), void>>
inline constexpr std::add_pointer_t<variant_alternative_t<I, variant<Ts...>>>
get_if(variant<Ts...>* pv) noexcept
{
  if (pv && pv->index() == I)
    return &variant_ns::unsafe_get(mp::m_size_t<I>{}, *pv);
  return nullptr;
}

template <
    std::size_t I,
    typename... Ts,
    typename = mp::m_if_c<(I < (sizeof...(Ts))), void>>
inline constexpr std::add_pointer_t<variant_alternative_t<I, variant<Ts...> const>>
get_if(variant<Ts...> const* pv) noexcept
{
  if (pv && pv->index() == I)
    return &variant_ns::unsafe_get(mp::m_size_t<I>{}, *pv);
  return nullptr;
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr std::add_pointer_t<T> get_if(variant<Ts...>* pv) noexcept
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");

  constexpr auto I = mp::m_find<variant<Ts...>, T>{};

  if (pv && pv->index() == I.value)
    return &variant_ns::unsafe_get(I, *pv);
  return nullptr;
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr std::add_pointer_t<T const> get_if(variant<Ts...> const* pv) noexcept
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");

  constexpr auto I = mp::m_find<variant<Ts...>, T>{};

  if (pv && pv->index() == I.value)
    return &variant_ns::unsafe_get(I, *pv);
  return nullptr;
}

namespace variant_ns
{

struct variant_eq_fn {
  template <typename I, typename... Ts>
  constexpr bool operator()(I, variant<Ts...> const& lhs, variant<Ts...> const& rhs)
  {
    return unsafe_get(I{}, lhs) == unsafe_get(I{}, rhs);
  }
};

struct variant_neq_fn {
  template <typename I, typename... Ts>
  constexpr void operator()(I, variant<Ts...> const& lhs, variant<Ts...> const& rhs)
  {

    return unsafe_get(I{}, lhs) != unsafe_get(I{}, rhs);
  }
};

struct variant_lt_fn {
  template <typename I, typename... Ts>
  constexpr void operator()(I, variant<Ts...> const& lhs, variant<Ts...> const& rhs)
  {
    return unsafe_get(I{}, lhs) < unsafe_get(I{}, rhs);
  }
};

struct variant_lte_fn {
  template <typename I, typename... Ts>
  constexpr void operator()(I, variant<Ts...> const& lhs, variant<Ts...> const& rhs)
  {
    return unsafe_get(I{}, lhs) <= unsafe_get(I{}, rhs);
  }
};

struct variant_gt_fn {
  template <typename I, typename... Ts>
  constexpr void operator()(I, variant<Ts...> const& lhs, variant<Ts...> const& rhs)
  {
    return unsafe_get(I{}, lhs) > unsafe_get(I{}, rhs);
  }
};

struct variant_gte_fn {
  template <typename I, typename... Ts>
  constexpr void operator()(I, variant<Ts...> const& lhs, variant<Ts...> const& rhs)
  {
    return unsafe_get(I{}, lhs) >= unsafe_get(I{}, rhs);
  }
};

}  // namespace variant_ns

template <typename... Ts>
inline constexpr bool operator==(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.index() != rhs.index())
    return false;
  return lhs.valueless_by_exception()
             ? true
             : mp::m_invoke_with_index<sizeof...(Ts)>(
                   lhs.index(), variant_ns::variant_eq_fn{}, lhs, rhs);
}

template <typename... Ts>
inline constexpr bool operator!=(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.index() != rhs.index())
    return true;
  return lhs.valueless_by_exception()
             ? false
             : mp::m_invoke_with_index<sizeof...(Ts)>(
                   lhs.index(), variant_ns::variant_neq_fn{}, lhs, rhs);
}

template <typename... Ts>
inline constexpr bool operator<(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (rhs.valueless_by_exception())
    return false;
  if (lhs.valueless_by_exception())
    return true;

  auto const LI = lhs.index();
  auto const RI = rhs.index();

  if (LI < RI)
    return true;
  if (LI > RI)
    return false;

  return mp::m_invoke_with_index<sizeof...(Ts)>(
      lhs.index(), variant_ns::variant_lt_fn{}, lhs, rhs);
}

template <typename... Ts>
inline constexpr bool operator>(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.valueless_by_exception())
    return false;
  if (rhs.valueless_by_exception())
    return true;

  auto const LI = lhs.index();
  auto const RI = rhs.index();

  if (LI > RI)
    return true;
  if (LI < RI)
    return false;

  return mp::m_invoke_with_index<sizeof...(Ts)>(
      lhs.index(), variant_ns::variant_gt_fn{}, lhs, rhs);
}

template <typename... Ts>
inline constexpr bool operator<=(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.valueless_by_exception())
    return true;
  if (rhs.valueless_by_exception())
    return false;

  auto const LI = lhs.index();
  auto const RI = rhs.index();

  if (LI < RI)
    return true;
  if (LI > RI)
    return false;

  return mp::m_invoke_with_index<sizeof...(Ts)>(
      lhs.index(), variant_ns::variant_lte_fn{}, lhs, rhs);
}

template <typename... Ts>
inline constexpr bool operator>=(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (rhs.valueless_by_exception())
    return true;
  if (lhs.valueless_by_exception())
    return false;

  auto const LI = lhs.index();
  auto const RI = rhs.index();

  if (LI > RI)
    return true;
  if (LI < RI)
    return false;

  return mp::m_invoke_with_index<sizeof...(Ts)>(
      lhs.index(), variant_ns::variant_gte_fn{}, lhs, rhs);
}

template <
    typename... Ts,
    typename = mp::m_if<
        mp::m_all<std::is_move_constructible<Ts>..., variant_ns::is_swappable<Ts>...>,
        void>>
inline constexpr void
swap(variant<Ts...>& lhs, variant<Ts...>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}

}  // namespace simple

#undef SVAR_BAD_ACCESS
#undef SVAR_BAD_INDEX
#undef SVAR_BAD_TYPE

#endif  // SIMPLE_VARIANT_HPP
