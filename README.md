## treap_stl
This educational project implements a
[treap](https://en.wikipedia.org/wiki/Treap), a data structure that combines the
properties of a binary search tree and a heap, to provide an efficient and
probabistically balanced binary search tree. It is designed to be a drop-in
replacement for the C++ Standard Library's `std::set` and `std::map`. Currently,
most of the commonly-used operations such as `find`, `insert`, `erase`,
`lower_bound`, and `upper_bound` have been implemented.

### Learning points
- Adopted
[test-driven development](https://en.wikipedia.org/wiki/Test-driven_development),
which sped up feature development and minimized bugs in existing code
- Template metaprogramming in C++, including
[SFINAE](https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error) to
re-use code between the `bst::set` and `bst::map` classes
- C++ memory allocators, including proper allocation,
deallocation, construction, and destruction of tree node pointers
- Custom C++ iterators

### Further extensions
- Augment the search tree with tree order statistics to enable the following
extra operations
([Order-statistic tree](https://en.wikipedia.org/wiki/Order_statistic_tree))
  - `select(i)`: Find the i-th smallest element in the tree
  - `rank(x)`: Find the rank of element x in the tree, i.e. its index in the
sorted list of elements of the tree
- Allowing multiple keys (implementing the interface of `std::multiset` and
`std::multimap`)
- Implementing other binary search trees, such as splay trees, AVL trees, and
scapegoat trees, and comparing the time and memory usage of each

### License
Copyright © 2023 Bill Chow. All rights reserved.

This repository is made available for evaluation purposes by authorized individuals only.
Unauthorized use, reproduction, modification, or distribution of this code is strictly prohibited.