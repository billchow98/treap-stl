// © 2023 Bill Chow. All rights reserved.
// Unauthorized use, modification, or distribution of this code is strictly
// prohibited.

#ifndef BST_DEBUG_ALLOC_H
#define BST_DEBUG_ALLOC_H

#include <cstddef>  // std::size_t
#include <cstdio>   // std::fflush, std::fprintf
#include <cstdlib>  // std::exit

#include <unordered_map> // std::unordered_map

#include "strndup.h"
#include "type_name.h"

template<class T>
class DebugAlloc {
public:
    using value_type = T;

    DebugAlloc() noexcept = default;

    template<class U>
    DebugAlloc(const DebugAlloc<U> &) noexcept {} // NOLINT(google-explicit-constructor)

    static T* allocate(std::size_t n) {
        void *p = ::operator new(n);
        std::fprintf(stderr,
                     "allocate(%zu) -> %zu bytes of memory"
                     " for %s"
                     " at location 0x%p\n",
                     n,
                     sizeof(T) * n,
                     name,
                     p);
        std::fflush(stderr);
        u[p]++;
        return static_cast<T*>(p);
    }

    template<class U, class... Args>
    static void construct(U *p, Args &&...args) {
        ::new(p) U(std::forward<Args>(args)...);
    }

    template<class U>
    static void destroy(U *p) {
        p->~U();
    }

    static void deallocate(T *p, std::size_t n) {
        if (auto it = u.find(p); u.find(p) != u.end()) {
            assert(it->second == 1);
            u.erase(it);
        } else {
            std::fprintf(stderr,
                         "Fatal error: pointer 0x%p passed to deallocate() already freed!\n",
                         p);
            std::fflush(stderr);
            std::exit(EXIT_FAILURE);
        }
        std::fprintf(stderr,
                     "deallocate(%zu) -> %zu bytes of memory"
                     " for %s"
                     " at location 0x%p\n",
                     n,
                     sizeof(T) * n,
                     name,
                     p);
        std::fflush(stderr);
        ::operator delete(p);
    }

private:
    inline static std::unordered_map<void *, int> u;
    inline static const char *const name = strndup(type_name<T>().data(), type_name<T>().length() + 1);
};

#endif //BST_DEBUG_ALLOC_H
