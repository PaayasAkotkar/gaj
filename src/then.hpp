#pragma once
#include "pipe.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
namespace gaj_then {
template <class Fn> struct then {
  struct impl {
    Fn fn;
    gaj_pipe::pipe<std::function<void()>> ch;
    std::jthread worker;

    impl(Fn fn_) : fn(std::move(fn_)) { this->start(); }

    ~impl() {
      ch.close();
      if (worker.joinable()) {
        worker.join();
      }
    }

    void push(auto &&value) {
      if constexpr (std::is_invocable_v<Fn &, decltype(value)>) {
        ch.send(std::function<void()>(
            [this, value = std::forward<decltype(value)>(value)]() mutable {
              std::invoke(fn, std::move(value));
            }));
      } else if constexpr (std::is_invocable_v<Fn &>) {
        ch.send(std::function<void()>([this] { std::invoke(fn); }));
      }
    }

  private:
    void start() {
      worker = std::jthread([this] { this->hub(); });
    }

    void hub() {
      while (auto job = ch.recv()) {
        try {
          (*job)();
        } catch (const std::exception &error) {
          std::cerr << error.what() << '\n';
        }
      }
    }
  };

public:
  explicit then(Fn fn) : impl_(std::make_shared<impl>(std::move(fn))) {}

  void operator()(auto &&value) {
    impl_->push(std::forward<decltype(value)>(value));
  }

private:
  std::shared_ptr<impl> impl_;
};
}; // namespace gaj_then