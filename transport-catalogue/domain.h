#pragma once

#include <string_view>

namespace database {
namespace detail {

struct StringViewHash {
  using is_transparent = void;

  size_t operator()(std::string_view sv) const noexcept {
    return std::hash<std::string_view>{}(sv);
  }
};

struct StringViewEqual {
  using is_transparent = void;

  bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
    return lhs == rhs;
  }
};

struct PairPtrHash {
  using is_transparent = void;

  template <typename T>
  size_t operator()(std::pair<const T *, const T *> ptrs_pair) const noexcept {
    auto h1 = std::hash<const void *>{}((ptrs_pair.first));
    auto h2 = std::hash<const void *>{}((ptrs_pair.second));

    return h1 ^ (h2 + 0x9e3779b97f4a7c15ull + (h1 << 6) + (h1 >> 2));
  }
};

} // namespace detail
} // namespace database
