// #include "iostream"
// #include <memory>
//
// study~~~part
// enum eorder { last, first };
// struct node {
//   node *next;
//   node *prev;
//   int val;
//   node(int val) : next(nullptr), prev(nullptr), val(val) {}
// };
//
// struct fuck {
//   node *head, *tail = nullptr;
//
//   void addItBoy(int val) {
//     node *bruh = new node(val);
//     if (!head && !tail) {
//       head = tail = bruh;
//     } else {
//       tail->next = bruh;
//       bruh->prev = nullptr;
//       tail = bruh;
//     }
//   }
//
//   void ino(node *n, eorder o) {
//     if (!n)
//       return;
//     switch (o) {
//     case last:
//       ino(n->prev, o);
//       std::cout << "visited: " << n->elem << std::endl;
//       ino(n->next, o);
//       break;
//     case first:
//       ino(n->next, o);
//       std::cout << "visited: " << n->elem << std::endl;
//       ino(n->prev, o);
//       break;
//     }
//   }
//
//   void preo(node *n, eorder o) {
//     if (!n)
//       return;
//     std::cout << "visited: " << n->elem << std::endl;
//     switch (o) {
//     case last:
//       preo(n->prev, o);
//       preo(n->next, o);
//       break;
//     case first:
//       preo(n->next, o);
//       preo(n->prev, o);
//       break;
//     }
//   }
//
//   int whatValue() {
//     auto c = head;
//     do {
//       c->next = nullptr;
//       ;
//     } while (!c);
//     return c->elem;
//   }
// };
//
// template <typename T, typename alloc = std::allocator<T>> class nm {
//   using node_alloc =
//       typename std::allocator_traits<alloc>::template rebind_alloc<nm>;
//   using traits = std::allocator_traits<node_alloc>;
//   using pointer = traits::pointer;
//
// public:
//   pointer left;
//   pointer right;
//   T val;
//   nm(const T &val) : val(val) {}
//
// };
//
// struct te {
//   using Node = nm<int>;
//   using NodeAlloc =
//       std::allocator_traits<std::allocator<int>>::rebind_alloc<Node>;
//   using Traits = std::allocator_traits<NodeAlloc>;
//   using NodePtr = Traits::pointer;
//   NodeAlloc aloc;
//   NodePtr root = nullptr;
//   void add(int val) {
//     NodePtr r = Traits::allocate(aloc, 1);
//     Traits::construct(aloc, std::to_address(r), val);
//
//     if (!root) {
//       root = r;
//     } else {
//       root->left = r;
//     }
//   }
//   void pop() {}
//   int get() {
//     NodePtr m = root;
//     while (!m) {
//       m = m->left;
//     }
//     return m->elem;
//   }
//
// private:
//   auto &operator*() const { return root->elem; }
//   te &operator++() {
//     if (root)
//       root = root->left;
//     return *this;
//   }
//
// };
#pragma  once
#include "pipe.hpp"
#include"when.hpp"
#include"when_all.hpp"
#include"schedule.hpp"



// still there are bugs with when & when_all for corutine race