[![Build Status](https://travis-ci.org/Tessil/sparse-map.svg?branch=master)](https://travis-ci.org/Tessil/sparse-map) [![Build status](https://ci.appveyor.com/api/projects/status/wp65gwsyyi7j1tli/branch/master?svg=true)](https://ci.appveyor.com/project/Tessil/sparse-map/branch/master)

## A C++ implementation of a memory efficient hash map and hash set

The sparse-map library is a C++ implementation of a memory efficient hash map and hash set. It uses open-addressing with sparse quadratic probing. The goal of the library is to be the most memory efficient possible, even at low load factor, while keeping reasonable performances. You can find an [article](https://smerity.com/articles/2015/google_sparsehash.html) of Stephen Merity which explains the idea behind `google::sparse_hash_map` and this project.

Four classes are provided: `tsl::sparse_map`, `tsl::sparse_set`, `tsl::sparse_pg_map` and `tsl::sparse_pg_set`. The first two are faster and use a power of two growth policy, the last two use a prime growth policy instead and are able to cope better with a poor hash function. Use the prime version if there is a chance of repeating patterns in the lower bits of your hash (e.g. you are storing pointers with an identity hash function). See [GrowthPolicy](#growth-policy) for details.

A **benchmark** of `tsl::sparse_map` against other hash maps may be found [here](https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html). The benchmark, in its additional tests page, notably includes `google::sparse_hash_map` and `spp::sparse_hash_map` to which `tsl::sparse_map` is an alternative. This page also gives some advices on which hash table structure you should try for your use case (useful if you are a bit lost with the multiple hash tables implementations in the `tsl` namespace).

### Key features

- Header-only library, just include the project to your include path and you are ready to go.
- Memory efficient while keeping good lookup speed, see the [benchmark](https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html) for some numbers.
- Support for heterogeneous lookups allowing to use `find` with a type different than `Key` (e.g. if you have a map that uses `std::unique_ptr<foo>` as key, you could use a `foo*` or a `std::uintptr_t` as key parameter to `find` without constructing a `std::unique_ptr<foo>`, see [example](#heterogeneous-lookups)).
- No need to reserve any sentinel value from the keys.
- If the hash is known before a lookup, it is possible to pass it as parameter to speed-up the lookup (see `precalculated_hash` parameter in [API](https://tessil.github.io/sparse-map/classtsl_1_1sparse__map.html)).
- Possibility to control the balance between insertion speed and memory usage with the `Sparsity` template parameter. A high sparsity means less memory but longer insertion times, and vice-versa for low sparsity. The default medium sparsity offers a good compromise (see [API](https://tessil.github.io/sparse-map/classtsl_1_1sparse__map.html#details) for details). For reference, with simple 64 bits integers as keys and values, a low sparsity offers ~15% faster insertions times but uses ~12% more memory. Nothing change regarding lookup speed.
- API closely similar to `std::unordered_map` and `std::unordered_set`.

### Differences compared to `std::unordered_map`

`tsl::sparse_map` tries to have an interface similar to `std::unordered_map`, but some differences exist.

- **By default only the basic exception safety is guaranteed** which mean that, in case of exception, all resources used by the hash map will be freed (no memory leaks) but the hash map may end-up in an undefined state (undefined here means that some elements may be missing). This can ONLY happen on rehash (either on insert or if `rehash` is called explicitly) and will occur if the `Allocator` can't allocate memory (`std::bad_alloc`) or if the copy constructor (when a nothrow move constructor is not available) throws and exception. This can be avoided by calling `reserve` beforehand. It is the same guarantee that the one provided by `google::sparse_hash_map` and `spp::sparse_hash_map` which don't provide the strong exception guarantee. For more information and if you need the strong exception guarantee, check the `ExceptionSafety` template parameter (see [API](https://tessil.github.io/sparse-map/classtsl_1_1sparse__map.html#details) for details).
- Iterator invalidation doesn't behave in the same way, any operation modifying the hash table invalidate them (see [API](https://tessil.github.io/sparse-map/classtsl_1_1sparse__map.html#details) for details).
- References and pointers to keys or values in the map are invalidated in the same way as iterators to these keys-values.
- For iterators, `operator*()` and `operator->()` return a reference and a pointer to `const std::pair<Key, T>` instead of `std::pair<const Key, T>` making the value `T` not modifiable. To modify the value you have to call the `value()` method of the iterator to get a mutable reference. Example:
```c++
tsl::sparse_map<int, int> map = {{1, 1}, {2, 1}, {3, 1}};
for(auto it = map.begin(); it != map.end(); ++it) {
    //it->second = 2; // Illegal
    it.value() = 2; // Ok
}
```
- Move-only types must have a nothrow move constructor.
- No support for some buckets related methods (like bucket_size, bucket, ...).

These differences also apply between `std::unordered_set` and `tsl::sparse_set`.

Thread-safety guarantees are the same as `std::unordered_map/set` (i.e. possible to have multiple readers with no writer).

### Optimization

#### Popcount
The library relies heavily on the [popcount](https://en.wikipedia.org/wiki/Hamming_weight) operation. 

With Clang and GCC, the library uses the `__builtin_popcount` function which will use the fast CPU instruction `POPCNT` when the library is compiled with `-mpopcnt`. Using the `POPCNT` instruction offers an improvement of ~15% to ~30% on lookups. So if you are compiling your code for a specific architecture that support the operation, don't forget the `-mpopcnt` (or `-march=native`) flag of your compiler.

On Windows with MSVC, the detection is done at runtime.

#### Move constructor
Make sure that your key `Key` and potential value `T` have a `noexcept` move constructor. The library will work without it but insertions will be much slower if the copy constructor is expensive (the structure often needs to move some values around on insertion).

### Growth policy

The library supports multiple growth policies through the `GrowthPolicy` template parameter. Three policies are provided by the library but you can easily implement your own if needed.

* **[tsl::sh::power_of_two_growth_policy.](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1power__of__two__growth__policy.html)** Default policy used by `tsl::sparse_map/set`. This policy keeps the size of the bucket array of the hash table to a power of two. This constraint allows the policy to avoid the usage of the slow modulo operation to map a hash to a bucket, instead of <code>hash % 2<sup>n</sup></code>, it uses <code>hash & (2<sup>n</sup> - 1)</code> (see [fast modulo](https://en.wikipedia.org/wiki/Modulo_operation#Performance_issues)). Fast but this may cause a lot of collisions with a poor hash function as the modulo with a power of two only masks the most significant bits in the end.
* **[tsl::sh::prime_growth_policy.](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1prime__growth__policy.html)** Default policy used by `tsl::sparse_pg_map/set`. The policy keeps the size of the bucket array of the hash table to a prime number. When mapping a hash to a bucket, using a prime number as modulo will result in a better distribution of the hash across the buckets even with a poor hash function. To allow the compiler to optimize the modulo operation, the policy use a lookup table with constant primes modulos (see [API](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1prime__growth__policy.html#details) for details). Slower than `tsl::sh::power_of_two_growth_policy` but more secure.
* **[tsl::sh::mod_growth_policy.](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1mod__growth__policy.html)** The policy grows the map by a customizable growth factor passed in parameter. It then just use the modulo operator to map a hash to a bucket. Slower but more flexible.


To implement your own policy, you have to implement the following interface.

```c++
struct custom_policy {
    // Called on the hash table creation and on rehash. The number of buckets for the table is passed in parameter.
    // This number is a minimum, the policy may update this value with a higher value if needed (but not lower).
    //
    // If 0 is given, min_bucket_count_in_out must still be 0 after the policy creation and
    // bucket_for_hash must always return 0 in this case.    
    explicit custom_policy(std::size_t& min_bucket_count_in_out);
    
    // Return the bucket [0, bucket_count()) to which the hash belongs. 
    // If bucket_count() is 0, it must always return 0.
    std::size_t bucket_for_hash(std::size_t hash) const noexcept;
    
    // Return the number of buckets that should be used on next growth
    std::size_t next_bucket_count() const;
    
    // Return the maximum number of buckets supported by the policy.
    std::size_t max_bucket_count() const;
    
    // Reset the growth policy as if it was created with a bucket count of 0.
    // After a clear, the policy must always return 0 when bucket_for_hash is called.
    void clear() noexcept;
}
```

### Installation

To use sparse-map, just add the project to your include path. It is a **header-only** library.

The code should work with any C++11 standard-compliant compiler and has been tested with GCC 4.8.4, Clang 3.5.0 and Visual Studio 2015.

To run the tests you will need the Boost Test library and CMake.

```bash
git clone https://github.com/Tessil/sparse-map.git
cd sparse-map
mkdir build
cd build
cmake ..
make
./test_sparse_map
```

### Usage

The API can be found [here](https://tessil.github.io/sparse-map/). 

All methods are not documented yet, but they replicate the behavior of the ones in `std::unordered_map` and `std::unordered_set`, except if specified otherwise.

### Example

```c++
#include <cstdint>
#include <iostream>
#include <string>
#include <tsl/sparse_map.h>
#include <tsl/sparse_set.h>

int main() {
    tsl::sparse_map<std::string, int> map = {{"a", 1}, {"b", 2}};
    map["c"] = 3;
    map["d"] = 4;
    
    map.insert({"e", 5});
    map.erase("b");
    
    for(auto it = map.begin(); it != map.end(); ++it) {
        //it->second += 2; // Not valid.
        it.value() += 2;
    }
    
    // {d, 6} {a, 3} {e, 7} {c, 5}
    for(const auto& key_value : map) {
        std::cout << "{" << key_value.first << ", " << key_value.second << "}" << std::endl;
    }
    
    if(map.find("a") != map.end()) {
        std::cout << "Found \"a\"." << std::endl;
    }
    
    const std::size_t precalculated_hash = std::hash<std::string>()("a");
    // If we already know the hash beforehand, we can pass it in parameter to speed-up lookups.
    if(map.find("a", precalculated_hash) != map.end()) {
        std::cout << "Found \"a\" with hash " << precalculated_hash << "." << std::endl;
    }
    
    
    
    
    tsl::sparse_set<int> set;
    set.insert({1, 9, 0});
    set.insert({2, -1, 9});
    
    // {0} {1} {2} {9} {-1}
    for(const auto& key : set) {
        std::cout << "{" << key << "}" << std::endl;
    }
}
```

#### Heterogeneous lookups

Heterogeneous overloads allow the usage of other types than `Key` for lookup and erase operations as long as the used types are hashable and comparable to `Key`.

To activate the heterogeneous overloads in `tsl::sparse_map/set`, the qualified-id `KeyEqual::is_transparent` must be valid. It works the same way as for [`std::map::find`](http://en.cppreference.com/w/cpp/container/map/find). You can either use [`std::equal_to<>`](http://en.cppreference.com/w/cpp/utility/functional/equal_to_void) or define your own function object.

Both `KeyEqual` and `Hash` will need to be able to deal with the different types.

```c++
#include <functional>
#include <iostream>
#include <string>
#include <tsl/sparse_map.h>


struct employee {
    employee(int id, std::string name) : m_id(id), m_name(std::move(name)) {
    }
    
    // Either we include the comparators in the class and we use `std::equal_to<>`...
    friend bool operator==(const employee& empl, int empl_id) {
        return empl.m_id == empl_id;
    }
    
    friend bool operator==(int empl_id, const employee& empl) {
        return empl_id == empl.m_id;
    }
    
    friend bool operator==(const employee& empl1, const employee& empl2) {
        return empl1.m_id == empl2.m_id;
    }
    
    
    int m_id;
    std::string m_name;
};

// ... or we implement a separate class to compare employees.
struct equal_employee {
    using is_transparent = void;
    
    bool operator()(const employee& empl, int empl_id) const {
        return empl.m_id == empl_id;
    }
    
    bool operator()(int empl_id, const employee& empl) const {
        return empl_id == empl.m_id;
    }
    
    bool operator()(const employee& empl1, const employee& empl2) const {
        return empl1.m_id == empl2.m_id;
    }
};

struct hash_employee {
    std::size_t operator()(const employee& empl) const {
        return std::hash<int>()(empl.m_id);
    }
    
    std::size_t operator()(int id) const {
        return std::hash<int>()(id);
    }
};


int main() {
    // Use std::equal_to<> which will automatically deduce and forward the parameters
    tsl::sparse_map<employee, int, hash_employee, std::equal_to<>> map; 
    map.insert({employee(1, "John Doe"), 2001});
    map.insert({employee(2, "Jane Doe"), 2002});
    map.insert({employee(3, "John Smith"), 2003});

    // John Smith 2003
    auto it = map.find(3);
    if(it != map.end()) {
        std::cout << it->first.m_name << " " << it->second << std::endl;
    }

    map.erase(1);



    // Use a custom KeyEqual which has an is_transparent member type
    tsl::sparse_map<employee, int, hash_employee, equal_employee> map2;
    map2.insert({employee(4, "Johnny Doe"), 2004});

    // 2004
    std::cout << map2.at(4) << std::endl;
}
```

### License

The code is licensed under the MIT license, see the [LICENSE file](LICENSE) for details.
