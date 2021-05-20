/** @file
 * @bief Home of a custom allocator for testing with fancy pointers.
 */

#ifndef TSL_SPARSE_MAP_TESTS_CUSTOMALLOCATOR_H
#define TSL_SPARSE_MAP_TESTS_CUSTOMALLOCATOR_H

#include <boost/interprocess/offset_ptr.hpp>

/** A custom allocator that simply wraps all pointers into boost offset ptr.
* It is used to check whether the implementation can handle Allocators using fancy pointers.
* @tparam T Typical Allocator parameter.
*/
template<typename T>
struct OffsetAllocator {
    using value_type = T;
    template <typename P> using offset_ptr = boost::interprocess::offset_ptr<P>;
    using pointer = offset_ptr<value_type>;
    using const_pointer = offset_ptr<const value_type>;
    using void_pointer = offset_ptr<void>;
    using const_void_pointer = offset_ptr<const void>;
    using difference_type = typename offset_ptr<value_type>::difference_type;

    OffsetAllocator() noexcept = default;
    OffsetAllocator(OffsetAllocator const &) noexcept = default;
    OffsetAllocator(OffsetAllocator &&) noexcept = default;
    OffsetAllocator &operator=(OffsetAllocator const &) noexcept = default;
    OffsetAllocator &operator=(OffsetAllocator &&) noexcept = default;
    template<typename V>
    OffsetAllocator(OffsetAllocator<V>) noexcept {}

    pointer allocate(std::size_t n) {
        return pointer(static_cast<T*>(::operator new(n*sizeof(T))));
    }
    void deallocate(pointer p, std::size_t) noexcept {
        ::operator delete(p.get());
    }
    friend bool operator==(OffsetAllocator const &, OffsetAllocator const &) noexcept {
        return true;
    }
    friend bool operator!=(OffsetAllocator const &l, OffsetAllocator const &r) noexcept {
        return !(l == r);
    }
};

#endif //TSL_SPARSE_MAP_TESTS_CUSTOMALLOCATOR_H