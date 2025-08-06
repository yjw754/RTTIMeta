#include <gtest/gtest.h>
#include <rttimeta/rttimeta.h>

#include <cstddef>

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

TEST(rtti_cast, ptr_cast) {
  D1 d1;
  A4* a4 = &d1;

  A2* stdPtr = dynamic_cast<A2*>(a4);
  A2* metaPtr = rtti_cast<A2*>(a4);
  EXPECT_EQ(stdPtr, metaPtr);
}

TEST(rtti_cast, const_ptr_cast) {
  D1 d1;
  const A4* a4 = &d1;

  const A2* stdPtr = dynamic_cast<const A2*>(a4);
  const A2* metaPtr = rtti_cast<const A2*>(a4);
  EXPECT_EQ(stdPtr, metaPtr);
}

TEST(rtti_cast, ref_cast) {
  D1 d1;
  A4& a4 = d1;

  try {
    A2& stdRef = dynamic_cast<A2&>(a4);
    A2& metaRef = rtti_cast<A2&>(a4);
    EXPECT_EQ(&stdRef, &metaRef);

  } catch (std::bad_cast& e) {
    FAIL() << "ref_cast: " << e.what();
  }
}

TEST(rtti_cast, const_ref_cast) {
  D1 d1;
  const A4& a4 = d1;

  try {
    const A2& stdRef = dynamic_cast<const A2&>(a4);
    const A2& metaRef = rtti_cast<const A2&>(a4);
    EXPECT_EQ(&stdRef, &metaRef);

  } catch (std::bad_cast& e) {
    FAIL() << "const_ref_cast: " << e.what();
  }
}

TEST(rtti_cast, rvalue_ptr_cast) {
  D1 d1;
  A2* stdPtr = dynamic_cast<A2*>(&d1);
  A2* metaPtr = rtti_cast<A2*>(&d1);
  EXPECT_EQ(stdPtr, metaPtr);
}

TEST(rtti_cast, rvalue_ref_cast) {
  D1 d1;

  try {
    A2& stdRef = dynamic_cast<A2&>(d1);
    A2& metaRef = rtti_cast<A2&>(d1);
    EXPECT_EQ(&stdRef, &metaRef);

  } catch (std::bad_cast& e) {
    FAIL() << "rvalue_ref_cast: " << e.what();
  }
}

namespace test {
  struct Animal {
    RTTI_META_DEFINE(Animal);
  };
}  // namespace test
using namespace test;

struct Flyable : virtual Animal {
  RTTI_META_DEFINE(Flyable, Animal);
};

struct Swimmable : virtual Animal {
  RTTI_META_DEFINE(Swimmable, Animal);
};

struct Penguin : Flyable, Swimmable {
  RTTI_META_DEFINE(Penguin, Flyable, Swimmable);
};

TEST(TypeInfo, SameTypesAreEqual) { EXPECT_STREQ(TypeName<Animal>().c_str(), "test::Animal"); }

TEST(rtti_cast, virtual_inheritance_cast) {
  Penguin penguin;

  // from Penguin to Animal*
  Animal* animal = &penguin;

  EXPECT_NE(rtti_cast<Flyable*>(animal), nullptr);
  EXPECT_NE(rtti_cast<Swimmable*>(animal), nullptr);
  EXPECT_NE(rtti_cast<Penguin*>(animal), nullptr);
}

TEST(RTTICastTest, fail_ptr_cast) {
  C1 c1;
  A1* ptr = &c1;

  EXPECT_EQ(rtti_cast<A4*>(ptr), nullptr);
}

TEST(RTTICastTest, fail_ref_cast) {
  C1 c1;
  A1& aRef = c1;

  EXPECT_THROW(
      {
        A4& pRef = rtti_cast<A4&>(aRef);
        (void)pRef;
      },
      std::bad_cast);
}