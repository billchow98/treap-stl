// © 2023 Bill Chow. All rights reserved.
// Unauthorized use, modification, or distribution of this code is strictly
// prohibited.

#ifndef BST_BST_H
#define BST_BST_H

#include <cassert>  // assert
#include <climits>  // UINT32_MAX
#include <cstddef>  // std::ptrdiff_t, std::size_t

#include <algorithm>   // std::less
#include <iterator>    // std::bidirectional_iterator_tag, std::next, std::prev
#include <memory>      // std::allocator, std::allocator_traits::{allocate, construct, deallocate, rebind_alloc}
#include <random>      // std::minstd_rand
#include <tuple>       // std::forward_as_tuple, std::ignore, std::make_tuple, std::tie, std::tuple
#include <type_traits> // std::enable_if_t, std::is_const_v, std::is_same_v, std::remove_const_t, std::remove_cv_t
#include <utility>     // std::pair, std::piecewise_construct

namespace bst {

namespace impl {

// We use template specialisation on this not void so user can't accidentally do
// map<int, void> for instance
struct null_type {};

// TODO: 1. Extend with extra stuff like tree-order statistics
template<class Key, class T, class Compare, class Allocator>
class treap {
private:
    struct node;

    template<class U>
    struct treap_iter;

    template<class U>
    static constexpr auto is_null_type = std::is_same_v<U, null_type>;

    template<class U, class Enable = void>
    struct value_type_of {};

    template<class U>
    struct value_type_of<U, std::enable_if_t<!is_null_type<U>>> { using type = std::pair<const Key, T>; };

    template<class U>
    struct value_type_of<U, std::enable_if_t<is_null_type<U>>> { using type = const Key; };

public:
    using value_type = typename value_type_of<T>::type;
    using size_type = std::size_t;
    using allocator_type = Allocator;
    using iterator = treap_iter<node>;
    using const_iterator = treap_iter<const node>;

private:
    // Concepts checks
#if __cplusplus > 201703L || defined(__STRICT_ANSI__)
    template<class U = T>
    static constexpr std::enable_if_t<is_null_type<U>, bool> check_static_asserts() {
        static_assert(std::is_same_v<std::remove_cv_t<Key>, Key>, "bst::set must have a non-const, non-volatile value_type");
        static_assert(std::is_same_v<typename Allocator::value_type, Key>, "bst::set must have the same value_type as its allocator");
        return true;
    }

    template<class U = T>
    static constexpr std::enable_if_t<!is_null_type<U>, bool> check_static_asserts() {
        static_assert(std::is_same_v<typename Allocator::value_type, value_type>, "bst::map must have the same value_type as its allocator");
        return true;
    }

    static_assert(check_static_asserts());
#endif

public:
    static_assert(!is_null_type<Key>, "class Key cannot be null_type");

    treap() : header({}, {}) {
        header.pri = UINT32_MAX;
        header.left = &header;
        header.right = &header;
        assert(header.par == nullptr);
    }

    ~treap() {
        if (root() != nullptr) {
            assert(!empty());
            destroy_node(root());
        } else {
            assert(empty());
        }
    }

    // TODO: Fix the Big 5

