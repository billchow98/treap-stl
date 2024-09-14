// © 2023 Bill Chow. All rights reserved.
// Unauthorized use, modification, or distribution of this code is strictly
// prohibited.

#include <iterator> // std::distance
#include <set>      // std::set
#include <utility>  // std::make_pair

#include <gtest/gtest.h>

#include "../src/bst.h"

#include "debug_alloc.h"

namespace {

using debug_set [[maybe_unused]] = bst::set<int, std::less<>, DebugAlloc<std::pair<int, std::uint32_t>>>;

TEST(TreapSet, EmptyIsEmpty) {
    bst::set<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

TEST(TreapMap, EmptyIsEmpty) {
    bst::map<int, int> m;
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0);
}

TEST(TreapSet, EmptyBeginIsEnd) {
    bst::set<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.begin(), s.end());
}

TEST(TreapMap, EmptyBeginIsEnd) {
    bst::map<int, int> m;
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.begin(), m.end());
}

TEST(TreapSet, EmptyBeginIsEnd2) {
    bst::set<int> s;
    s.insert(0);
    EXPECT_EQ(s.size(), 1);
    s.erase(0);
    EXPECT_EQ(s.begin(), s.end());
}

TEST(TreapMap, EmptyBeginIsEnd2) {
    bst::map<int, int> m;
    m.insert(std::make_pair(0, 1));
    EXPECT_EQ(m.size(), 1);
    m.erase(0);
    EXPECT_EQ(m.begin(), m.end());
}

TEST(TreapSet, UpdatedSize) {
    bst::set<int> s;
    EXPECT_EQ(s.size(), 0);
    for (int i = 1; i <= 10; i++) {
        s.insert(i);
        EXPECT_EQ(s.size(), i);
    }
    while (!s.empty()) {
        const int old_sz = static_cast<int>(s.size());
        s.erase(s.begin());
        EXPECT_EQ(s.size(), old_sz - 1);
    }
}

TEST(TreapMap, UpdatedSize) {
    bst::map<int, int> m;
    EXPECT_EQ(m.size(), 0);
    for (int i = 1; i <= 10; i++) {
        m.insert(std::make_pair(i, i));
        EXPECT_EQ(m.size(), i);
    }
    while (!m.empty()) {
        const int old_sz = static_cast<int>(m.size());
        m.erase(m.begin());
        EXPECT_EQ(m.size(), old_sz - 1);
    }
}

TEST(TreapSet, FindPointsToSame) {
    bst::set<int> s;
    s.insert(1);
    auto it1 = s.find(1);
    s.insert(2);
    auto it2 = s.find(1);
    EXPECT_EQ(it1, it2);
}

TEST(TreapMap, FindPointsToSame) {
    bst::map<int, int> m;
    m.insert(std::make_pair(1, 2));
    auto it1 = m.find(1);
    m.insert(std::make_pair(2, 3));
    auto it2 = m.find(1);
    EXPECT_EQ(it1, it2);
}

TEST(TreapSet, CantFindReturnsEnd) {
    bst::set<int> s;
    auto it = s.find(42);
    EXPECT_EQ(it, s.end());
}

TEST(TreapMap, CantFindReturnsEnd) {
    bst::map<int, int> m;
    auto it = m.find(42);
    EXPECT_EQ(it, m.end());
}

TEST(TreapSet, SizeIsBeginEndDistance) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    EXPECT_EQ(s.size(), std::distance(s.begin(), s.end()));
}

TEST(TreapMap, SizeIsBeginEndDistance) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    EXPECT_EQ(m.size(), std::distance(m.begin(), m.end()));
}

TEST(TreapSet, AfterLastIsEnd) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    auto it = s.find(9);
    EXPECT_EQ(std::next(it), s.end());
}

TEST(TreapMap, AfterLastIsEnd) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    auto it = m.find(9);
    EXPECT_EQ(std::next(it), m.end());
}

TEST(TreapSet, BeforeEndIsLast) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    auto it = s.find(9);
    EXPECT_EQ(it, std::prev(s.end()));
}

TEST(TreapMap, BeforeEndIsLast) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    auto it = m.find(9);
    EXPECT_EQ(it, std::prev(m.end()));
}

TEST(TreapSet, IteratorIncWorks) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    auto it = s.begin();
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(*it, i);
        it++;
    }
}

TEST(TreapMap, IteratorIncWorks) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 2 * i));
    }
    auto it = m.begin();
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(it->first, i);
        EXPECT_EQ(it->second, 2 * i);
        it++;
    }
}

TEST(TreapSet, IteratorDecWorks) {
    bst::set<int> s;
    for (int i = 9; i >= 0; i--) {
        s.insert(i);
    }
    auto it = s.end();
    for (int i = 9; i >= 0; i--) {
        it--;
        EXPECT_EQ(*it, i);
    }
}

