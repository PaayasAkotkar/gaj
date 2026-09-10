#pragma  once
#include <string>
using error_t = std::string;
inline const error_t nill = "";

template <typename... P> size_t total_param(P &&...ps) { return sizeof...(ps); }
