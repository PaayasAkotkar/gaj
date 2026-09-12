#pragma once
#include <string>
#include <chrono>

using error_t = std::string;
inline const error_t nil = "";

template <typename... P> size_t total_param(P &&...ps) { return sizeof...(ps); }

inline std::uint64_t create_trace_id() noexcept {
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return 10'000'000ULL + (static_cast<std::uint64_t>(now) % 90'000'000ULL);
}
