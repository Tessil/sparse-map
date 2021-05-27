#include <boost/test/unit_test.hpp>
#include <iostream>
#include <memory>
#include <scoped_allocator>
#include <tsl/sparse_set.h>
#include <type_traits>
#include <unordered_set>

// Globals
constexpr auto MAX_INDEX = 32; // BITMAP_NB_BITS

template <typename T> void compilation() { typename T::Array test; }

template <typename T> void construction() {
  typename T::Allocator a;
  typename T::Array test(MAX_INDEX, a);
  test.clear(a);
}

template <typename T>
void set(std::initializer_list<typename T::value_type> l) {
  typename T::Allocator a;
  typename T::Array array(MAX_INDEX, a);
  std::vector<typename T::value_type> check;
  check.reserve(l.size());
  std::size_t counter = 0;
  for (auto const &value : l) {
    array.set(a, counter++, value);
    check.emplace_back(value);
  }
  //'set' did not create the correct order of items
  BOOST_REQUIRE(std::equal(array.begin(), array.end(), check.begin()));
  array.clear(a);
}

template <typename T> void uses_allocator() {
  //uses_allocator returns false
  BOOST_REQUIRE((std::uses_allocator<typename T::Array, typename T::Allocator>::value));
}

template <typename T, typename... Args>
void trailing_allocator_convention(Args...) {
  using Alloc = typename T::Allocator;
  //trailing_allocator thinks construction is not possible
  BOOST_REQUIRE((std::is_constructible<typename T::Array, Args..., const Alloc &>::value));
}

template <typename T> void trailing_allocator_convention_without_parameters() {
  using Alloc = typename std::allocator_traits<
      typename T::Allocator>::template rebind_alloc<typename T::Array>;
  //trailing_allocator thinks construction is not possible
  BOOST_REQUIRE((std::is_constructible<typename T::Array, const Alloc &>::value));
}

template <typename T>
void is_move_insertable(std::initializer_list<typename T::value_type> l) {
  using A = typename std::allocator_traits<
      typename T::Allocator>::template rebind_alloc<typename T::Array>;
  A m;
  auto p = std::allocator_traits<A>::allocate(m, 1);
  typename T::Allocator ArrayAlloc;
  typename T::Array rv(MAX_INDEX, ArrayAlloc);
  std::size_t counter = 0;
  for (auto const &value : l) {
    rv.set(ArrayAlloc, counter++, value);
  }
  std::allocator_traits<A>::construct(m, p, std::move(rv));
  rv.clear(ArrayAlloc);
  p->clear(ArrayAlloc);
  std::allocator_traits<A>::destroy(m, p);
  std::allocator_traits<A>::deallocate(m, p, 1);
}

template <typename T> void is_default_insertable() {
  using A = typename std::allocator_traits<
      typename T::Allocator>::template rebind_alloc<typename T::Array>;
  A m;
  typename T::Array *p = std::allocator_traits<A>::allocate(m, 1);
  std::allocator_traits<A>::construct(m, p);
  std::allocator_traits<A>::deallocate(m, p, 1);
}

template <typename T, tsl::sh::sparsity Sparsity = tsl::sh::sparsity::medium>
struct NORMAL {
  using value_type = T;
  using Allocator = std::allocator<T>;
  using Array = tsl::detail_sparse_hash::sparse_array<T, Allocator, Sparsity>;
};

template <typename T, tsl::sh::sparsity Sparsity = tsl::sh::sparsity::medium>
struct SCOPED {
  using value_type = T;
  using Allocator = std::scoped_allocator_adaptor<std::allocator<T>>;
  using Array = tsl::detail_sparse_hash::sparse_array<T, Allocator, Sparsity>;
};

BOOST_AUTO_TEST_SUITE(scoped_allocators)
BOOST_AUTO_TEST_SUITE(sparse_array_tests)

BOOST_AUTO_TEST_CASE(normal_compilation) { compilation<NORMAL<int>>(); }
BOOST_AUTO_TEST_CASE(normal_construction) { construction<NORMAL<int>>(); }
BOOST_AUTO_TEST_CASE(normal_set) { set<NORMAL<int>>({0, 1, 2, 3, 4}); }
BOOST_AUTO_TEST_CASE(normal_uses_allocator) { uses_allocator<NORMAL<int>>(); }
BOOST_AUTO_TEST_CASE(normal_trailing_allocator_convention) {
  trailing_allocator_convention<NORMAL<int>>(0);
}
BOOST_AUTO_TEST_CASE(normal_is_move_insertable) {
  is_move_insertable<NORMAL<int>>({0, 1, 2, 3, 4, 5});
}
BOOST_AUTO_TEST_CASE(normal_is_default_insertable) {
  is_default_insertable<NORMAL<int>>();
}

BOOST_AUTO_TEST_CASE(scoped_compilation) { compilation<SCOPED<int>>(); }
BOOST_AUTO_TEST_CASE(scoped_construction) { construction<SCOPED<int>>(); }
BOOST_AUTO_TEST_CASE(scoped_set) { set<SCOPED<int>>({0, 1, 2, 3, 4}); }
BOOST_AUTO_TEST_CASE(scoped_uses_allocator) { uses_allocator<SCOPED<int>>(); }
BOOST_AUTO_TEST_CASE(scoped_trailing_allocator_convention) {
  trailing_allocator_convention<SCOPED<int>>(0);
}
BOOST_AUTO_TEST_CASE(scoped_is_move_insertable) {
  is_move_insertable<SCOPED<int>>({0, 1, 2, 3, 4, 5});
}
BOOST_AUTO_TEST_CASE(scoped_is_default_insertable) {
  is_default_insertable<SCOPED<int>>();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
