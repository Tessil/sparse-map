/**
 * MIT License
 * 
 * Copyright (c) 2017 Tessil
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define BOOST_TEST_DYN_LINK


#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

#include <tsl/sparse_set.h>
#include "utils.h"


BOOST_AUTO_TEST_SUITE(test_sparse_set)

using test_types = boost::mpl::list<tsl::sparse_set<std::int64_t>,
                                    tsl::sparse_set<std::string>,
                                    tsl::sparse_set<self_reference_member_test>,
                                    tsl::sparse_set<move_only_test>,
                                    tsl::sparse_pg_set<self_reference_member_test>,
                                    tsl::sparse_set<move_only_test, 
                                                    std::hash<move_only_test>,
                                                    std::equal_to<move_only_test>, 
                                                    std::allocator<move_only_test>,
                                                    tsl::sh::prime_growth_policy>,
                                    tsl::sparse_set<self_reference_member_test, 
                                                    std::hash<self_reference_member_test>,
                                                    std::equal_to<self_reference_member_test>, 
                                                    std::allocator<self_reference_member_test>,
                                                    tsl::sh::mod_growth_policy<>>,
                                    tsl::sparse_set<move_only_test, 
                                                    std::hash<move_only_test>,
                                                    std::equal_to<move_only_test>, 
                                                    std::allocator<move_only_test>,
                                                    tsl::sh::mod_growth_policy<>>
                                    >;
                                    
                                    
                                    
BOOST_AUTO_TEST_CASE_TEMPLATE(test_insert, HSet, test_types) {
    // insert x values, insert them again, check values
    using key_t = typename HSet::key_type;
    
    const std::size_t nb_values = 1000;
    HSet set;
    typename HSet::iterator it;
    bool inserted;
    
    for(std::size_t i = 0; i < nb_values; i++) {
        std::tie(it, inserted) = set.insert(utils::get_key<key_t>(i));
        
        BOOST_CHECK_EQUAL(*it, utils::get_key<key_t>(i));
        BOOST_CHECK(inserted);
    }
    BOOST_CHECK_EQUAL(set.size(), nb_values);
    
    for(std::size_t i = 0; i < nb_values; i++) {
        std::tie(it, inserted) = set.insert(utils::get_key<key_t>(i));
        
        BOOST_CHECK_EQUAL(*it, utils::get_key<key_t>(i));
        BOOST_CHECK(!inserted);
    }
    
    for(std::size_t i = 0; i < nb_values; i++) {
        it = set.find(utils::get_key<key_t>(i));
        
        BOOST_CHECK_EQUAL(*it, utils::get_key<key_t>(i));
    }
}

BOOST_AUTO_TEST_CASE(test_compare) {
    const tsl::sparse_set<std::string> set1_1 = {"a", "e", "d", "c", "b"};
    const tsl::sparse_set<std::string> set1_2 = {"e", "c", "b", "a", "d"};
    const tsl::sparse_set<std::string> set2_1 = {"e", "c", "b", "a", "d", "f"};
    const tsl::sparse_set<std::string> set3_1 = {"e", "c", "b", "a"};
    const tsl::sparse_set<std::string> set4_1 = {};
    const tsl::sparse_set<std::string> set4_2 = {};
    
    BOOST_CHECK(set1_1 == set1_2);
    BOOST_CHECK(set1_2 == set1_1);
    
    BOOST_CHECK(set4_1 == set4_2);
    BOOST_CHECK(set4_2 == set4_1);
    
    BOOST_CHECK(set1_1 != set2_1);
    BOOST_CHECK(set2_1 != set1_1);
    
    BOOST_CHECK(set1_1 != set4_1);
    BOOST_CHECK(set4_1 != set1_1);
    
    BOOST_CHECK(set1_1 != set3_1);
    BOOST_CHECK(set3_1 != set1_1);
    
    BOOST_CHECK(set2_1 != set3_1);
    BOOST_CHECK(set3_1 != set2_1);
}

BOOST_AUTO_TEST_CASE(test_insert_pointer) {
    // Test added mainly to be sure that the code compiles with MSVC
    std::string value;
    std::string* value_ptr = &value;

    tsl::sparse_set<std::string*> set;
    set.insert(value_ptr);
    set.emplace(value_ptr);

    BOOST_CHECK_EQUAL(set.size(), 1);
    BOOST_CHECK_EQUAL(**set.begin(), value);
}


    
/**
 * serialize and deserialize
 */
BOOST_AUTO_TEST_CASE(test_serialize_desearialize_empty_set) {
    // serialize empty set; deserialize in new set; check equal.
    // for deserialization, test it with and without hash compatibility..
    std::stringstream buffer;
    buffer.exceptions(buffer.badbit | buffer.failbit | buffer.eofbit);
    
    tsl::sparse_set<move_only_test> empty_set(0);
    empty_set.serialize(serializer(), buffer);
    
    
    
    
    auto empty_set_deserialized = decltype(empty_set)::deserialize(serializer(), buffer, true);
    BOOST_CHECK(empty_set_deserialized == empty_set);
    
    buffer.seekg(0);
    empty_set_deserialized = decltype(empty_set)::deserialize(serializer(), buffer, false);
    BOOST_CHECK(empty_set_deserialized == empty_set);
}

BOOST_AUTO_TEST_CASE(test_serialize_desearialize_set) {
    // insert x values; delete some values; serialize set; deserialize in new set; check equal.
    // for deserialization, test it with and without hash compatibility..
    const std::size_t nb_values = 1000;
    
    std::stringstream buffer;
    buffer.exceptions(buffer.badbit | buffer.failbit | buffer.eofbit);
    
    
    tsl::sparse_set<move_only_test> set;
    for(std::size_t i = 0; i < nb_values + 40; i++) {
        set.insert(utils::get_key<move_only_test>(i));
    }
    
    for(std::size_t i = nb_values; i < nb_values + 40; i++) {
        set.erase(utils::get_key<move_only_test>(i));
    }
    BOOST_CHECK_EQUAL(set.size(), nb_values);
    
    set.serialize(serializer(), buffer);
    
    
    
    
    auto set_deserialized = decltype(set)::deserialize(serializer(), buffer, true);
    BOOST_CHECK(set_deserialized == set);
    
    buffer.seekg(0);
    set_deserialized = decltype(set)::deserialize(serializer(), buffer, false);
    BOOST_CHECK(set_deserialized == set);
}

BOOST_AUTO_TEST_CASE(test_serialize_desearialize_set_with_different_hash) {
    // insert x values; serialize set; deserialize in new set which has a different hash; check equal
    struct hash_str_with_size {
        std::size_t operator()(const move_only_test& v) const {
            return v.value().size();
        }
    };
    
    
    const std::size_t nb_values = 1000;
    
    std::stringstream buffer;
    buffer.exceptions(buffer.badbit | buffer.failbit | buffer.eofbit);
    
    
    tsl::sparse_set<move_only_test> set;
    for(std::size_t i = 0; i < nb_values; i++) {
        set.insert(utils::get_key<move_only_test>(i));
    }
    BOOST_CHECK_EQUAL(set.size(), nb_values);
    
    set.serialize(serializer(), buffer);
    
    
    
    
    auto set_deserialized = tsl::sparse_set<move_only_test, hash_str_with_size>::deserialize(serializer(), buffer);
    
    BOOST_CHECK_EQUAL(set_deserialized.size(), set.size());
    for(const auto& val: set) {
        BOOST_CHECK(set_deserialized.find(val) != set_deserialized.end());
    }
}


BOOST_AUTO_TEST_SUITE_END()