TEST(TreapMap, IteratorDecWorks) {
    bst::map<int, int> m;
    for (int i = 9; i >= 0; i--) {
        m.insert(std::make_pair(i, 2 * i));
    }
    auto it = m.end();
    for (int i = 9; i >= 0; i--) {
        it--;
        EXPECT_EQ(it->first, i);
        EXPECT_EQ(it->second, 2 * i);
    }
}

TEST(TreapSet, MultipleInsertsSizeUnchanged) {
    bst::set<int> s;
    for (int i = 0; i < 100; i++) {
        s.insert(0);
    }
    EXPECT_EQ(s.size(), 1);
}

TEST(TreapMap, MultipleInsertsSizeUnchanged) {
    bst::map<int, int> m;
    for (int i = 0; i < 100; i++) {
        m.insert(std::make_pair(0, i));
    }
    EXPECT_EQ(m.size(), 1);
}

TEST(TreapSet, SizeCorrectInsertsOnly) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    EXPECT_EQ(s.size(), 10);
}

TEST(TreapMap, SizeCorrectInsertsOnly) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, i));
    }
    EXPECT_EQ(m.size(), 10);
}

TEST(TreapSet, SizeCorrectErasesOnly) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    for (int i = 9; i >= 0; i--) {
        s.erase(i);
        EXPECT_EQ(s.size(), i);
    }
}

TEST(TreapMap, SizeCorrectErasesOnly) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    for (int i = 9; i >= 0; i--) {
        m.erase(i);
        EXPECT_EQ(m.size(), i);
    }
}

TEST(TreapSet, EmptyMeansSizeZero) {
    bst::set<int> s;
    EXPECT_EQ(s.empty(), s.size() == 0); // NOLINT(readability-container-size-empty)
}

TEST(TreapMap, EmptyMeansSizeZero) {
    bst::map<int, int> m;
    EXPECT_EQ(m.empty(), m.size() == 0); // NOLINT(readability-container-size-empty)
}

TEST(TreapSet, AddAndEraseToEmpty) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    int i = 0;
    for (auto it = s.begin(); it != s.end(); ) {
        EXPECT_EQ(*it, i);
        it = s.erase(it);
        i++;
    }
    EXPECT_TRUE(s.empty());
}

TEST(TreapMap, AddAndEraseToEmpty) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    int i = 0;
    for (auto it = m.begin(); it != m.end(); ) {
        EXPECT_EQ(it->first, i);
        EXPECT_EQ(it->second, 0);
        it = m.erase(it);
        i++;
    }
    EXPECT_TRUE(m.empty());
}

TEST(TreapSet, BackForthIterIsSameInsertOnly) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        const auto [it, ok] = s.insert(i);
        EXPECT_TRUE(ok);
        EXPECT_NE(it, s.end());
        EXPECT_EQ(it, std::prev(std::next(it)));
    }
}

TEST(TreapMap, BackForthIterIsSameInsertOnly) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        const auto [it, ok] = m.insert(std::make_pair(i, 0));
        EXPECT_TRUE(ok);
        EXPECT_NE(it, m.end());
        EXPECT_EQ(it, std::prev(std::next(it)));
    }
}

TEST(TreapSet, BackForthIterIsSameEraseOnly) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(i);
    }
    int i = 0;
    for (auto it = s.begin(); it != s.end(); ) {
        it = s.erase(it);
        EXPECT_EQ(it, s.find(i + 1));
        if (i + 1 < 10) {
            EXPECT_NE(it, s.end());
            EXPECT_EQ(it, std::prev(std::next(it)));
        }
        i++;
    }
}

TEST(TreapMap, BackForthIterIsSameEraseOnly) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    int i = 0;
    for (auto it = m.begin(); it != m.end(); ) {
        it = m.erase(it);
        EXPECT_EQ(it, m.find(i + 1));
        if (i + 1 < 10) {
            EXPECT_NE(it, m.end());
            EXPECT_EQ(it, std::prev(std::next(it)));
        }
        i++;
    }
}

TEST(TreapSet, LowerBoundTest) {
    bst::set<int> s;
    for (int i = 9; i >= 1; i -= 2) {
        s.insert(i);
    }
    const int ans[] = {1, 1, 3, 3, 5, 5, 7, 7, 9, 9 };
    for (int i = 0; i <= 9; i++) {
        EXPECT_EQ(*s.lower_bound(i), ans[i]);
    }
    EXPECT_EQ(s.lower_bound(10), s.end());
}

TEST(TreapMap, LowerBoundTest) {
    bst::map<int, int> m;
    for (int i = 9; i >= 1; i -= 2) {
        m.insert(std::make_pair(i, 0));
    }
    const int ans[] = {1, 1, 3, 3, 5, 5, 7, 7, 9, 9 };
    for (int i = 0; i <= 9; i++) {
        EXPECT_EQ(m.lower_bound(i)->first, ans[i]);
    }
    EXPECT_EQ(m.lower_bound(10), m.end());
}

