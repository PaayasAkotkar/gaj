#include "async.hpp"
#include <chrono>
#include <thread>
using namespace std; // bad practice;

// x1.cpp-> note: it is recommeded by me to you that make changes in the example
// for better understnding

void pipe_example() {

  pipe<int> p;
  auto id = p.send(12);
  cout << "id: " << id << endl;
  id = p.send(14);

  try {
    id = p.send(124);
  } catch (const std::runtime_error &error) {
    cerr << "caught: " << error.what() << endl;
  }
  cout << "id: " << id << endl;
  auto val = p.recv();
  if (val) {
    cout << "found: " << val.value() << endl;
  }
  val = p.recv();
  if (val) {
    cout << "found: " << val.value() << endl;
  }
  p.clear();

  auto s = p.size();
  cout << "size: " << s << endl;
}

void when_example() {
  auto task = when([]() { cout << "yo" << endl; }, "not succeeded :");
  task.signal(); // more time you fire the same method triggers
}

void when_all_example() {
  auto task = when_all(
      "unsucceed 😲", []() { cout << "crazy" << endl; },
      []() { cout << "play" << endl; });
  task.signal();
  this_thread::sleep_for(chrono::seconds(2));
  task.signal(); // play final method
}