    // Finds an element with key equivalent to key
    [[nodiscard]] iterator find(const Key &key) {
        iterator lb = lower_bound(key);
        return lb != end() && lb.node->key() == key ? lb : end();
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    // Returns an iterator pointing to the first element that is not less than
    // (i.e. greater or equal to) key
#ifdef INSTRUMENT_DEPTH
    inline static std::size_t down = 0;
    inline static std::size_t called = 0;
#endif
    [[nodiscard]] iterator lower_bound(const Key &key) {
#ifdef INSTRUMENT_DEPTH
        called++;
#endif
        iterator rt{root()};
        iterator res = end();
        while (rt.node != nullptr) {
            assert(rt.node != rt.node->left);
            assert(rt.node != rt.node->right);
            if (!Compare()(rt.node->key(), key)) { // Basically key <= rt.node->key()
                res = rt;
                rt.node = rt.node->left;
            } else {
                rt.node = rt.node->right;
            }
#ifdef INSTRUMENT_DEPTH
            down++;
#endif
        }
        return res;
    }

    // Returns an iterator pointing to the first element that is greater than key
    [[nodiscard]] iterator upper_bound(const Key &key) {
        iterator rt{root()};
        iterator res = end();
        while (rt.node != nullptr) {
            assert(rt.node != rt.node->left);
            assert(rt.node != rt.node->right);
            if (Compare()(key, rt.node->key())) {
                res = rt;
                rt.node = rt.node->left;
            } else {
                rt.node = rt.node->right;
            }
        }
        return res;
    }
#pragma clang diagnostic pop

    // Insertion fails when an element with the same key already exists
    // In that case, the returned iterator points to that element
    std::pair<iterator, bool> insert(const value_type &value) {
        const Key &key = key_of(value);
        iterator it = lower_bound(key);
        // Return if element already exists
        if (it != end() && key_of(*it) == key) {
            return {it, false};
        }
        return {insert_(it, value), true};
    }

    // Inserts value in the position as close as possible to the position
    // just prior to pos
    // Returns an iterator to the inserted element, or to the element
    // that prevented the insertion
    iterator insert(const_iterator pos, const value_type &value) {
        const Key &key = key_of(value);
        // Replace hint with default (lower_bound) if it is bad
        // i.e. !(key < iterator's key) or !(prev(iterator)'s key < key)
        if (!Compare()(key, key_of(*pos)) || !(pos == begin() || Compare()(key_of(*std::prev(pos)), key))) {
            pos = lower_bound(key);
        }
        // Return if element already exists
        if (pos != end() && key_of(*pos) == key) {
            return iterator{pos.node};
        }
        return insert_(pos, value);
    }

    // Removes the element at pos
    // Returns the iterator following the last removed element
    iterator erase(iterator pos) {
        return erase_(pos);
    }

    // Returns the number of elements removed (0 or 1)
    // 0 elements are removed when no element with key exists
    size_type erase(const Key &key) {
        iterator it = find(key);
        if (it == end()) {
            return 0;
        }
        std::ignore = erase_(it);
        return 1;
    }

    [[nodiscard]] iterator begin() noexcept {
        return iterator{n_begin()};
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator{n_begin()};
    }

    [[nodiscard]] iterator end() noexcept {
        return iterator{&header};
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator{&header};
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    // Don't use size() == 0 as after create_node it will be inaccurate
    [[nodiscard]] bool empty() const noexcept {
        return begin() == end();
    }

    template<typename U = T>
    typename std::enable_if_t<!is_null_type<U>, U&> operator[](const Key &key) {
        if (auto it = find(key) ; it != end()) {
            return it->second;
        }
        const value_type new_val(std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>());
        auto [it, ok] = insert(new_val);
        assert(ok);
        return it->second;
    }

    [[nodiscard]] allocator_type get_allocator() const noexcept {
        return {allocator};
    }

private:
    // Just remember that incrementing treap_iter does an in-order traversal
    template<class U>
    struct treap_iter {
    public:
        template<class V>
        treap_iter(const treap_iter<V> &rhs) : treap_iter(rhs.node) {} // NOLINT(google-explicit-constructor)

        constexpr bool operator==(const treap_iter &rhs) const {
            return node == rhs.node;
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"

        constexpr bool operator!=(const treap_iter &rhs) const {
            return !(*this == rhs);
        }

#pragma clang diagnostic pop

    private:
        // SFINAE
        template<class V, class Enable = void>
        struct value_type_of {};

        template<class V>
        struct value_type_of<V, std::enable_if_t<!std::is_const_v<V>>> { using type = treap::value_type; };

        template<class V>
        struct value_type_of<V, std::enable_if_t<std::is_const_v<V>>> { using type = const treap::value_type; };

    public:
        // LegacyIterator requirements

        // typedefs
        using value_type = typename value_type_of<U>::type;
        using difference_type [[maybe_unused]] = std::ptrdiff_t;
        using reference = value_type &;
        using pointer = value_type *;
        using iterator_category [[maybe_unused]] = std::bidirectional_iterator_tag;

        template<class V = U, std::enable_if_t<!std::is_const_v<V>, bool> = true>
        // *it
        [[nodiscard]] reference operator*() {
            return node->record;
        }

        template<class V = U, std::enable_if_t<std::is_const_v<V>, bool> = true>
        // *it
        [[nodiscard]] reference operator*() const {
            return node->record;
        }

        // ++it
        // Find node with the smallest key greater than current
        // Assume it != end()
        treap_iter &operator++() {
            // Case 1: Has no right child -> Largest in left subtree; go up to parent of this subtree
            if (node->right == nullptr) {
                while (node->par->right == node) {
                    node = node->par;
                }
                // Special case when std::next(iterator) == end()
                // Things work when this is the case but node->par != node->right
                // so we let that fallthrough
                if (node->par == node->right) {
                    return *this;
                }
                node = node->par;
                return *this;
            }
            // Case 2: Has right child -> Get smallest in right subtree
            node = node->right;
            while (node->left != nullptr) {
                node = node->left;
            }
            return *this;
        }

        // LegacyInputIterator requirements

        // it->m
        pointer operator->() {
            return &**this;
        }

        // LegacyForwardIterator requirements

        // it++
        treap_iter operator++(int) { // NOLINT(cert-dcl21-cpp)
            treap_iter tmp = *this;
            ++*this;
            return tmp;
        }

        // LegacyBidirectionalIterator requirements

        // --it
        // Find node with the largest key smaller than current
        // Assume it != begin()
        treap_iter &operator--() {
            // Case 1: Has no left child -> Smallest in right subtree; go up to parent of this subtree
            if (node->left == nullptr) {
                while (node->par->left == node) {
                    node = node->par;
                }
                node = node->par;
                return *this;
            }
            // Special case when iterator == end()
            // Things work when this is the case but node->left->par == node
            // so we let that fallthrough
            if (node->left->par != node) {
                node = node->right;
                return *this;
            }
            // Case 2: Has left child -> Get largest in left subtree
            node = node->left;
            while (node->right != nullptr) {
                node = node->right;
            }
            return *this;
        }

        // it--
        treap_iter operator--(int) { // NOLINT(cert-dcl21-cpp)
            treap_iter tmp = *this;
            --*this;
            return tmp;
        }

    private:
        friend class treap;

        treap_iter() = default;

        explicit treap_iter(U *_node) : node(const_cast<std::remove_const_t<U> *>(_node)) {}

        std::remove_const_t<U> *node{};
    };

    using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<node>;

    struct node {
        using priority = std::uint32_t;

        explicit node(const value_type &value, node *_par)
                : record(value),
                  pri(treap::generator()),
                  par(_par) {
            assert(this != par);
        }

        template<class U = T, std::enable_if_t<!is_null_type<U>, bool> = true>
        [[nodiscard]] const Key &key() const {
            return record.first;
        }

        template<class U = T, std::enable_if_t<is_null_type<U>, bool> = true>
        [[nodiscard]] const Key &key() const {
            return record;
        }

        value_type record{};
        node       *left{};
        node       *right{};
        node       *par{};
        priority   pri{};
    };
    static_assert(sizeof(node) == 40);

    [[nodiscard]] node *&root() {
        if (header.par && header.par->par != &header) {
            assert(false);
        }
        assert(header.par == nullptr || header.par->par == &header);
        return header.par;
    }

    [[nodiscard]] node *&n_begin() {
        assert(header.left != nullptr);
        return header.left;
    }

    [[nodiscard]] const node *n_begin() const {
        assert(header.left != nullptr);
        return header.left;
    }

    [[nodiscard]] node *&n_rightmost() {
        assert(header.right != nullptr);
        return header.right;
    }

    [[nodiscard]] const node *n_rightmost() const {
        assert(header.right != nullptr);
        return header.right;
    }

    [[nodiscard]] const_iterator rightmost() const {
        return const_iterator{n_rightmost()};
    }

    // Auxiliary operation: Time complexity O(log n)
    // Requires all keys in lhs <= all keys in rhs
    // If we call split() and get (ll, rr), calling
    // merge(ll, rr) is guaranteed to meet this condition
    [[nodiscard]] node *merge(node *lhs, node *rhs) {
        if (lhs == nullptr || rhs == nullptr) {
            return lhs != nullptr ? lhs : rhs;
        }
        assert(lhs->left == nullptr || lhs->left->par == lhs);
        assert(lhs->right == nullptr || lhs->right->par == lhs);
        assert(rhs->left == nullptr || rhs->left->par == rhs);
        assert(rhs->right == nullptr || rhs->right->par == rhs);
        // lhs has to be subtree of rhs
        // Merge lhs and rhs->left to form new rhs->left
        // Return new root rhs
        // Don't use Compare because we always use our own priorities and < operator
        if (lhs->pri < rhs->pri) {
            assign_and_keep(rhs->left, merge(lhs, rhs->left), rhs);
            return rhs;
        }
        // rhs has to be subtree of lhs
        // Merge lhs->right and rhs to form new lhs->right
        // Return new root lhs
        assign_and_keep(lhs->right, merge(lhs->right, rhs), lhs);
        return lhs;
    }

    template<class U = T>
    [[nodiscard]] std::enable_if_t<!is_null_type<U>, const Key &> key_of(const value_type &value) {
        return value.first;
    }

    template<class U = T>
    [[nodiscard]] std::enable_if_t<is_null_type<U>, const Key &> key_of(const value_type &value) {
        return value;
    }

    // Helper tree rotation functions
    void rotate_right(node *&par, node *&child) {
        assert(par != &header);
        assert(child != &header);
        assert(par->left == child);
        assert(child->par == par);
        // Special root handling
        if (par->par->par == par) {
            par->par->par = child;
        } else if (par->par->left == par) {
            par->par->left = child;
        } else {
            assert(par->par->right == par);
            par->par->right = child;
        }

        if (child->right == nullptr) {
            std::tie(child->par, child->right, par->par, par->left)
                    = std::make_tuple(par->par, par, child, child->right);
            assert(par->par != nullptr);
        } else {
            std::tie(child->par, child->right, child->right->par, par->par, par->left)
                    = std::make_tuple(par->par, par, par, child, child->right);
        }
    }

    void rotate_left(node *&par, node *&child) {
        assert(par != &header);
        assert(child != &header);
        assert(par->right == child);
        assert(child->par == par);
        // Special root handling
        if (par->par->par == par) {
            par->par->par = child;
        } else if (par->par->left == par) {
            par->par->left = child;
        } else {
            assert(par->par->right == par);
            par->par->right = child;
        }

        if (child->left == nullptr) {
            std::tie(par->par, par->right, child->par, child->left)
                    = std::make_tuple(child, child->left, par->par, par);
        } else {
            std::tie(par->par, par->right, child->par, child->left, child->left->par)
                    = std::make_tuple(child, child->left, par->par, par, par);
        }
    }

    // Assumes that pos really points to the position right after where value is to be inserted
    // TODO: See if we can just do the 1st method: Split at correct position & call merge()
    //  Use that if the performance is the same. That way we can eliminate the tree rotation code.
    [[nodiscard]] iterator insert_(const_iterator pos, const value_type &value) {
        const Key &key = key_of(value);

        // Make new node from value_type
        node *node_ = create_node(value, nullptr);

        iterator it{node_};

        // par == end() corner cases
        if (empty()) {
            it.node->par = &header;
            n_begin() = it.node;
            n_rightmost() = it.node;
            root() = it.node;
            return it;
        }

        // Add to a correct place based on key
        iterator par;
        if (pos.node->left == nullptr) { // Includes begin()
            assert(pos != end());
            par = pos;
            assert(Compare()(key, key_of(*par)));
            // Update left node of leaf
            par.node->left = it.node;
        } else { // Includes end()
            assert(pos != begin());
            par = std::prev(pos);
            assert(par.node->right == nullptr);
            if (!Compare()(key_of(*par), key))
                assert(false);
            assert(Compare()(key_of(*par), key));
            // Update right node of leaf
            par.node->right = it.node;
        }
        it.node->par = par.node;
        assert(par != end());

        // Strict inequality so we won't rotate out header
        while (par.node->pri < it.node->pri) {
            assert(par.node != &header); // Rotates will mess up
            if (par.node->left == it.node) {
                rotate_right(par.node, it.node);
            } else {
                assert(par.node->right == it.node);
                rotate_left(par.node, it.node);
            }
            par.node = it.node->par;
        }

        // Update begin() and rightmost()
        assert(!empty());
        if (Compare()(key_of(*it), key_of(*begin()))) {
            n_begin() = it.node; // TODO: Force compiler errors when doing begin() = it.node
        }
        if (Compare()(key_of(*rightmost()), key_of(*it))) {
            n_rightmost() = it.node;
        }

        return it;
    }

    // Merge left and right of pos's node and replace pos's node with the result
    [[nodiscard]] iterator erase_(iterator pos) {
        assert(pos != end());
        iterator next_it = std::next(pos); // Gets iterator to element with next biggest key

        // Update begin() and rightmost()
        // If size_ is going from 1 to 0, next_it is automatically end()
        if (pos == begin()) {
            n_begin() = next_it.node;
        } else if (next_it == end()) {
            n_rightmost() = std::prev(pos).node;
        }

        assign_and_destroy(pos.node, merge(pos.node->left, pos.node->right), pos.node->par);

        return next_it;
    }

    // Replaces dst with src, taking care of dangling pointers
    // Does not delete the node pointed to by dst
    // Pass par if dst may be nullptr
    void assign_and_keep(node *&dst, node *src, node *par) {
        assert(dst != &header);
        assert(dst != par);
        assert(src != par);
        assert(par != nullptr);
        if (par == &header) {
            if (header.par != dst) {
                assert(false);
            }
            assert(header.par == dst);
        } else {
            assert(par->left == dst || par->right == dst);
        }

        // Should not be the root node!
        if (dst == nullptr) {
            dst = src;
            assert(par != nullptr);
            if (dst != nullptr) {
                dst->par = par;
            }
            return;
        }

        // Special root node treatment
        if (par == &header) {
            dst = src;
            if (dst != nullptr) {
                dst->par = &header;
            }
            header.par = dst;
            return;
        }

        // Backup
        const bool is_left_child = par->left == dst;

        // Assign
        dst = src;

        // Fix par's child
        is_left_child ? par->left = dst : par->right = dst;

        // Fix new dst's par
        // May be nullptr if src is nullptr; in that case, only par's child is updated
        if (dst != nullptr) {
            dst->par = par;
        }

        assert(dst == nullptr || dst->par == par);
        assert(par->left == dst || par->right == dst);
    }

    // Replaces dst with src, taking care of dangling pointers
    // Also destroys the node pointed to by dst
    // Requires that dst points to a valid node
    void assign_and_destroy(node *&dst, node *src, node *par) {
        assert(dst != &header);
        assert(dst != nullptr);
        assert(par != nullptr);
        // Backup
        node *const dst_old = dst;
        assign_and_keep(dst, src, par);
        // Prevent transferred children from being destroyed
        dst_old->left = nullptr;
        dst_old->right = nullptr;
        // Destroy old dst
        destroy_node(dst_old);
        // Special empty treatment
        // Have to use this condition as begin() is invalidated
        // in the line ??? // TODO update
        if (size() == 0) {
            header.left = &header;
            header.right = &header;
            header.par = nullptr;
        }
    }

    node_allocator &get_node_allocator() {
        return allocator;
    }

    template<class... Args>
    node *create_node(Args &&...args) {
        node *res = std::allocator_traits<node_allocator>::allocate(get_node_allocator(), 1);
        std::allocator_traits<node_allocator>::construct(get_node_allocator(), res, std::forward<Args>(args)...);
        size_++;
        return res;
    }

    void destroy_node(node *node) {
        size_--;
        if (node->left != nullptr) {
            destroy_node(node->left);
        }
        if (node->right != nullptr) {
            destroy_node(node->right);
        }
        std::allocator_traits<node_allocator>::deallocate(get_node_allocator(), node, 1);
    }

    size_type size_{};
    node header; // This is needed so that different treaps have different end()s
    node_allocator allocator{};

#ifndef INSTRUMENT_DEPTH
    inline static std::minstd_rand generator{}; // NOLINT(cert-msc51-cpp)
#else
#if INSTRUMENT_DEPTH == 1
    inline static std::minstd_rand generator{}; // NOLINT(cert-msc51-cpp)
#else
#if INSTRUMENT_DEPTH == 2
    inline static std::mt19937 generator{}; // NOLINT(cert-msc51-cpp)
#else
#if INSTRUMENT_DEPTH == 3
    inline static std::ranlux24_base generator{}; // NOLINT(cert-msc51-cpp)
#endif // INSTRUMENT_DEPTH == 3
#endif // INSTRUMENT_DEPTH == 2
#endif // INSTRUMENT_DEPTH == 1
#endif // INSTRUMENT_DEPTH
};

}

#define BST_IMPL TREAP

#if BST_IMPL == TREAP
template<
        class Key,
        class Compare   = std::less<Key>,
        class Allocator = std::allocator<Key>
>
using set = impl::treap<Key, impl::null_type, Compare, Allocator>;

template<
        class Key,
        class T,
        class Compare   = std::less<Key>,
        class Allocator = std::allocator<std::pair<const Key, T>>
>
using map = impl::treap<Key, T, Compare, Allocator>;
#endif

}

// TODO: Splay trees, AVL trees, scapegoat trees
//  More: Red-black trees, B*-trees

#endif //BST_BST_H
