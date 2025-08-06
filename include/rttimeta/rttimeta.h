#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "typename.h"

constexpr uint32_t fnv1a_hash(const char* str) {
  uint32_t hash = 2166136261u;
  while (*str) {
    hash ^= (uint8_t)*str++;
    hash *= 16777619u;
  }
  return hash;
};

struct typeInfo {
  const char* name_;
  uint32_t id_;

  typeInfo(const char* name) : name_(name), id_(fnv1a_hash(name)){};
};

class rttiMeta {
public:
  bool is_derived_from(const rttiMeta* baseMeta) const {
    if (!baseMeta) return false;
    if (baseMeta == this) return true;

    for (const auto* parent : parent_meta_list_) {
      if (parent->is_derived_from(baseMeta)) return true;
    }
    return false;
  }

  uint32_t type_id() const { return typeInfo_.id_; }
  const char* type_name() const { return typeInfo_.name_; };

  virtual ~rttiMeta() = default;

protected:
  void add_parent_meta(const rttiMeta* parent) { parent_meta_list_.push_back(parent); }

  rttiMeta(const char* name) : typeInfo_(name){};

public:
  const std::vector<const rttiMeta*>* get_parent_meta_list() const { return &parent_meta_list_; }

private:
  std::vector<const rttiMeta*> parent_meta_list_;
  typeInfo typeInfo_;
};

template <typename DERIVED_CLASS, typename... BASE_CLASSES> class rttiMetaRegister final
    : public rttiMeta {
public:
  static rttiMetaRegister& get() {
    static rttiMetaRegister ins;
    return ins;
  }

private:
  rttiMetaRegister() : rttiMeta(TypeName<DERIVED_CLASS>().c_str()) {
    build_parent_meta<BASE_CLASSES...>();
  }
  rttiMetaRegister(const rttiMetaRegister&) = delete;
  rttiMetaRegister& operator=(const rttiMetaRegister&) = delete;

  template <typename FIRST, typename... REST> void build_parent_meta() {
    static_assert(std::is_convertible<DERIVED_CLASS, FIRST>::value, "not public inheritance");
    static_assert(!std::is_base_of<rttiMeta, DERIVED_CLASS>::value, "can't inherit rttiMeta");
    this->add_parent_meta(FIRST::static_rtti_meta());
    build_parent_meta<REST...>();
  }

  template <typename... REST>
  typename std::enable_if<(sizeof...(REST) == 0)>::type build_parent_meta() {}

public:
  template <typename T>
  static const void* dynamicCast(const rttiMeta* dst_meta, const T* real_ptr) {
    if (dst_meta->type_id() == get().type_id()) {
      return static_cast<const DERIVED_CLASS*>(real_ptr);
    }

#if defined(_DYNAMIC_CAST_FROM_PARENTS_IMPL) && defined(__GUNC__)
    return DynamicCastFromParentsImpl<T>(dst_meta, real_ptr, std::index_sequence<0, 1>{});
#else
    return DynamicCastFromParents<T, BASE_CLASSES...>(dst_meta, real_ptr);
#endif
  }

private:
#if defined(_DYNAMIC_CAST_FROM_PARENTS_IMPL) && defined(__GUNC__)
  template <typename T, std::size_t... Is>
  static void* DynamicCastFromParentsImpl(const rttiMeta* dst_meta, T* real_ptr,
                                          std::index_sequence<Is...>) noexcept {
    void* results[]
        = {nullptr, BASE_CLASSES::rttiMetaRegisterTy::dynamicCast(dst_meta, real_ptr)...};

    for (void* p : results) {
      if (p != nullptr) {
        return p;
      }
    }
    return nullptr;
  }
#else
  template <typename T> static const void* DynamicCastFromParents(const rttiMeta*, const T*) {
    return nullptr;
  }

  template <typename T, typename PARENT, typename... REST>
  static const void* DynamicCastFromParents(const rttiMeta* dst_meta, const T* real_ptr) {
    if (auto* result = PARENT::rttiMetaRegisterTy::dynamicCast(dst_meta, real_ptr)) return result;
    return DynamicCastFromParents<T, REST...>(dst_meta, real_ptr);
  }
#endif
};

#define RTTI_META_DEFINE(DERIVED_CLASS, ...)                                       \
public:                                                                            \
  using CUR_TYPE = DERIVED_CLASS;                                                  \
  static const rttiMeta* static_rtti_meta() { return &rttiMetaRegisterTy::get(); } \
                                                                                   \
  virtual const rttiMeta* rtti_meta() const { return static_rtti_meta(); }         \
                                                                                   \
  static const char* rtti_name() { return rttiMetaRegisterTy::get().type_name(); } \
                                                                                   \
  static uint32_t rtti_id() { return rttiMetaRegisterTy::get().type_id(); };       \
                                                                                   \
  virtual const void* rtti_cast(const rttiMeta* dst_meta) const {                  \
    return rttiMetaRegisterTy::dynamicCast(dst_meta, this);                        \
  }                                                                                \
                                                                                   \
public:                                                                            \
  using rttiMetaRegisterTy = rttiMetaRegister<DERIVED_CLASS, ##__VA_ARGS__>

template <typename DST, typename SRC>
typename std::enable_if<std::is_pointer<DST>::value && std::is_pointer<SRC>::value, DST>::type
rtti_cast(SRC srcPtr) {
  using DST_TY = typename std::remove_pointer<DST>::type;

  static_assert(std::is_const<typename std::remove_pointer<DST>::type>::value
                    || !std::is_const<typename std::remove_pointer<SRC>::type>::value,
                "const object can't be converted to non-const");

  if (srcPtr == nullptr) return nullptr;

  if (srcPtr->rtti_meta()->is_derived_from(DST_TY::static_rtti_meta())) {
    return reinterpret_cast<DST>(const_cast<void*>(srcPtr->rtti_cast(DST_TY::static_rtti_meta())));
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
  if (srcPtr->rtti_meta()->is_derived_from(DST_TY::static_rtti_meta())) {
    return *reinterpret_cast<DST_TY*>(
        const_cast<void*>(srcPtr->rtti_cast(DST_TY::static_rtti_meta())));
  } else {
    throw std::bad_cast();
  }
}
