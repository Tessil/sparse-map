/** @file
 * @brief Checks for fancy pointer support in the sparse_hash implementation for pair values (maps).
 */

#include <unordered_map>
#include <boost/test/unit_test.hpp>
#include <tsl/sparse_map.h>
#include <tsl/sparse_hash.h>
#include "CustomAllocator.h"

/* Tests are analogous to the  tests in sparse_array_tests.cpp.
 * The template parameter now also holds the value_type.
 */
namespace details {
    template<typename Key, typename T>
    struct KeySelect {
        using key_type = Key;
        const key_type &operator()(std::pair<Key, T> const &key_value) const noexcept {
            return key_value.first;
        }
        key_type &operator()(std::pair<Key, T> &key_value) noexcept {
            return key_value.first;
        }
    };

    template<typename Key, typename T>
    struct ValueSelect {
        using value_type = T;
        const value_type &operator()(std::pair<Key, T> const &key_value) const noexcept {
            return key_value.second;
        }
        value_type &operator()(std::pair<Key, T> &key_value) noexcept {
            return key_value.second;
        }
    };

    template<typename Key, typename T, typename Alloc>
    using sparse_map= tsl::detail_sparse_hash::sparse_hash<
            std::pair<Key, T>, KeySelect<Key, T>, ValueSelect<Key,T>, std::hash<T>, std::equal_to<T>, Alloc,
            tsl::sh::power_of_two_growth_policy<2>,
            tsl::sh::exception_safety::basic,
            tsl::sh::sparsity::medium,
            tsl::sh::probing::quadratic>;

    template<typename T>
    typename T::Map default_construct_map() {
        using Key = typename T::key_type;
        return typename T::Map(T::Map::DEFAULT_INIT_BUCKET_COUNT,
                        std::hash<Key>(),
                        std::equal_to<Key>(),
                        typename T::Allocator(),
                        T::Map::DEFAULT_MAX_LOAD_FACTOR);
    }

    /** Checks if all values of the map are in the initializer_list and than if the lengths are equal.
     *  So basically Map \subset l and |Map| == |l|.
     *  Needs 'map.contains(.)' and 'map.at(.)' to work correctly.
     */
    template <typename Map>
    bool is_equal(Map const& map, std::initializer_list<typename Map::value_type> l) {
        auto check_in_map = [&map](typename Map::value_type p) {
           return map.contains(p.first) && map.at(p.first) == p.second;
        };
        return std::all_of(l.begin(), l.end(), check_in_map) && map.size() == l.size();
    }
    template <typename Map1, typename Map2>
    bool is_equal(Map1 const& custom_map, Map2 const &normal_map) {
        auto check_in_map = [&custom_map](typename Map2::value_type const& p) {
            return custom_map.count(p.first) == 1 && custom_map.at(p.first) == p.second;
        };
        return std::all_of(normal_map.begin(), normal_map.end(), check_in_map) && custom_map.size() == normal_map.size();
    }
}

template<typename T>
void construction() {
   auto map = details::default_construct_map<T>();
}

template <typename T>
void insert(std::initializer_list<typename T::value_type> l) {
    auto map = details::default_construct_map<T>();
    for (auto dataPair : l)  map.insert(dataPair);
    //'insert' did not create exactly the values needed
    BOOST_REQUIRE(details::is_equal(map, l));
}

template <typename T>
void iterator_insert(std::initializer_list<typename T::value_type> l) {
   auto map = details::default_construct_map<T>();
   map.insert(l.begin(), l.end());
   //'insert' with iterators did not create exactly the values needed
   BOOST_REQUIRE(details::is_equal(map, l));
}

template <typename T>
void iterator_access(typename T::value_type single_value) {
    auto map = details::default_construct_map<T>();
    map.insert(single_value);
    //iterator cannot access single value
    BOOST_REQUIRE( (*(map.begin()) == single_value));
}

