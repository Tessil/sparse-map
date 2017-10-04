## A C++ implementation of a memory efficient hash map and hash set

**The library is still in alpha stage.**


Four classes are provided: `tsl::sparse_map`, `tsl::sparse_set`, `tsl::sparse_pg_map` and `tsl::sparse_pg_set`. The first two are faster and use a power of two growth policy, the last two use a prime growth policy instead and are able to cope better with a poor hash function. Use the prime version if there is a chance of repeating patterns in the lower bits of your hash (e.g. you are storing pointers with an identity hash function). See [GrowthPolicy](#growth-policy) for details.

### Growth policy

The library supports multiple growth policies through the `GrowthPolicy` template parameter. Three policies are provided by the library but you can easly implement your own if needed.

* **[tsl::sh::power_of_two_growth_policy.](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1power__of__two__growth__policy.html)** Default policy used by `tsl::sparse_map/set`. This policy keeps the size of the bucket array of the hash table to a power of two. This constraint allows the policy to avoid the usage of the slow modulo operation to map a hash to a bucket, instead of <code>hash % 2<sup>n</sup></code>, it uses <code>hash & (2<sup>n</sup> - 1)</code> (see [fast modulo](https://en.wikipedia.org/wiki/Modulo_operation#Performance_issues)). Fast but this may cause a lot of collisions with a poor hash function as the modulo with a power of two only masks the most significant bits in the end.
* **[tsl::sh::prime_growth_policy.](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1prime__growth__policy.html)** Default policy used by `tsl::sparse_pg_map/set`. The policy keeps the size of the bucket array of the hash table to a prime number. When mapping a hash to a bucket, using a prime number as modulo will result in a better distribution of the hash across the buckets even with a poor hash function. To allow the compiler to optimize the modulo operation, the policy use a lookup table with constant primes modulos (see [API](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1prime__growth__policy.html#details) for details). Slower than `tsl::sh::power_of_two_growth_policy` but more secure.
* **[tsl::sh::mod_growth_policy.](https://tessil.github.io/sparse-map/classtsl_1_1sh_1_1mod__growth__policy.html)** The policy grows the map by a customizable growth factor passed in parameter. It then just use the modulo operator to map a hash to a bucket. Slower but more flexible.


To implement your own policy, you have to implement the following interface.

```c++
struct custom_policy {
    // Called on hash table construction, min_bucket_count_in_out is the minimum size
    // that the hash table needs. The policy can change it to a higher bucket count if needed
    custom_policy(std::size_t& min_bucket_count_in_out);
    
    // Return the bucket for the corresponding hash
    std::size_t bucket_for_hash(std::size_t hash) const noexcept;
    
    // Return the number of buckets that should be used on next growth
    std::size_t next_bucket_count() const;
    
    // Maximum number of buckets supported by the policy
    std::size_t max_bucket_count() const;
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

struct hash_employee {
    std::size_t operator()(const employee& empl) const {
        return std::hash<int>()(empl.m_id);
    }
    
    std::size_t operator()(int id) const {
        return std::hash<int>()(id);
    }
};

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
