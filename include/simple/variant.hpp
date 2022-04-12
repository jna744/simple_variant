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
      typename T = variant_alternative<I, variant<Ts...>>>
  constexpr auto&
  emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>{})
  {
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
      typename T = variant_alternative<I, variant<Ts...>>>
  auto& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>{})
  {
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

struct variant_friend {
  template <typename Variant, typename I>
  static constexpr decltype(auto) get(Variant&& variant, I)
  {
    return std::forward<Variant>(variant).get(I{});
  }
};

#define SVAR_GET_INDEX(variant, index)                                                   \
  ::simple::variant_ns::variant_friend::get(variant, mp::m_size_t<index>{})

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
constexpr bind_front_arg<Fn, Arg> bind_front(Fn&& fn, Arg&& arg)
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
        SVAR_GET_INDEX(std::forward<Variant>(variant), I::value));
    return mp::m_invoke_with_index<mp::m_size<mp::m_remove_cvref<Variant>>::value>(
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
        SVAR_GET_INDEX(std::forward<Variant>(variant), I::value));
  }
};

}  // namespace variant_ns

template <typename Visitor>
inline constexpr decltype(auto)
visit(Visitor&& visitor)
{
  return std::forward<Visitor>(visitor);
}

template <typename Visitor, typename Variant, typename... Variants>
inline constexpr decltype(auto)
visit(Visitor&& visitor, Variant&& variant, Variants&&... variants)
{
  return mp::m_invoke_with_index<mp::m_size<mp::m_remove_cvref<Variant>>::value>(
      variant.index(),
      variant_ns::visit_impl{},
      std::forward<Visitor>(visitor),
      std::forward<Variant>(variant),
      std::forward<Variants>(variants)...);
}

#define SVAR_BAD_INDEX() SVAR_BAD_ACCESS("bad_variant_access: index not active")

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>>& get(variant<Ts...>& v)
{
  return v.index() == I ? SVAR_GET_INDEX(v, I) : SVAR_BAD_INDEX();
}

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>>&& get(variant<Ts...>&& v)
{
  return v.index() == I ? SVAR_GET_INDEX(std::move(v), I) : SVAR_BAD_INDEX();
}

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>> const&
get(variant<Ts...> const& v)
{
  return v.index() == I ? SVAR_GET_INDEX(v, I) : SVAR_BAD_INDEX();
}

template <std::size_t I, typename... Ts, typename = mp::m_if_c<(I < sizeof...(Ts)), void>>
inline constexpr variant_alternative_t<I, variant<Ts...>> const&&
get(variant<Ts...> const&& v)
{
  return v.index() == I ? SVAR_GET_INDEX(std::move(v), I) : SVAR_BAD_INDEX();
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
  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? SVAR_GET_INDEX(v, I) : SVAR_BAD_TYPE();
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T&& get(variant<Ts...>&& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");
  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? SVAR_GET_INDEX(std::move(v), I) : SVAR_BAD_TYPE();
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T const& get(variant<Ts...> const& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");
  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? SVAR_GET_INDEX(v, I) : SVAR_BAD_TYPE();
}

template <
    typename T,
    typename... Ts,
    typename = mp::m_if<mp::m_contains<variant<Ts...>, T>, void>>
inline constexpr T const&& get(variant<Ts...> const&& v)
{
  static_assert(
      mp::m_count<variant<Ts...>, T>::value == 1, "type T must be unique in variant");
  constexpr auto I = mp::m_find<variant<Ts...>, T>::value;
  return v.index() == I ? SVAR_GET_INDEX(std::move(v), I) : SVAR_BAD_TYPE();
}

template <typename... Ts>
class variant : public variant_ns::variant_ma_base<Ts...>
{

  friend struct variant_ns::variant_friend;

  using base_type = variant_ns::variant_ma_base<Ts...>;

  using type_list = mp::m_list<Ts...>;

  template <std::size_t I>
  using type_at = mp::m_at_c<type_list, I>;

public:

  constexpr variant() noexcept(std::is_nothrow_default_constructible<type_at<0>>::value)
    : base_type{mp::m_size_t<0>{}}
  {
  }

  template <
      std::size_t I,
      typename... Args,
      typename = mp::m_if_c<(sizeof...(Ts) > I), void>>
  constexpr explicit variant(in_place_index_t<I>, Args&&... args) noexcept(
      std::is_nothrow_constructible<type_at<I>, Args...>::value)
    : base_type{mp::m_size_t<I>{}, std::forward<Args>(args)...}
  {
  }

  variant(variant const&) = default;

  variant(variant&&) = default;

  ~variant() = default;

  variant& operator=(variant const&) = default;

  variant& operator=(variant&&) = default;

  using base_type::index;

  using base_type::valueless_by_exception;
};

}  // namespace simple

#undef SVAR_GET_INDEX
#undef SVAR_BAD_ACCESS
#undef SVAR_BAD_INDEX
#undef SVAR_BAD_TYPE

#endif  // SIMPLE_VARIANT_HPP
