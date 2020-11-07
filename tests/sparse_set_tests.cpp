/**
 * MIT License
 *
 * Copyright (c) 2017 Thibaut Goetghebuer-Planchon <tessil@gmx.com>
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
    const tsl::sparse_set<std::string> set1 = {"a", "e", "d", "c", "b"};
    const tsl::sparse_set<std::string> set1_copy = {"e", "c", "b", "a", "d"};
    const tsl::sparse_set<std::string> set2 = {"e", "c", "b", "a", "d", "f"};
    const tsl::sparse_set<std::string> set3 = {"e", "c", "b", "a"};
    const tsl::sparse_set<std::string> set4 = {"a", "e", "d", "c", "z"};

    BOOST_CHECK(set1 == set1_copy);
    BOOST_CHECK(set1_copy == set1);

    BOOST_CHECK(set1 != set2);
    BOOST_CHECK(set2 != set1);

    BOOST_CHECK(set1 != set3);
    BOOST_CHECK(set3 != set1);

    BOOST_CHECK(set1 != set4);
    BOOST_CHECK(set4 != set1);

    BOOST_CHECK(set2 != set3);
    BOOST_CHECK(set3 != set2);

    BOOST_CHECK(set2 != set4);
    BOOST_CHECK(set4 != set2);

    BOOST_CHECK(set3 != set4);
    BOOST_CHECK(set4 != set3);
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
BOOST_AUTO_TEST_CASE(test_serialize_deserialize_reserve) {
    // Insert a number of values without intermediate resizes.
    for(std::size_t nb_values : {0, 1, 3, 17, 1000}) {
        tsl::sparse_set<move_only_test> set;
        set.reserve(nb_values);
        for(std::size_t i = 0; i < nb_values; i++) {
            set.insert(utils::get_key<move_only_test>(i));
        }


        serializer serial;
        set.serialize(serial);

        deserializer dserial(serial.str());
        auto set_deserialized = decltype(set)::deserialize(dserial, true);
        BOOST_CHECK(set == set_deserialized);

        deserializer dserial2(serial.str());
        set_deserialized = decltype(set)::deserialize(dserial2, false);
        BOOST_CHECK(set_deserialized == set);
    }
}


BOOST_AUTO_TEST_CASE(test_serialize_deserialize) {
    // insert x values; delete some values; serialize set; deserialize in new
    // set; check equal. for deserialization, test it with and without hash
    // compatibility.
    for(std::size_t nb_values : {0, 1, 3, 17, 1000}) {
        tsl::sparse_set<move_only_test> set;
        for(std::size_t i = 0; i < nb_values + 40; i++) {
            set.insert(utils::get_key<move_only_test>(i));
        }

        for(std::size_t i = nb_values; i < nb_values + 40; i++) {
            set.erase(utils::get_key<move_only_test>(i));
        }
        BOOST_CHECK_EQUAL(set.size(), nb_values);



        serializer serial;
        set.serialize(serial);

        deserializer dserial(serial.str());
        auto set_deserialized = decltype(set)::deserialize(dserial, true);
        BOOST_CHECK(set == set_deserialized);

        deserializer dserial2(serial.str());
        set_deserialized = decltype(set)::deserialize(dserial2, false);
        BOOST_CHECK(set_deserialized == set);
    }
}


BOOST_AUTO_TEST_SUITE_END()