template <typename T>
void iterator_access_multi(std::initializer_list<typename T::value_type> l) {
    auto map = details::default_construct_map<T>();
    map.insert(l.begin(), l.end());
    std::vector<typename T::value_type> l_sorted = l;
    std::vector<typename T::value_type> map_sorted(map.begin(), map.end());
    std::sort(l_sorted.begin(), l_sorted.end());
    std::sort(map_sorted.begin(), map_sorted.end());
    //iterating over the map didn't work
    BOOST_REQUIRE(std::equal(l_sorted.begin(), l_sorted.end(),
                                  map_sorted.begin()));
}

template<typename T>
void value(std::initializer_list<typename T::value_type> l, typename T::value_type to_change) {
    auto map = details::default_construct_map<T>();
    map.insert(l.begin(), l.end());
    map[to_change.first] = to_change.second;

    std::unordered_map<typename T::value_type::first_type, typename T::value_type::second_type> check(l.begin(), l.end());
    check[to_change.first] = to_change.second;

    //changing a single value didn't work
    BOOST_REQUIRE(details::is_equal(map, check));
}


template<typename Key, typename T>
struct STD {
    using key_type = Key;
    using value_type = std::pair<Key, T>;
    using Allocator = std::allocator<value_type>;
    using Map = details::sparse_map<Key, T, Allocator>;
};

template<typename Key, typename T>
struct CUSTOM {
    using key_type = Key;
    using value_type = std::pair<Key, T>;
    using Allocator = OffsetAllocator<value_type>;
    using Map = details::sparse_map<Key, T, Allocator>;
};


BOOST_AUTO_TEST_SUITE(fancy_pointers)
BOOST_AUTO_TEST_SUITE(sparse_hash_map_tests)

BOOST_AUTO_TEST_CASE(std_alloc_compiles) {construction<STD<int, int>>();}
BOOST_AUTO_TEST_CASE(std_alloc_insert) {insert<STD<int, int>>({{1,2},{3,4},{5,6}});}
BOOST_AUTO_TEST_CASE(std_alloc_iterator_insert) {insert<STD<int, int>>({{1,2},{3,4},{5,6}});}
BOOST_AUTO_TEST_CASE(std_alloc_iterator_access) {iterator_access<STD<int, int>>({1,42});}
BOOST_AUTO_TEST_CASE(std_alloc_iterator_access_multi) {iterator_access_multi<STD<int, int>>({{1,2},{3,4},{5,6}});}
BOOST_AUTO_TEST_CASE(std_alloc_value) {value<STD<int, int>>({{1,2},{3,4},{5,6}}, {1, 42});}

BOOST_AUTO_TEST_CASE(custom_alloc_compiles) {construction<CUSTOM<int, int>>();}
BOOST_AUTO_TEST_CASE(custom_alloc_insert) {insert<CUSTOM<int, int>>({{1,2},{3,4},{5,6}});}
BOOST_AUTO_TEST_CASE(custom_alloc_iterator_insert) {insert<CUSTOM<int, int>>({{1,2},{3,4},{5,6}});}
BOOST_AUTO_TEST_CASE(custom_alloc_iterator_access) {iterator_access<CUSTOM<int, int>>({1,42});}
BOOST_AUTO_TEST_CASE(custom_alloc_iterator_access_multi) {iterator_access_multi<CUSTOM<int, int>>({{1,2},{3,4},{5,6}});}
BOOST_AUTO_TEST_CASE(custom_alloc_value) {value<CUSTOM<int, int>>({{1,2},{3,4},{5,6}}, {1, 42});}

BOOST_AUTO_TEST_CASE(full_map) {
    tsl::sparse_map<int, int, std::hash<int>, std::equal_to<int>, OffsetAllocator<std::pair<int,int>>> map;
    std::vector<std::pair<int,int>> data = {
            {0,1},{2,3},{4,5},{6,7},{8,9}
    };
    map.insert(data.begin(), data.end());
    auto check = [&map](std::pair<int,int> p) {
        if (!map.contains(p.first)) return false;
        return map.at(p.first) == p.second;
    };
    //size did not match
    BOOST_REQUIRE(data.size() == map.size());
    //map did not contain all values
    BOOST_REQUIRE(std::all_of(data.begin(), data.end(), check));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
