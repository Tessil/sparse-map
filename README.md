## A C++ implementation of a memory efficient hash map and hash set

**The library is still in alpha stage.**


Two classes are provided: `tsl::sparse_map` and `tsl::sparse_set`.

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


### License

The code is licensed under the MIT license, see the [LICENSE file](LICENSE) for details.
