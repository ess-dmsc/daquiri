#include "gtest_color_print.h"
#include <core/producer_factory.h>

class ProducerFactory : public TestBase
{
};

class FakeProducer : public DAQuiri::Producer
{
 public:
  FakeProducer() {}
  ~FakeProducer() {}

  void settings(const DAQuiri::Setting&) override {}
  DAQuiri::Setting settings() const override { return DAQuiri::Setting(); }

  void boot() override {}
  void die() override {}
};

class Producer1 : public FakeProducer
{
 public:
  std::string plugin_name() const override { return "Producer1"; }
  DAQuiri::Setting settings() const override
  {
    using namespace DAQuiri;
    Setting set(SettingMeta("root", SettingType::stem));
    set.branches.add_a(Setting::integer("a1", 10));
    return set;
  }
};

class Producer2 : public FakeProducer
{
 public:
  std::string plugin_name() const override { return "Producer2"; }
  DAQuiri::Setting settings() const override
  {
    using namespace DAQuiri;
    Setting set(SettingMeta("root", SettingType::stem));
    set.branches.add_a(Setting::integer("a2", 10));
    return set;
  }
};

class Producer3 : public FakeProducer
{
 public:
  std::string plugin_name() const override { return "Producer3"; }
  DAQuiri::Setting settings() const override
  {
    using namespace DAQuiri;
    Setting set(SettingMeta("root", SettingType::stem));
    set.branches.add_a(Setting::integer("a3", 10));
    return set;
  }
};


TEST_F(ProducerFactory, Singleton)
{
  auto& a = DAQuiri::ProducerFactory::singleton();
  auto& b = DAQuiri::ProducerFactory::singleton();
  EXPECT_EQ(&a, &b);
}

TEST_F(ProducerFactory, types)
{
  auto& cf = DAQuiri::ProducerFactory::singleton();
  cf.clear();

  EXPECT_TRUE(cf.types().empty());
  EXPECT_EQ(cf.types().size(), 0UL);

  DAQUIRI_REGISTER_PRODUCER(Producer1);
  EXPECT_EQ(cf.types().size(), 1UL);

  DAQUIRI_REGISTER_PRODUCER(Producer2);
  EXPECT_EQ(cf.types().size(), 2UL);

  DAQUIRI_REGISTER_PRODUCER(Producer3);
  EXPECT_EQ(cf.types().size(), 3UL);

  cf.clear();
  EXPECT_EQ(cf.types().size(), 0UL);
  EXPECT_TRUE(cf.types().empty());
}

TEST_F(ProducerFactory, create_type)
{
  auto& cf = DAQuiri::ProducerFactory::singleton();
  cf.clear();
  DAQUIRI_REGISTER_PRODUCER(Producer1);
  DAQUIRI_REGISTER_PRODUCER(Producer2);

  EXPECT_FALSE(cf.create_type("bad_id"));
  EXPECT_EQ(cf.create_type("Producer1")->plugin_name(), "Producer1");
  EXPECT_EQ(cf.create_type("Producer2")->plugin_name(), "Producer2");
}

TEST_F(ProducerFactory, default_settings)
{
  auto& cf = DAQuiri::ProducerFactory::singleton();
  cf.clear();
  DAQUIRI_REGISTER_PRODUCER(Producer1);
  DAQUIRI_REGISTER_PRODUCER(Producer2);

  EXPECT_FALSE(cf.default_settings("bad_id"));
  EXPECT_TRUE(cf.default_settings("Producer1").has(DAQuiri::Setting("a1")));
  EXPECT_TRUE(cf.default_settings("Producer2").has(DAQuiri::Setting("a2")));
}


