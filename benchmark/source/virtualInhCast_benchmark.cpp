#include <benchmark/benchmark.h>
#include <rttimeta/rttimeta.h>

struct GrandParent {
  RTTI_META_DEFINE(GrandParent);
};

struct ParentA : virtual GrandParent {
  RTTI_META_DEFINE(ParentA, GrandParent);
};

struct ParentB : virtual GrandParent {
  RTTI_META_DEFINE(ParentB, GrandParent);
};

struct Child : ParentA, ParentB {
  RTTI_META_DEFINE(Child, ParentA, ParentB);
};

static void virtualInhRttiCast(benchmark::State& state) {
  Child c;

  for (auto _ : state) {
    // Upcasting
    GrandParent* pg;
    benchmark::DoNotOptimize(pg = rtti_cast<GrandParent*>(&c));

    ParentA* pa;
    benchmark::DoNotOptimize(pa = rtti_cast<ParentA*>(&c));

    ParentB* pb;
    benchmark::DoNotOptimize(pb = rtti_cast<ParentB*>(&c));

    // Downcasting
    Child* pc;
    benchmark::DoNotOptimize(pc = rtti_cast<Child*>(pg));
    benchmark::DoNotOptimize(pc = rtti_cast<Child*>(pa));
    benchmark::DoNotOptimize(pc = rtti_cast<Child*>(pb));
  }
}
BENCHMARK(virtualInhRttiCast)->Iterations(1000000);

static void virtualInhDynamicCast(benchmark::State& state) {
  Child c;

  for (auto _ : state) {
    // Upcasting
    GrandParent* pg;
    benchmark::DoNotOptimize(pg = dynamic_cast<GrandParent*>(&c));

    ParentA* pa;
    benchmark::DoNotOptimize(pa = dynamic_cast<ParentA*>(&c));

    ParentB* pb;
    benchmark::DoNotOptimize(pb = dynamic_cast<ParentB*>(&c));

    // Downcasting
    Child* pc;
    benchmark::DoNotOptimize(pc = dynamic_cast<Child*>(pg));
    benchmark::DoNotOptimize(pc = dynamic_cast<Child*>(pa));
    benchmark::DoNotOptimize(pc = dynamic_cast<Child*>(pb));
  }
}
BENCHMARK(virtualInhDynamicCast)->Iterations(1000000);

BENCHMARK_MAIN();