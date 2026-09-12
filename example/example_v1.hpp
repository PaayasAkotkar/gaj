#pragma once
#include "HTTPRequest.hpp" // shot-out to https://github.com/elnormous/HTTPRequest
#include "async.hpp"
#include <chrono>
#include <iostream>

namespace gaj_example {

inline void pipe_example() {
  std::cout << "pipe-example" << "\n";
  gaj_pipe::pipe<int> p;
  auto id = p.send(12);
  std::cout << "id: " << id << "\n";
  id = p.send(14);

  try {
    id = p.send(124);
  } catch (const std::runtime_error &error) {
    std::cerr << "caught: " << error.what() << "\n";
  }
  std::cout << "id: " << id << "\n";
  auto val = p.recv();
  if (val) {
    std::cout << "found: " << val.value() << "\n";
  }
  val = p.recv();
  if (val) {
    std::cout << "found: " << val.value() << "\n";
  }
  p.clear();

  auto s = p.size();
  std::cout << "size: " << s << "\n";
}

inline void when_example() {
  std::cout << "when-example" << "\n";

  auto task =
      gaj_when::when([]() { std::cout << "yo" << "\n"; }, "not succeeded :");
  // note: the value doesnt matter
  task(12); // more time you fire the same method triggers
  task(1);
  std::cout << "end" << "\n";
  task.release();
}

inline void when_all_example() {
  std::cout << "when-all-example" << "\n";

  auto task = gaj_when::when_all(
      "unsucceed 😲", []() { std::cout << "crazy" << "\n"; },
      []() { std::cout << "play" << "\n"; });
  task(1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  task(2); // play final method
  task.release();
}

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

// inorder for this to work run the golang setup-server
inline void http_example() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8); // shotout to gemini
#endif
  std::cout << "http-example" << std::endl;
  const std::string url = "http://127.0.0.1:1080/get/";

  auto ch1 = std::make_shared<gaj_pipe::pipe<std::string>>();
  gaj_select::select p(gaj_select::ace(ch1, gaj_then::then([=](auto v) {
                                         std::cout << "received: " << v
                                                   << std::endl;
                                       })));

  try {
    http::Request req{url};
    const auto res = req.send("GET");

    if (res.status.code == http::Status::Ok) {
      std::string body(res.body.begin(), res.body.end());
      ch1->send(body);
    } else {
      std::cout << "http Error: " << res.status.code << "\n";
    }
  } catch (const std::exception &e) {
    std::cout << "exception: " << e.what() << "\n";
  }
}

} // namespace gaj_example