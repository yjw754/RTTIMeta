# RTTI for C++14
* 支持多继承、虚继承合法类型转换
* 更快的转换，取代dynamic_cast
* head-only
* 支持GCC/Clang and MSVC
* 支持指针和引用类型
## How to use
```C++
struct Animal {
  RTTI_META_DEFINE(Animal);
};

struct Mammal : virtual Animal {
  RTTI_META_DEFINE(Mammal, Animal);
};

struct Bird : virtual Animal {
  RTTI_META_DEFINE(Bird, Animal);
};

struct Bat : Mammal, Bird {
  RTTI_META_DEFINE(Bat, Mammal, Bird);
};

// Upcasting
Bat bat;
Animal* pa = rtti_cast<Animal*>(&bat);

// Downcasting
Bat* pbat = rtti_cast<Bat*>(pa);
```
* 为类添加RTTI_META_DEFINE宏，用于记录父子关系
* rtti_cast实现类型转换
* 不适用于private继承！！！
## Benchmark
```
Run on (20 X 2918.41 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x10)
  L1 Instruction 32 KiB (x10)
  L2 Unified 1280 KiB (x10)
  L3 Unified 24576 KiB (x1)
Load Average: 0.27, 0.12, 0.03
-----------------------------------------------------------------------------------
Benchmark                                         Time             CPU   Iterations
-----------------------------------------------------------------------------------
staticRttiCast/iterations:1000000             0.239 ns        0.234 ns      1000000
staticDynamicCast/iterations:1000000            173 ns          170 ns      1000000
virtualInhRttiCast/iterations:1000000          7.10 ns         6.98 ns      1000000
virtualInhDynamicCast/iterations:1000000       22.2 ns         21.8 ns      1000000
```