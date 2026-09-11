#pragma once
#include <atomic>
#include <cassert>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <future>
#include <iostream>
#include <iterator>
#include <latch>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>

template <typename T, typename Alloc = std::allocator<T>> struct pipe {
protected:
  template <typename _T> struct sd {

    using allocate = std::allocator_traits<Alloc>::template rebind_alloc<sd>;
    using traits = std::allocator_traits<allocate>;
    using pointer = typename traits::pointer;
    using value_allocate =
        typename std::allocator_traits<Alloc>::template rebind_alloc<_T>;
    using value_traits = std::allocator_traits<value_allocate>;
    using value_pointer = typename value_traits::pointer;
    pointer left = nullptr;
    pointer right = nullptr;
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
    ~sd() {
      if (elem) {
        value_traits::destroy(value_alloc, std::to_address(elem));
        value_traits::deallocate(value_alloc, elem, 1);
      }
    }
  };

  inline std::uint64_t create_trace_id() noexcept {
    const auto now =
        std::chrono::steady_clock::now().time_since_epoch().count();
    
    return 10'000'000ULL + (static_cast<std::uint64_t>(now) % 90'000'000ULL);
  }

  enum class epipe_status { open, close };
  using node = sd<T>;
  using node_allocate =
      std::allocator_traits<Alloc>::template rebind_alloc<node>;
  using traits = std::allocator_traits<node_allocate>;
  using node_ptr = traits::pointer;
  node_allocate alloc;

  node_ptr root = nullptr;
  node_ptr tail = nullptr;
  size_t s = 0;
  std::atomic<epipe_status> status{epipe_status::open};
  std::condition_variable cv;
  std::mutex mu;

  inline void destroy_node(node_ptr node) {
    traits::destroy(alloc, std::to_address(node));
    traits::deallocate(alloc, node, 1);
    --s;
  }

public:
  pipe() = default;
  ~pipe() { clear(); }

  inline std::uint64_t send(const T &val) {

    if (this->status.load(std::memory_order_acquire) == epipe_status::close) {
      throw std::runtime_error("channel closed");
    }
    std::lock_guard<std::mutex> loc(this->mu);

    node_ptr p = traits::allocate(alloc, 1);
    traits::construct(alloc, std::to_address(p), std::move(val));

    auto id = this->create_trace_id();
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

  inline std::optional<T> recv_for(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> loc(this->mu);
    if (!cv.wait_for(loc, timeout, [this]() {
          return this->root != nullptr ||
                 this->status.load(std::memory_order_acquire) ==
                     epipe_status::close;
        })) {
      std::cerr << "recv timeout: channel is empty, size=" << s << '\n';
      return std::nullopt;
    }
    return receive_locked();
  }

protected:
  inline std::optional<T> receive_locked() {
    node_ptr p = nullptr;
    node_ptr c = root;

    while (c != tail) {
      p = c;
      c = c->left;
    }

    if (p) {
      p->left = nullptr;
      tail = p;
    } else {
      root = nullptr;
      tail = nullptr;
    }

    auto value = std::move(*c->elem);
    destroy_node(c); // remove current value
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
    this->tail = nullptr;
  }

  // close the sender is block forever
  inline void close() {
    this->status.store(epipe_status::close, std::memory_order_release);
    cv.notify_all();
  }
};
