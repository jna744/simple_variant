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

class bad_variant_access : public std::exception
{
public:

  explicit bad_variant_access(char const* what) : what_{what} {}
  virtual ~bad_variant_access() = default;

  char const* what() const noexcept override { return what_; }

private:

  char const* what_;
};

#define SV_BAD_ACCESS(what) throw ::simple::bad_variant_access(what)

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
  constexpr auto& emplace(mp::m_size_t<I>, Args&&... args)
  {
    return rest_.emplace(mp::m_size_t<I - 1>{}, std::forward<Args>(args)...);
  }

  template <typename... Args>
  constexpr T& emplace(mp::m_size_t<0>, Args&&... args)
  {
    new (&first_) T(std::forward<Args>(args)...);
    return first_;
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

private:

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

private:

  T                      first_;
  variant_storage<Ts...> rest_;
};

struct dummy_type {
};

template <bool, typename... Ts>
class variant_base_impl;

template <typename... Ts>
using variant_base = variant_base_impl<
    mp::m_all<std::is_trivially_destructible<Ts>...>::value,
    dummy_type,
    Ts...>;

template <typename... Ts>
class variant_base_impl<true, Ts...>
{

  using storage_type = variant_storage<Ts...>;

public:

  constexpr variant_base_impl(mp::m_size_t<0> index)
    : index_{variant_npos}, storage_{index}
  {
  }

  template <std::size_t I, typename... Args>
  constexpr variant_base_impl(mp::m_size_t<I> index, Args&&... args)
    : index_{I}, storage_{index, std::forward<Args>(args)...}
  {
  }

  ~variant_base_impl() = default;

protected:

  constexpr void destroy() noexcept { index_ = variant_npos; }

  std::size_t  index_;
  storage_type storage_;
};

template <typename... Ts>
class variant_base_impl<false, Ts...>
{

  using storage_type = variant_storage<Ts...>;

public:

  variant_base_impl(mp::m_size_t<0> index) : index_{variant_npos}, storage_{index} {}

  template <std::size_t I, typename... Args>
  variant_base_impl(mp::m_size_t<I> index, Args&&... args)
    : index_{I}, storage_{index, std::forward<Args>(args)...}
  {
  }

  ~variant_base_impl()
  {
    if (index_ != variant_npos)
      destroy();
  }

protected:

  struct destroy_fn {
    template <typename I, typename Variant>
    void operator()(I, Variant* this_) const
    {
      using index_type = mp::m_at<mp::m_list<Ts...>, I>;
      this_->storage_.get(I{}).~index_type();
    }
  };

  void destroy() noexcept
  {
    mp::m_vtable_invoke<sizeof...(Ts)>(index_, destroy_fn{}, this);
    index_ = variant_npos;
  }

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
      this_.storage_.emplace(I{}, other.storage_.get(I{}));
    }
  };

public:

  using variant_base<Ts...>::variant_base;

  constexpr variant_cc_base_impl(variant_cc_base_impl const& other) noexcept(
      mp::m_all<std::is_nothrow_copy_constructible<Ts>...>::value)
    : variant_base<Ts...>{mp::m_size_t<0>{}}
  {
    if (other.index_ != variant_npos) {
      mp::m_vtable_invoke<sizeof...(Ts) + 1>(other.index_, copy_fn{}, *this, other);
      this->index_ = other.index_;
    }
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
      this_.storage_.get(I{}) = other.storage_.get(I{});
    }
  };

  struct copy_emplace_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_ca_base_impl& this_, variant_ca_base_impl const& other) const
    {
      this_.storage_.emplace(I{}, other.storage_.get(I{}));
    }
  };

  template <typename = void>
  constexpr static void
  emplace(mp::m_true, variant_ca_base_impl& this_, variant_ca_base_impl const& other)
  {
    if (this_.index_ != variant_npos)
      this_.destroy();
    mp::m_vtable_invoke<sizeof...(Ts) + 1>(other.index_, copy_emplace_fn{}, this_, other);
    this_.index_ = other.index_;
  }

  template <typename = void>
  constexpr static void
  emplace(mp::m_false, variant_ca_base_impl& this_, variant_ca_base_impl const& other)
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
      if (this->index_ == other.index_) {
        if (this->index_ != variant_npos) {
          mp::m_vtable_invoke<sizeof...(Ts) + 1>(
              this->index_, copy_assign_fn{}, *this, other);
        }
      } else {
        if (other.index_ == variant_npos) {
          this->destroy();
        } else {
          emplace(is_valid_emplace{}, *this, other);
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
      this_.storage_.emplace(I{}, std::move(other.storage_).get(I{}));
    }
  };

public:

  using variant_ca_base<Ts...>::variant_ca_base;

  variant_mc_base_impl(variant_mc_base_impl const&) = default;
  variant_mc_base_impl& operator=(variant_mc_base_impl const&) = default;

  constexpr variant_mc_base_impl(variant_mc_base_impl&& other) noexcept(
      mp::m_all<std::is_nothrow_move_constructible<Ts>...>::value)
    : variant_ca_base<Ts...>{mp::m_size_t<0>{}}
  {
    if (other.index_ != variant_npos) {
      mp::m_vtable_invoke<sizeof...(Ts) + 1>(
          other.index_, move_fn{}, *this, std::move(other));
      this->index_ = other.index_;
    }
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
      this_.storage_.get(I{}) = std::move(other.storage_).get(I{});
    }
  };

  struct move_emplace_fn {
    template <typename I>
    constexpr void
    operator()(I, variant_ma_base_impl& this_, variant_ma_base_impl&& other) const
    {
      this_.storage_.emplace(I{}, std::move(other.storage_).get(I{}));
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
      if (this->index_ == other.index_) {
        if (this->index_ != variant_npos) {
          mp::m_vtable_invoke<sizeof...(Ts) + 1>(
              this->index_, move_assign_fn{}, *this, std::move(other));
        }
      } else {
        if (other.index_ == variant_npos) {
          this->destroy();
        } else {
          if (this->index_ != variant_npos)
            this->destroy();
          mp::m_vtable_invoke<sizeof...(Ts) + 1>(
              other.index_, move_emplace_fn{}, *this, std::move(other));
          this->index_ = other.index_;
        }
      }
    }
    return *this;
  }
};

}  // namespace variant_ns

template <typename... Ts>
class variant : public variant_ns::variant_ma_base<Ts...>
{

  using base_type = variant_ns::variant_ma_base<Ts...>;

  using type_list = mp::m_list<Ts...>;

  template <std::size_t I>
  using type_at = mp::m_at_c<type_list, I>;

public:

  constexpr variant() noexcept(std::is_nothrow_default_constructible<type_at<0>>::value)
    : base_type{mp::m_size_t<1>{}}
  {
  }

  template <
      std::size_t I,
      typename... Args,
      typename = mp::m_if_c<(sizeof...(Ts) > I), void>>
  constexpr explicit variant(in_place_index_t<I>, Args&&... args) noexcept(
      std::is_nothrow_constructible<type_at<I>, Args...>::value)
    : base_type{mp::m_size_t<I + 1>{}, std::forward<Args>(args)...}
  {
  }

  variant(variant const&) = default;

  variant(variant&&) = default;

  ~variant() = default;

  variant& operator=(variant const&) = default;

  variant& operator=(variant&&) = default;

  constexpr std::size_t index() const noexcept { return this->index_ - 1; }

  constexpr bool valueless_by_exception() const noexcept
  {
    return this->index_ == variant_npos;
  }
};

}  // namespace simple

#endif  // SIMPLE_VARIANT_HPP
