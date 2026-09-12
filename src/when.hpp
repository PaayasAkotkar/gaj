#pragma once
#include "pipe.hpp"
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gaj_when {

// when_all -> if you push the total-number as per your methods
// it typically runs the func one-by-one
template <typename... Fn> class when_all {
  struct impl {
    gaj_pipe::pipe<int> ch;
    std::tuple<Fn...> funcs;
    std::string panic;
    std::jthread worker;

    impl(std::string panic_, Fn... fn)
        : funcs(std::move(fn)...), panic(std::move(panic_)) {
      this->start();
    }

    void start() {
      worker = std::jthread([this] { hub(); });
    }
    ~impl() {
      if (worker.joinable()) {
        worker.join();
      }
    }
    void release() { this->ch.close(); }

    template <std::size_t Index = 0>
    void invoke_at(std::size_t target, int value) {
      if constexpr (Index < sizeof...(Fn)) {
        if (Index == target) {
          auto &fn = std::get<Index>(funcs);
          if constexpr (std::is_invocable_v<decltype(fn) &, int>) {
            std::invoke(fn, value);
          } else {
            std::invoke(fn);
          }
        } else {
          invoke_at<Index + 1>(target, value);
        }
      }
    }

    void hub() {
      std::size_t next = 0;
      while (auto value = ch.recv()) {
        if (next < sizeof...(Fn)) {
          try {
            invoke_at(next, *value);
          } catch (const std::exception &error) {
            std::cerr << error.what() << '\n';
          } catch (...) {
            std::cerr << (panic.empty() ? "callback failed" : panic) << '\n';
          }
          ++next;
        }
      }
    }
  };

public:
  explicit when_all(std::string panic, Fn... fn)
      : impl_(std::make_shared<impl>(std::move(panic), std::move(fn)...)) {}

  void operator()(int value = 1) { impl_->ch.send(value); }
  void release() { this->impl_->release(); }

private:
  std::shared_ptr<impl> impl_;
};

// when single manual concurrent call to implement method 
template <typename Fn> class when {
  struct impl {
    gaj_pipe::pipe<int> ch;
    Fn fn;
    std::string panic;
    std::jthread worker;

    impl(Fn fn_, std::string panic_)
        : fn(std::move(fn_)), panic(std::move(panic_)),
          worker([this] { hub(); }) {}

    ~impl() {
      if (worker.joinable()) {
        worker.join();
      }
    }
    void release() { ch.close(); }

    void invoke(int value) {
      if constexpr (std::is_invocable_v<Fn &, int>) {
        std::invoke(fn, value);
      } else {
        std::invoke(fn);
      }
    }

    void hub() {
      while (auto value = ch.recv()) {
        try {
          invoke(*value);
        } catch (const std::exception &error) {
          std::cerr << error.what() << '\n';
        } catch (...) {
          std::cerr << (panic.empty() ? "when callback failed" : panic) << '\n';
        }
      }
    }
  };

public:
  explicit when(Fn fn, std::string panic = "")
      : impl_(std::make_shared<impl>(std::move(fn), std::move(panic))) {}
  void release() { this->impl_->release(); }

  void operator()(int value) { impl_->ch.send(value); }

private:
  std::shared_ptr<impl> impl_;
};
}; // namespace gaj_when