TEST(TreapSet, UpperBoundTest) {
    bst::set<int> s;
    for (int i = 9; i >= 1; i -= 2) {
        s.insert(i);
    }
    const int ans[] = {1, 3, 3, 5, 5, 7, 7, 9, 9 };
    for (int i = 0; i <= 8; i++) {
        EXPECT_EQ(*s.upper_bound(i), ans[i]);
    }
    EXPECT_EQ(s.upper_bound(9), s.end());
    EXPECT_EQ(s.upper_bound(10), s.end());
}

TEST(TreapMap, UpperBoundTest) {
    bst::map<int, int> m;
    for (int i = 9; i >= 1; i -= 2) {
        m.insert(std::make_pair(i, 0));
    }
    const int ans[] = {1, 3, 3, 5, 5, 7, 7, 9, 9 };
    for (int i = 0; i <= 8; i++) {
        EXPECT_EQ(m.upper_bound(i)->first, ans[i]);
    }
    EXPECT_EQ(m.upper_bound(9), m.end());
    EXPECT_EQ(m.upper_bound(10), m.end());
}

TEST(TreapSet, EraseNonExistent) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.erase(i);
    }
    EXPECT_TRUE(s.empty());
}

TEST(TreapMap, EraseNonExistent) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.erase(i);
    }
    EXPECT_TRUE(m.empty());
}

TEST(TreapSet, InsertThroughGoodHint) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(s.end(), i);
    }
    EXPECT_EQ(s.size(), 10);
    for (int i = 0; i < 10; i++) {
        EXPECT_NE(s.find(i), s.end());
    }
}

TEST(TreapMap, InsertThroughGoodHint) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(m.end(), std::make_pair(i, i));
    }
    EXPECT_EQ(m.size(), 10);
    for (int i = 0; i < 10; i++) {
        EXPECT_NE(m.find(i), m.end());
    }
}

TEST(TreapSet, InsertThroughBadHint) {
    bst::set<int> s;
    for (int i = 0; i < 10; i++) {
        s.insert(s.begin(), i);
    }
    EXPECT_EQ(s.size(), 10);
    for (int i = 0; i < 10; i++) {
        EXPECT_NE(s.find(i), s.end());
    }
}

TEST(TreapMap, InsertThroughBadHint) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(m.begin(), std::make_pair(i, i));
    }
    EXPECT_EQ(m.size(), 10);
    for (int i = 0; i < 10; i++) {
        EXPECT_NE(m.find(i), m.end());
    }
}

// Map specific tests
TEST(TreapMap, ModifyThroughIterator) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 0));
    }
    auto it = m.find(3);
    it->second = 42;
    auto it2 = m.find(3);
    EXPECT_EQ(it, it2);
    EXPECT_EQ(it2->second, 42);
    EXPECT_EQ(m.size(), 10);
}

TEST(TreapMap, InsertSameKeySameSize) {
    bst::map<int, int> m;
    m.insert(std::make_pair(1, 2));
    EXPECT_EQ(m.size(), 1);
    auto it = m.find(1);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, 2);
    auto [it2, ok] = m.insert(std::make_pair(1, 3));
    EXPECT_FALSE(ok);
    EXPECT_EQ(m.size(), 1);
    auto it3 = m.find(1);
    EXPECT_EQ(it, it3);
    EXPECT_EQ(it3->first, 1);
    EXPECT_EQ(it3->second, 2); // If element with equivalent key exists, it should fail
}

TEST(TreapMap, GetThroughIndexing) {
    bst::map<int, int> m;
    for (int i = 0; i < 10; i++) {
        m.insert(std::make_pair(i, 2 * i));
    }
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(m[i], 2 * i);
    }
}

TEST(TreapMap, CreateIfIndexingFails) {
    bst::map<int, int> m;
    EXPECT_EQ(m.find(0), m.end());
    EXPECT_TRUE(m.empty());
    m[0] = 123;
    EXPECT_NE(m.find(0), m.end());
    EXPECT_EQ(m.find(0)->second, 123);
    EXPECT_EQ(m.size(), 1);
}

TEST(TreapMap, ModifyThroughIndex) {
    bst::map<int, int> m;
    m[0] = 123;
    EXPECT_NE(m.find(0), m.end());
    EXPECT_EQ(m.find(0)->second, 123);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(m[0], 123);
    m[0] = 1;
    EXPECT_NE(m.find(0), m.end());
    EXPECT_EQ(m.find(0)->second, 1);
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(m[0], 1);
}

TEST(TreapMap, RandomFixedSeedKiller) {
    bst::map<int, int> m;
    std::set<int> s;
    std::minstd_rand g;
    for (int i = 0; i < (1 << 2); i++) {
        int tmp1 = g(), tmp2 = g();
        s.insert(tmp1);
        m.insert(std::make_pair(tmp1, tmp2));
    }
    EXPECT_EQ(m.size(), s.size());
}

// TODO: Tests with custom Compare

}