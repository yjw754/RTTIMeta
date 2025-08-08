#include <benchmark/benchmark.h>
#include <rttimeta/rttimeta.h>

// 顶层：最基础的类
struct A1 {
  RTTI_META_DEFINE(A1);
};
struct A2 {
  RTTI_META_DEFINE(A2);
};
struct A3 {
  RTTI_META_DEFINE(A3);
};
struct A4 {
  RTTI_META_DEFINE(A4);
};

// 第二层：分别继承不同的基础类
struct B1 : A1 {
  RTTI_META_DEFINE(B1, A1);
};
struct B2 : A2 {
  RTTI_META_DEFINE(B2, A2);
};
struct B3 : A3 {
  RTTI_META_DEFINE(B3, A3);
};
struct B4 : A4 {
  RTTI_META_DEFINE(B4, A4);
};

// 第三层：组合不同的 B 类（无重复基类，避免菱形）
struct C1 : B1, B2 {
  RTTI_META_DEFINE(C1, B1, B2);

};  // A1, A2
struct C2 : B3, B4 {
  RTTI_META_DEFINE(C2, B3, B3);

};  // A3, A4

// 第四层：最终组合为一个类
struct D1 : C1, C2 {
  RTTI_META_DEFINE(D1, C1, C2);

};  // A1, A2, A3, A4

static void rttiMetaCast(benchmark::State& state) {
  D1 d1;
  A1* a1 = &d1;
  // warm-up
  benchmark::DoNotOptimize(rtti_cast<A4*>(a1));
  benchmark::DoNotOptimize(rtti_cast<B4*>(a1));
  benchmark::DoNotOptimize(rtti_cast<C2*>(a1));

  for (auto _ : state) {
    benchmark::DoNotOptimize(rtti_cast<A4*>(a1));
    benchmark::DoNotOptimize(rtti_cast<B4*>(a1));
    benchmark::DoNotOptimize(rtti_cast<C2*>(a1));
  }
}
BENCHMARK(rttiMetaCast);

static void dynamicCast(benchmark::State& state) {
  D1 d1;
  A1* a1 = &d1;
  // warm-up
  benchmark::DoNotOptimize(dynamic_cast<A4*>(a1));
  benchmark::DoNotOptimize(dynamic_cast<B4*>(a1));
  benchmark::DoNotOptimize(dynamic_cast<C2*>(a1));

  for (auto _ : state) {
    benchmark::DoNotOptimize(dynamic_cast<A4*>(a1));
    benchmark::DoNotOptimize(dynamic_cast<B4*>(a1));
    benchmark::DoNotOptimize(dynamic_cast<C2*>(a1));
  }
}
BENCHMARK(dynamicCast);

BENCHMARK_MAIN();
