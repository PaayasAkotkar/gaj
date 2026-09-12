#pragma once
#include "types.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>


// gaj_pipe implements the concurrent commnucation
namespace gaj_pipe {

template <typename T, typename Alloc = std::allocator<T>> struct pipe {
protected:
  template <typename _T> struct sd {
    // type
    using allocate = std::allocator_traits<Alloc>::template rebind_alloc<sd>;
    using traits = std::allocator_traits<allocate>;
    using pointer = typename traits::pointer;
    // end
    // elem
    using value_allocate =
        typename std::allocator_traits<Alloc>::template rebind_alloc<_T>;
    using value_traits = std::allocator_traits<value_allocate>;
    using value_pointer = typename value_traits::pointer;
    // end
    pointer left = nullptr;
    value_allocate value_alloc;
    value_pointer elem = nullptr;
    uint64_t trace_id;

    sd() = default;

    explicit sd(const _T &value) : elem(value_alloc.allocate(1)) {
      try {
        value_traits::construct(value_alloc, std::to_address(elem), value);
      } catch (...) {
        value_traits::deallocate(value_alloc, elem, 1);
        elem = nullptr;
        throw;
      }
    }

    explicit sd(_T &&value) : elem(value_alloc.allocate(1)) {
      try {
        value_traits::construct(value_alloc, std::to_address(elem),
                                std::move(value));
      } catch (...) {
        value_traits::deallocate(value_alloc, elem, 1);
        elem = nullptr;
        throw;
      }
    }

    ~sd() {
      if (elem) {
        value_traits::destroy(value_alloc, std::to_address(elem));
        value_traits::deallocate(value_alloc, elem, 1);
      }
    }
  };

  enum class epipe_status { open, close };
  // type
  using node = sd<T>;
  using node_allocate =
      std::allocator_traits<Alloc>::template rebind_alloc<node>;
  using traits = std::allocator_traits<node_allocate>;
  using node_ptr = traits::pointer;
  node_allocate alloc;
  // end

  node_ptr root = nullptr; // oldest
  node_ptr tail = nullptr; // newest
  size_t s = 0;
  std::atomic<epipe_status> status{epipe_status::open};
  std::condition_variable cv;
  std::mutex mu;

  inline void destroy_node(node_ptr node) {
    traits::destroy(alloc, std::to_address(node));
    traits::deallocate(alloc, node, 1);
    --s;
  }

  template <typename U> inline std::uint64_t send_impl(U &&val) {
    if (this->status.load(std::memory_order_acquire) == epipe_status::close) {
      throw std::runtime_error("channel closed");
    }
    std::lock_guard<std::mutex> loc(this->mu);

    node_ptr p = traits::allocate(alloc, 1);
    traits::construct(alloc, std::to_address(p), std::forward<U>(val));

    auto id = create_trace_id();
    p->trace_id = id;

    if (!root) {
      root = tail = p;
    } else {
      tail->left = p;
      tail = p;
    }
    ++s;
    cv.notify_one();
    return id;
  }

public:
  pipe() = default;
  pipe(const pipe &) = delete;
  pipe &operator=(const pipe &) = delete;
  pipe(pipe &&) noexcept {};
  pipe &operator=(pipe &&) noexcept = default;

  pipe &operator++() { this->root = root->left; }

  ~pipe() { clear(); }
  inline std::uint64_t send(const T &val) { return send_impl(val); }
  inline std::uint64_t send(T &&val) { return send_impl(std::move(val)); }

  inline std::optional<T> recv() {
    std::unique_lock<std::mutex> loc(this->mu);
    cv.wait(loc, [this]() {
      return this->root != nullptr ||
             this->status.load(std::memory_order_acquire) ==
                 epipe_status::close;
    });
    if (!this->root) {
      return std::nullopt;
    }
    return receive_locked();
  }

protected:
  inline std::optional<T> receive_locked() {
    node_ptr c = root;
    root = c->left;
    if (!root) {
      tail = nullptr;
    }
    auto value = std::move(*c->elem);
    destroy_node(c);
    return value;
  }

public:
  inline size_t size() { return s; }

  inline std::optional<T> find(std::uint64_t trace_id) {
    std::lock_guard<std::mutex> loc(this->mu);
    for (auto c = this->root; c; c = c->left) {
      if (c->trace_id == trace_id) {
        return *c->elem;
      }
    }
    return std::nullopt;
  }

  inline bool release(std::uint64_t trace_id) {
    std::lock_guard<std::mutex> loc(this->mu);
    node_ptr p = nullptr;
    node_ptr c = this->root;

    while (c && c->trace_id != trace_id) {
      p = c;
      c = c->left;
    }
    if (!c) {
      return false;
    }
    if (p) {
      p->left = c->left;
    } else {
      this->root = c->left;
    }
    if (this->tail == c) {
      this->tail = p;
    }
    destroy_node(c);
    return true;
  }

  inline void clear() {
    if(!root)return;
    std::lock_guard<std::mutex> loc(this->mu);
    auto c = root;
    root = nullptr;
    tail = nullptr;
    while (c) {
      auto n = c->left;
      c->left = nullptr;
      destroy_node(c);
      c = n;
    }
  }

  inline void close() {
    this->status.store(epipe_status::close, std::memory_order_release);
    cv.notify_all();
  }

  inline bool is_closed() {
    return status.load(std::memory_order_acquire) == epipe_status::close;
  }
};

} // namespace gaj_pipe