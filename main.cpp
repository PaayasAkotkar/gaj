#include "queue"
#include "async.hpp"
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
  ctx ctx;
  auto [_ctx,cancel] = ctx.with_cancel();
  _ctx->close();
  _ctx->done();
}
