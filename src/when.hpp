#pragma once
#include "types.hpp"
#include "pipe.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>


template <typename Fn> class when {
public:
  explicit when(Fn fn, std::string panic = "")
      : fn(std::move(fn)), panic(std::move(panic)), worker([this] { hub(); }) {}

  ~when() {
    ch.close();
    if (worker.joinable()) {
      worker.join();
    }
  }

  void signal(int value = 1) { ch.send(value); }

protected:
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

  pipe<int> ch;
  Fn fn;
  std::string panic;
  std::jthread worker;
};
