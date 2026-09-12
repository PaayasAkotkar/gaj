#pragma once
#include "pipe.hpp"
#include "then.hpp"
#include "types.hpp"
#include <atomic>
#include <barrier>
#include <concepts>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <latch>
#include <memory>
#include <mutex>
#include <tuple>
#include <type_traits>

// select behaves as an async logic which depends upons ace
// note: inspiration is taken from the golang hub-architecture select{}
// select(
// ace(channel,then),
// ace(channel,then),
// ace(channel,then),
// )
// you can use the qace for your own use which is
// detach version of ace but its for your own use
// for contributes:
namespace gaj_select {

// frame only for tracing
struct frame {
  void signal(uint64_t id) { cur.send(id); }
  uint64_t wait_next() {
    auto v = cur.recv();
    return v ? *v : 0;
  }
  void close() { cur.close(); }
  gaj_pipe::pipe<uint64_t> cur;
  frame() = default;
}; // end

// note: you explicitly have to handle the pipe removal
// this is case sensitive
template <typename Ch, typename Process> struct qace {

  explicit qace(std::shared_ptr<gaj_pipe::pipe<Ch>> _ch, gaj_then::then<Process> _proc)
      : ch(std::move(_ch)), proc(std::move(_proc)) {
    this->start();
  }

  ~qace() {}

  void release_ace() {
    ch->clear();
    if (this->worker.joinable()) {
      this->worker.join();
    }
  }

private:
  void start() {
    worker = std::jthread([this]() { hub(); });
  }

  void hub() {
    while (auto v = ch->recv()) {
      try {
        std::invoke(proc, *v);
      } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
      }
    }
  }

private:
  std::shared_ptr<gaj_pipe::pipe<Ch>> ch;
  gaj_then::then<Process> proc;
  std::jthread worker;
}; // end

template <typename Ch, typename Process> struct _ace {

  explicit _ace(std::shared_ptr<gaj_pipe::pipe<Ch>> _ch, gaj_then::then<Process> _proc,
                std::shared_ptr<frame> _f)
      : ch(std::move(_ch)), proc(std::move(_proc)), f(std::move(_f)) {
    this->start();
  }

  ~_ace() {}

  void release_ace() {
    ch->clear();
    if (this->worker.joinable()) {
      this->worker.join();
    }
  }

private:
  void start() {
    worker = std::jthread([this]() { hub(); });
  }

  void hub() {
    while (auto v = ch->recv()) {
      try {
        std::invoke(proc, *v);
        f->signal(create_trace_id());
      } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
      }
    }
  }

private:
  std::shared_ptr<gaj_pipe::pipe<Ch>> ch;
 gaj_then:: then<Process> proc;
  std::shared_ptr<frame> f;
  std::jthread worker;
}; // end

template <class ch, class pro> struct ace_spec {
  std::shared_ptr<gaj_pipe::pipe<ch>> _ch;
  gaj_then::then<pro> process;
  auto build(std::shared_ptr<frame> f) && {
    return std::make_unique<_ace<ch, pro>>(std::move(_ch), std::move(process),
                                           std::move(f));
  }
}; // end

template <class ch, class process>
auto ace(std::shared_ptr<gaj_pipe::pipe<ch>> _ch, gaj_then::then<process> _proc) {
  return ace_spec<ch, process>{std::move(_ch), std::move(_proc)};
} // end

template <class... Cases> struct select {
  explicit select(Cases... cases)
      : _impl(std::make_shared<impl>(std::move(cases)...)) {}

  void release() { _impl->release(); }
  uint64_t wait_next() { return this->_impl->f->wait_next(); }

private:
  struct impl {
    explicit impl(Cases... cases)
        : f(std::make_shared<frame>()), shelter(std::move(cases).build(f)...) {}

    void release() {
      f->close();
      std::apply([](auto &...a) { (a->release_ace(), ...); }, shelter);
    }

    std::shared_ptr<frame> f;
    std::tuple<decltype(std::declval<Cases>().build(
        std::declval<std::shared_ptr<frame>>()))...>
        shelter;
  };

  std::shared_ptr<impl> _impl;
}; // end

}; // namespace gaj_select
