#include <cstddef>

template <std::size_t N> struct constexprString {
  char data[N + 1]{};

  constexpr constexprString() = default;

  constexpr constexprString(const char (&str)[N + 1]) {
    for (std::size_t i = 0; i <= N; ++i) data[i] = str[i];
  }
  constexpr char* c_str() { return data; }

  constexpr char operator[](std::size_t i) const { return data[i]; }

  constexpr std::size_t size() const { return N; }

  template <size_t N2> constexpr constexprString<N2> substr(std::size_t pos) const {
    constexprString<N2> result{};
    for (std::size_t i = 0; i < N2; ++i) {
      result.data[i] = data[pos + i];
    }
    result.data[N2] = '\0';
    return result;
  }

  template <size_t N2> constexpr std::size_t find(constexprString<N2> needle) const {
    for (size_t i = 0; i + N2 <= N; ++i) {
      bool match = true;
      for (size_t j = 0; j < N2 - 1; ++j) {
        if (data[i + j] != needle.data[j]) {
          match = false;
          break;
        }
      }
      if (match) return i;
    }
    return static_cast<size_t>(-1);
  }

  constexpr const char* c_str() const { return data; }
};