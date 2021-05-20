/** @file
 * @brief Checks for fancy pointer support in the sparse_array implementation.
 */

#include <boost/test/unit_test.hpp>
#include <tsl/sparse_hash.h>
#include "CustomAllocator.h"

// Globals
constexpr auto MAX_INDEX = 32; //BITMAP_NB_BITS

/* Tests are formulated via templates to reduce code duplication.
 * The template parameter contains the Allocator type and the shorthand "Array" for the sparse_array
 * (with all template parameter already inserted).
 */

template <typename T>
void compilation() {
    typename T::Array test;
}

template <typename T>
void construction() {
    typename T::Allocator a;
    typename T::Array test(MAX_INDEX, a);
    test.clear(a); //needed because destructor asserts
}

namespace details {
    template<typename T>
    typename T::Array generate_test_array(typename T::Allocator &a) {
        typename T::Array arr(MAX_INDEX, a);
        for (std::size_t i = 0; i < MAX_INDEX; ++i) {
            arr.set(a, i, i);
        }
        return arr;
    }

    template<typename T>
    std::vector<typename T::Allocator::value_type> generate_check_for_test_array() {
        std::vector<typename T::Allocator::value_type> check(MAX_INDEX);
        for (std::size_t i = 0; i < MAX_INDEX; ++i) {
            check[i] = i;
        }
        return check;
    }
}

template <typename T>
void set() {
    typename T::Allocator a;
    auto test = details::generate_test_array<T>(a);
    auto check = details::generate_check_for_test_array<T>();
    BOOST_TEST_REQUIRE(std::equal(test.begin(), test.end(), check.begin()),
                       "'set' did not create the correct order of items");
    test.clear(a); //needed because destructor asserts
}

template <typename T>
void copy_construction() {
    typename T::Allocator a;
    //needs to be its own line, otherwise the move-construction would take place
    auto test = details::generate_test_array<T>(a);
    typename T::Array copy(test, a);
    auto check = details::generate_check_for_test_array<T>();
    BOOST_TEST_REQUIRE(std::equal(copy.begin(), copy.end(), check.begin()),
                       "'copy' changed the order of the items");
    test.clear(a);
    copy.clear(a);
}

template <typename T>
void move_construction() {
    typename T::Allocator a;
    //two lines needed. Otherwise move/copy elision
    auto moved_from = details::generate_test_array<T>(a);
    typename T::Array moved_to(std::move(moved_from));
    auto check = details::generate_check_for_test_array<T>();
    BOOST_TEST_REQUIRE(std::equal(moved_to.begin(), moved_to.end(), check.begin()),
                       "'move' changed the order of the items");
    moved_to.clear(a);
}

template <typename T>
void const_iterator() {
    typename T::Allocator a;
    auto test = details::generate_test_array<T>(a);
    auto const_iter = test.cbegin();
    BOOST_TEST_REQUIRE((std::is_same<decltype(const_iter), typename T::Const_Iterator>::value),
                       "const iterator has the wrong type");
    test.clear(a);
}


/*
 * This are the types you can give the tests as template parameters.
 */
template <typename T, tsl::sh::sparsity Sparsity = tsl::sh::sparsity::medium>
struct STD {
    using Allocator = std::allocator<T>;
    using Array = tsl::detail_sparse_hash::sparse_array<T, std::allocator<T>, Sparsity>;
    using Const_Iterator = T const*;
};

template<typename T, tsl::sh::sparsity Sparsity = tsl::sh::sparsity::medium>
struct CUSTOM {
    using Allocator = OffsetAllocator<T>;
    using Array = tsl::detail_sparse_hash::sparse_array<T, OffsetAllocator<T>, Sparsity>;
    using Const_Iterator = boost::interprocess::offset_ptr<const T>;
};



/* The instantiation of the tests.
 * I don't use the boost template test cases because with this I can set the title of every test case myself.
 */
BOOST_AUTO_TEST_SUITE(fancy_pointers)
BOOST_AUTO_TEST_SUITE(sparse_array_tests)

BOOST_AUTO_TEST_CASE(std_alloc_compile) {compilation<STD<int>>();}
BOOST_AUTO_TEST_CASE(std_alloc_construction) {construction<STD<int>>();}
BOOST_AUTO_TEST_CASE(std_alloc_set) {set<STD<int>>();}
BOOST_AUTO_TEST_CASE(std_alloc_copy_construction) {copy_construction<STD<int>>();}
BOOST_AUTO_TEST_CASE(std_alloc_move_construction) {move_construction<STD<int>>();}
BOOST_AUTO_TEST_CASE(std_const_iterator) {const_iterator<STD<int>>();}

BOOST_AUTO_TEST_CASE(custom_alloc_compile) {compilation<CUSTOM<int>>();}
BOOST_AUTO_TEST_CASE(custom_alloc_construction) {construction<CUSTOM<int>>();}
BOOST_AUTO_TEST_CASE(custom_alloc_set) {set<CUSTOM<int>>();}
BOOST_AUTO_TEST_CASE(custom_alloc_copy_construction) {copy_construction<CUSTOM<int>>();}
BOOST_AUTO_TEST_CASE(custom_alloc_move_construction) {move_construction<CUSTOM<int>>();}
BOOST_AUTO_TEST_CASE(custom_const_iterator) {const_iterator<CUSTOM<int>>();}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
