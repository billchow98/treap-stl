// © 2023 Bill Chow. All rights reserved.
// Unauthorized use, modification, or distribution of this code is strictly
// prohibited.

#ifndef BST_STRNDUP_H
#define BST_STRNDUP_H

#include <cstddef> // std::size_t

#include <new> // ::operator new[]

char *strndup(const char *s, std::size_t n) {
    if (s == nullptr) {
        return nullptr;
    }

    std::size_t len = n;
    for (int i = 0; i < n; i++) {
        if (s[i] == '\x00') {
            len = i;
            break;
        }
    }

    char *p = static_cast<char *>(::operator new[](len + 1));
    p[len] = '\x00';

    std::copy_n(s, len, p);

    return p;
}

#endif //BST_STRNDUP_H
