#pragma once
#include "pipe.hpp"
#include "types.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

// incomplete
template <class K, class V, class Alloc = std::allocator<V>> struct record {

  struct sdp {
    using alloc =
        typename std::allocator_traits<Alloc>::template rebind_alloc<sdp>;
    using traits = std::allocator_traits<alloc>;
    using pointer = typename traits::pointer;

    using valloc =
        typename std::allocator_traits<Alloc>::template rebind_alloc<V>;
    using vtraits = std::allocator_traits<valloc>;
    using vpointer = typename vtraits::pointer;

    pointer left = nullptr;
    pointer right = nullptr;
    vpointer elem = nullptr;
    K k;
    uint64_t trace_id{0};

    sdp() = default;
    ~sdp() {
      if (elem) {
        valloc va;
        vtraits::destroy(va, std::to_address(elem));
        vtraits::deallocate(va, elem, 1);
      }
    }
    template <typename KeyArg, typename ValArg>
    sdp(KeyArg &&key, ValArg &&val) : k(std::forward<KeyArg>(key)), elem(val) {}
    sdp(K key, V val) : k(std::move(key)), elem(std::move(val)) {}
  };

  using node = sdp;
  using alloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<node>;
  using traits = std::allocator_traits<alloc>;
  using pointer = typename traits::pointer;
  using trace = gaj_pipe::pipe<uint64_t>;

  alloc al;
  pointer root{nullptr};
  size_t len{0};
  trace pos;
  std::mutex mu;

  record() = default;

  inline void entry(K k, V v) {
    pointer p = traits::allocate(al, 1);

    try {
      traits::construct(al, std::to_address(p), std::move(k), std::move(v));
    } catch (...) {
      traits::deallocate(al, p, 1);
      throw;
    }

    uint64_t handle = reinterpret_cast<uint64_t>(std::to_address(p));
    p->trace_id = handle;

    {
      std::lock_guard<std::mutex> lock(this->mu);
      if (!root) {
        root = p;
      } else {
        iadd(p);
      }
      ++len;
    }

    this->pos.send(handle);
  }

  inline std::optional<V> recv() {
    if (auto handle = pos.recv()) {
      if (*handle == 0)
        return std::nullopt;

      pointer p = reinterpret_cast<pointer>(*handle);
      return p->elem;
    }
    return std::nullopt;
  }

  inline std::optional<V> recv_with(const K &k) {
    std::lock_guard<std::mutex> lock(this->mu);
    pointer c = this->root;

    while (c != nullptr) {
      if (k == c->k) {
        return c->elem;
      }
      if (k < c->k) {
        c = c->left;
      } else {
        c = c->right;
      }
    }
    return std::nullopt;
  }

  inline void destroy_node(pointer n) {
    traits::destroy(al, std::to_address(n));
    traits::deallocate(al, n, 1);
    --len;
  }

  inline void rem(const K &k) { this->root = this->irem(this->root, k); }

private:
  std::recursive_mutex rm;

  inline pointer _xor(pointer a, pointer b) {
    return reinterpret_cast<pointer>(
        reinterpret_cast<uintptr_t>(std::to_address(a)) ^
        reinterpret_cast<uintptr_t>(std::to_address(b)));
  }

  inline pointer irem(pointer r, const K &k) {
    std::lock_guard<std::recursive_mutex> loc(rm);
    if (!r)
      return nullptr;

    if (r->left->k > k) {
      irem(r->left, k);
    } else if (r->right->k < k) {
      irem(r->right.k);
    } else {
      if (!r->left) {
        auto t = r->right;
        destroy_node(t);
        return t;
      } else if (!r->right) {
        auto t = r->left;
        destroy_node(t);
        return t;
      }
      pointer s = r->right;
      while (!s) {
        s = s->left;
      }
      r->k = s->k;
      r->elem = std::move(s->elem);
      r->trace_id = std::move(s->trace_id);
      r->right = irem(r->right, s->k);
    }
  }

  inline void iadd(pointer p) {
    pointer cur = root;
    pointer pa = nullptr;

    while (cur != nullptr) {
      pa = cur;
      if (p->k < cur->k) {
        cur = cur->left;
      } else if (p->k > cur->k) {
        cur = cur->right;
      } else {
        cur->elem = std::move(p->elem);
        destroy_node(p);
        return;
      }
    }

    if (p->k < pa->k) {
      pa->left = p;
    } else {
      pa->right = p;
    }
  }
};