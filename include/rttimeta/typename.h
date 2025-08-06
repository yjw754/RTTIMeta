#pragma once
#include "constexprString.h"

template <typename T> constexpr auto WrappedTypeName() {
#if defined(_MSC_VER)
  return constexprString<sizeof(__FUNCSIG__) - 1>(__FUNCSIG__);
#elif defined(__clang__) || defined(__GNUC__)
  return constexprString<sizeof(__PRETTY_FUNCTION__) - 1>(__PRETTY_FUNCTION__);
#else
#  error Unsupported compiler
#endif
}

// 提取T的名称
template <typename T> constexpr auto TypeName() {
  constexpr auto wrapped = WrappedTypeName<T>();
  constexpr auto ref = WrappedTypeName<void>();

  constexpr auto prefix_len = ref.find(constexprString<4>{"void"});
  constexpr auto suffix_len = ref.size() - prefix_len - 4;

  constexpr auto result_size = wrapped.size() - prefix_len - suffix_len;
  return wrapped.template substr<result_size>(prefix_len);
}