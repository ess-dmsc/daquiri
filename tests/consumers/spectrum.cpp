#include "gtest_color_print.h"
#include <consumers/spectrum.h>

class MockSpectrumDataspace : public DAQuiri::Dataspace
{
  public:
    MockSpectrumDataspace() {}

    MockSpectrumDataspace* clone() const override { return new MockSpectrumDataspace(*this); }

    bool empty() const override { return (total_count_ == 0); }

    void clear() override { total_count_ = 0; }
    void add(const DAQuiri::Entry& e) override { total_count_ += e.second; }
    void add_one(const DAQuiri::Coords&) override { total_count_++; }
    PreciseFloat get(const DAQuiri::Coords&) const override { return 0; }
    DAQuiri::EntryList range(std::vector<DAQuiri::Pair>) const override { return DAQuiri::EntryList(); }
    void recalc_axes() override {}

    void export_csv(std::ostream&) const override {}

  protected:
    void data_save(const hdf5::node::Group&) const override {}
    void data_load(const hdf5::node::Group&) override {}
};

class MockSpectrum : public DAQuiri::Spectrum
{
  public:
    MockSpectrum()
    {
      data_ = std::make_shared<MockSpectrumDataspace>();
    }
    MockSpectrum* clone() const override { return new MockSpectrum(*this); }

    bool accept_events{true};

  protected:
    std::string my_type() const override { return "MockSpectrum"; }

    void _recalc_axes() override {}

    //event processing
    void _push_event(const DAQuiri::Event&) override
    {
      if (accept_events)
        data_->add_one({});
    }

    bool _accept_events(const DAQuiri::Spill&) override { return accept_events; }
};

class Spectrum : public TestBase
{
  protected:
    virtual void SetUp()
    {
    }

    MockSpectrum m;
    DAQuiri::Spill s{"", DAQuiri::Spill::Type::start};
};

TEST_F(Spectrum, DefaultConstruct)
{
  EXPECT_FALSE(m.changed());
  EXPECT_EQ(m.type(), "MockSpectrum");
}

TEST_F(Spectrum, StartTime)
{
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), hr_time_t());

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::start);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), s.time);
}

TEST_F(Spectrum, FiltersSpillType)
{
  s = DAQuiri::Spill("", DAQuiri::Spill::Type::daq_status);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), hr_time_t());

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::start);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), s.time);
}

TEST_F(Spectrum, FiltersStreamID)
{
  m.set_attribute(DAQuiri::Setting::text("stream_id", "someid"));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), hr_time_t());

  s = DAQuiri::Spill("someid", DAQuiri::Spill::Type::start);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), s.time);
}

TEST_F(Spectrum, TotalCount)
{
  s.events.reserve(3, DAQuiri::Event(DAQuiri::EventModel()));
  ++s.events;
  ++s.events;
  ++s.events;
  s.events.finalize();

  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 3);
}

TEST_F(Spectrum, RecentCount)
{
  EXPECT_EQ(m.metadata().get_attribute("recent_native_time_rate").get_number(), 0);

  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 0));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("recent_native_time_rate").get_number(), 0);

  s.state.set(DAQuiri::Setting::floating("native_time", pow(10, 9)));
  s.events.reserve(3, DAQuiri::Event(DAQuiri::EventModel()));
  ++s.events;
  ++s.events;
  ++s.events;
  s.events.finalize();
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("recent_native_time_rate").get_number(), 3);

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("recent_native_time_rate").get_number(), 0);
}

TEST_F(Spectrum, RealTime)
{
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(), hr_duration_t());

  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 1 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(),
      std::chrono::microseconds(0));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 2 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(),
            std::chrono::microseconds(1));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::stop);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 3 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(),
            std::chrono::microseconds(2));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::start);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 7 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(),
            std::chrono::microseconds(2));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 10 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(),
            std::chrono::microseconds(5));

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration(),
            std::chrono::microseconds(5));
}

TEST_F(Spectrum, LiveTimeDefaultsToRealTime)
{
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(), hr_duration_t());

  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 1 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(0));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 2 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(1));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::stop);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 3 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(2));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::start);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 7 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(2));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 10 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(5));

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(5));
}

TEST_F(Spectrum, LiveTime)
{
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(), hr_duration_t());

  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 1 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(0));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 2 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(1));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::stop);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 3 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(2));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::start);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 7 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(2));

  s = DAQuiri::Spill("", DAQuiri::Spill::Type::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 10 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(5));

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration(),
            std::chrono::microseconds(5));
}

TEST_F(Spectrum, PeriodicClear)
{
  m.set_attribute(DAQuiri::Setting::boolean("periodic_trigger/enabled", true));
  m.set_attribute(DAQuiri::Setting("periodic_trigger/time_out", std::chrono::seconds(2)));

  s.events.reserve(3, DAQuiri::Event(DAQuiri::EventModel()));
  ++s.events;
  ++s.events;
  ++s.events;
  s.events.finalize();

  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 3);

  s.time += std::chrono::seconds(1);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 6);

  s.time += std::chrono::seconds(1);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 9);

  s.time += std::chrono::seconds(1);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 3);
}
