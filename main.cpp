#include "example_v1.hpp"
#include "pipe.hpp"
#include "queue"
#include "src/record.hpp"
#include "src/select.hpp"
#include "when.hpp"
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <type_traits>


using namespace std;
void end() { std::cout << "end" << std::endl; }

int main() {
  gaj_example::http_example(); // required server/main.go or any server you would like to run and connect
}
