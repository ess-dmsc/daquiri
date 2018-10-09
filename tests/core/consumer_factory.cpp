#include "gtest_color_print.h"
#include <core/consumer_factory.h>

class ConsumerFactory : public TestBase
{
};

class FakeConsumer : public DAQuiri::Consumer
{
 public:
  FakeConsumer() : DAQuiri::Consumer() {}

 protected:
  void _recalc_axes() override {}
  bool _accept_spill(const DAQuiri::Spill&) override { return true; }
  bool _accept_events(const DAQuiri::Spill&) override { return false; }
  void _push_event(const DAQuiri::Event&) override {}
};

class Consumer1 : public FakeConsumer
{
 public:
  Consumer1() : FakeConsumer()
  {
    metadata_ = DAQuiri::ConsumerMetadata(my_type(), "Description 1");
  }
  Consumer1* clone() const override { return new Consumer1(*this); }
 protected:
  std::string my_type() const override { return "Consumer1"; }
};

class Consumer2 : public FakeConsumer
{
 public:
  Consumer2() : FakeConsumer()
  {
    metadata_ = DAQuiri::ConsumerMetadata(my_type(), "Description 2");
  }
  Consumer2* clone() const override { return new Consumer2(*this); }
 protected:
  std::string my_type() const override { return "Consumer2"; }
};

class Consumer3 : public FakeConsumer
{
 public:
  Consumer3() : FakeConsumer()
  {
    metadata_ = DAQuiri::ConsumerMetadata(my_type(), "Description 3");
  }
  Consumer3* clone() const override { return new Consumer3(*this); }
 protected:
  std::string my_type() const override { return "Consumer3"; }
};


TEST_F(ConsumerFactory, Singleton)
{
  auto& a = DAQuiri::ConsumerFactory::singleton();
  auto& b = DAQuiri::ConsumerFactory::singleton();
  EXPECT_EQ(&a, &b);
}

TEST_F(ConsumerFactory, types)
{
  auto& cf = DAQuiri::ConsumerFactory::singleton();
  cf.clear();

  EXPECT_TRUE(cf.types().empty());
  EXPECT_EQ(cf.types().size(), 0UL);

  DAQUIRI_REGISTER_CONSUMER(Consumer1);
  EXPECT_EQ(cf.types().size(), 1UL);

  DAQUIRI_REGISTER_CONSUMER(Consumer2);
  EXPECT_EQ(cf.types().size(), 2UL);

  DAQUIRI_REGISTER_CONSUMER(Consumer3);
  EXPECT_EQ(cf.types().size(), 3UL);

  cf.clear();
  EXPECT_EQ(cf.types().size(), 0UL);
  EXPECT_TRUE(cf.types().empty());
}

TEST_F(ConsumerFactory, create_type)
{
  auto& cf = DAQuiri::ConsumerFactory::singleton();
  cf.clear();
  DAQUIRI_REGISTER_CONSUMER(Consumer1);
  DAQUIRI_REGISTER_CONSUMER(Consumer2);

  EXPECT_FALSE(cf.create_type("bad_id"));
  EXPECT_EQ(cf.create_type("Consumer1")->type(), "Consumer1");
  EXPECT_EQ(cf.create_type("Consumer2")->type(), "Consumer2");
}

TEST_F(ConsumerFactory, create_copy)
{
  auto& cf = DAQuiri::ConsumerFactory::singleton();
  cf.clear();
  DAQUIRI_REGISTER_CONSUMER(Consumer1);

  auto c1 = cf.create_type("Consumer1");
  auto c2 = cf.create_copy(c1);

  EXPECT_EQ(c2->type(), "Consumer1");
  EXPECT_NE(c1.get(), c2.get());

  EXPECT_FALSE(cf.create_copy(nullptr));
}

