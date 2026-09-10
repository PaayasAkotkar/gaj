#pragma  once
#include "types.hpp"
#include "pipe.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
// when_all strict binding it is based on the total-number-of-funs applied
template <typename... Fn> class when_all {
public:
  explicit when_all(std::string panic="",Fn... fn)
      : funcs(std::move(fn)...), panic(std::move(panic)),worker([this] { hub(); }) {}

  ~when_all() {
    ch.close();
    if (worker.joinable()) {
      worker.join();
    }
  }

  void signal(int value = 1) { ch.send(value); }

protected:
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
        invoke_at(next++, *value);
      }
    }
  }

  pipe<int> ch;
  std::tuple<Fn...> funcs;
  std::string panic;
  std::jthread worker;
};

