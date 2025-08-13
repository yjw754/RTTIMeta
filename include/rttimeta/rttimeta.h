#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include "typename.h"

using rttiId = uint32_t;

constexpr rttiId fnv1a_hash(const char* str) {
  rttiId hash = 2166136261u;
  while (*str) {
    hash ^= (uint8_t)*str++;
    hash *= 16777619u;
  }
  return hash;
};

template <typename DERIVED_CLASS, typename... BASE_CLASSES> class rttiMeta final {
public:
  constexpr static const char* type_name() noexcept { return TypeName<DERIVED_CLASS>().c_str(); }

  constexpr static rttiId type_id() noexcept {
    return fnv1a_hash(TypeName<DERIVED_CLASS>().c_str());
  }

private:
  rttiMeta(const rttiMeta&) = delete;
  rttiMeta& operator=(const rttiMeta&) = delete;

public:
  template <typename T> static const void* dynamicCast(rttiId dst_id, const T* real_ptr) noexcept {
    if (dst_id == type_id()) {
      return static_cast<const DERIVED_CLASS*>(real_ptr);
    }
    return dynamicCastFromParents<T, BASE_CLASSES...>(dst_id, real_ptr);
  }

  static bool is(rttiId dst_id) noexcept {
    if (dst_id == type_id()) {
      return true;
    }
    return IsFromParents<BASE_CLASSES...>(dst_id);
  }

private:
  template <typename T> static const void* dynamicCastFromParents(rttiId, const T*) noexcept {
    return nullptr;
  }

  template <typename T, typename PARENT, typename... REST>
  static const void* dynamicCastFromParents(rttiId dst_id, const T* real_ptr) noexcept {
    if (auto* result = PARENT::rttiMetaTy::dynamicCast(dst_id, real_ptr)) return result;
    return dynamicCastFromParents<T, REST...>(dst_id, real_ptr);
  }

  // Base case: no parents left
  template <typename... REST>
  static typename std::enable_if<sizeof...(REST) == 0, bool>::type IsFromParents(rttiId) noexcept {
    return false;
  }

  // Recursive case: check each parent
  template <typename PARENT, typename... REST> static bool IsFromParents(rttiId dst_id) noexcept {
    if (PARENT::rttiMetaTy::is(dst_id)) return true;
    return IsFromParents<REST...>(dst_id);
  }
};

#define RTTI_META_DEFINE(DERIVED_CLASS, ...)                                       \
public:                                                                            \
  static const char* rtti_name() noexcept { return rttiMetaTy::type_name(); }      \
                                                                                   \
  static rttiId rtti_id() noexcept { return rttiMetaTy::type_id(); }               \
                                                                                   \
  virtual const void* rtti_cast(rttiId dst_id) const noexcept {                    \
    return rttiMetaTy::dynamicCast(dst_id, this);                                  \
  }                                                                                \
                                                                                   \
  virtual bool is(rttiId dst_id) const noexcept { return rttiMetaTy::is(dst_id); } \
                                                                                   \
public:                                                                            \
  using rttiMetaTy = rttiMeta<DERIVED_CLASS, ##__VA_ARGS__>

template <typename DST, typename SRC>
typename std::enable_if<std::is_pointer<DST>::value && std::is_pointer<SRC>::value, DST>::type
rtti_cast(SRC srcPtr) noexcept {
  using DST_TY = typename std::remove_pointer<DST>::type;

  static_assert(std::is_const<typename std::remove_pointer<DST>::type>::value
                    || !std::is_const<typename std::remove_pointer<SRC>::type>::value,
                "const object can't be converted to non-const");

  if (srcPtr == nullptr) return nullptr;

  if (srcPtr->is(DST_TY::rttiMetaTy::type_id())) {
    return reinterpret_cast<DST>(
        const_cast<void*>(srcPtr->rtti_cast(DST_TY::rttiMetaTy::type_id())));
  } else {
    return nullptr;
  }
}

template <typename DST, typename SRC>
typename std::enable_if<std::is_reference<DST>::value, DST>::type rtti_cast(SRC& srcRef) {
  using SRC_TY = typename std::remove_reference<SRC>::type;
  using DST_TY = typename std::remove_reference<DST>::type;

  static_assert(std::is_const<SRC_TY>::value || !std::is_const<DST_TY>::value,
                "const object can't be converted to non-const");

  auto* srcPtr = &srcRef;
  if (srcPtr->is(DST_TY::rtti_id())) {
    return *reinterpret_cast<DST_TY*>(const_cast<void*>(srcPtr->rtti_cast(DST_TY::rtti_id())));
  } else {
    throw std::bad_cast();
  }
}
