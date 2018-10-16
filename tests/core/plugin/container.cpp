#include <core/plugin/container.h>
#include <gtest/gtest.h>

struct SomeType
{
  int a_ {0};
  int b_ {0};

  SomeType() {}

  SomeType(int a, int b)
  {
    a_ = a;
    b_ = b;
  }

  bool shallow_equals(const SomeType& other) const
  {
    return (a_ == other.a_);
  }

  bool operator== (const SomeType& other) const
  {
    return ((a_ == other.a_) && (b_ == other.b_));
  }

  bool operator!= (const SomeType& other) const
  {
    return !operator ==(other);
  }
};

using STC = Container<SomeType>;

TEST(Container, Init)
{
  STC n;
  ASSERT_TRUE(n.empty());
}

TEST(Container, Has)
{
  STC n;

  n.add(SomeType(1,1));
  ASSERT_TRUE(n.has(SomeType(1,1)));
  ASSERT_TRUE(n.has_a(SomeType(1,1)));
  ASSERT_TRUE(n.has_a(SomeType(1,2)));
  ASSERT_FALSE(n.has_a(SomeType(2,2)));
  ASSERT_FALSE(n.has(SomeType(1,2)));
}

TEST(Container, AddClear)
{
  STC n;

  //Will not add null object
  n.add_a(SomeType());
  ASSERT_EQ(n.size(), 0UL);

  n.add(SomeType(1,1));
  ASSERT_EQ(n.size(), 1UL);

  n.add(SomeType(2,2));
  ASSERT_EQ(n.size(), 2UL);

  // add duplicate
  n.add(SomeType(2,2));
  ASSERT_EQ(n.size(), 2UL);

  // force add duplicate
  n.add_a(SomeType(2,2));
  ASSERT_EQ(n.size(), 3UL);

  n.clear();
  ASSERT_EQ(n.size(), 0UL);
}

TEST(Container, Replace)
{
  STC n;

  n.add(SomeType(1,1));
  n.add(SomeType(2,2));
  n.add(SomeType(3,3));

  n.replace(SomeType(2,3));
  ASSERT_TRUE(n.has(SomeType(2,3)));
  ASSERT_FALSE(n.has(SomeType(2,2)));

  n.replace(2, SomeType(3,4));
  ASSERT_TRUE(n.has(SomeType(3,4)));
  ASSERT_FALSE(n.has(SomeType(3,3)));
}

TEST(Container, Remove)
{
  STC n;

  n.add(SomeType(1,1));
  n.add(SomeType(2,2));
  n.add(SomeType(3,3));

  n.remove(SomeType(2,2));
  ASSERT_FALSE(n.has(SomeType(2,2)));

  n.remove_a(SomeType(3,2));
  ASSERT_FALSE(n.has(SomeType(3,3)));

  n.add(SomeType(2,2));
  n.add(SomeType(3,3));

  n.remove(1);
  ASSERT_FALSE(n.has(SomeType(2,2)));
}

TEST(Container, Get)
{
  STC n;

  n.add(SomeType(1,1));
  n.add(SomeType(2,2));
  n.add(SomeType(3,3));

  auto m = n.get(SomeType(2,3));
  ASSERT_EQ(m, SomeType(2,2));

  m = n.get(2);
  ASSERT_EQ(m, SomeType(3,3));
}

TEST(Container, UpDown)
{
  STC n;

  n.add(SomeType(1,1));
  n.add(SomeType(2,2));
  n.add(SomeType(3,3));

  n.up(2);
  ASSERT_EQ(n.get(1), SomeType(3,3));
  ASSERT_EQ(n.get(2), SomeType(2,2));

  n.down(0);
  ASSERT_EQ(n.get(0), SomeType(3,3));
  ASSERT_EQ(n.get(1), SomeType(1,1));
}

TEST(Container, Compare)
{
  STC n, m, o;

  m.add(SomeType(1,1));
  m.add(SomeType(2,2));

  n.add(SomeType(1,1));
  n.add(SomeType(2,2));
  n.add(SomeType(3,3));

  o = m;

  ASSERT_TRUE(m == o);
  ASSERT_TRUE(o == m);
  ASSERT_FALSE(m == n);
  ASSERT_FALSE(n == m);
  ASSERT_TRUE(m != n);
  ASSERT_TRUE(n != m);
  ASSERT_FALSE(m != o);
  ASSERT_FALSE(o != m);
}
