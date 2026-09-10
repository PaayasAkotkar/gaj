#include "queue"
#include "x1.cpp"
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>

using namespace std;
void end() { std::cout << "end" << std::endl; }
int main() {
  pipe_example();
}
