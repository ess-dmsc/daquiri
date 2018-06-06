#include "gtest_color_print.h"
#include "spectrum.h"

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
    DAQuiri::EntryList range(std::vector<DAQuiri::Pair> list) const override { return DAQuiri::EntryList(); }
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
    void _push_event(const DAQuiri::Event& event) override
    {
      if (accept_events)
        data_->add_one({});
    }

    bool _accept_events(const DAQuiri::Spill& spill) override { return accept_events; }
};

class Spectrum : public TestBase
{
  protected:
    virtual void SetUp()
    {
    }

    MockSpectrum m;
    DAQuiri::Spill s{"", DAQuiri::StatusType::start};
};

TEST_F(Spectrum, DefaultConstruct)
{
  EXPECT_FALSE(m.changed());
  EXPECT_EQ(m.type(), "MockSpectrum");
}

TEST_F(Spectrum, StartTime)
{
  EXPECT_TRUE(m.metadata().get_attribute("start_time").time().is_not_a_date_time());

  s = DAQuiri::Spill("", DAQuiri::StatusType::start);
  m.push_spill(s);
  EXPECT_FALSE(m.metadata().get_attribute("start_time").time().is_not_a_date_time());
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), s.time);
}

TEST_F(Spectrum, FiltersSpillType)
{
  s = DAQuiri::Spill("", DAQuiri::StatusType::daq_status);
  m.push_spill(s);
  EXPECT_TRUE(m.metadata().get_attribute("start_time").time().is_not_a_date_time());

  s = DAQuiri::Spill("", DAQuiri::StatusType::start);
  m.push_spill(s);
  EXPECT_FALSE(m.metadata().get_attribute("start_time").time().is_not_a_date_time());
  EXPECT_EQ(m.metadata().get_attribute("start_time").time(), s.time);
}

TEST_F(Spectrum, FiltersStreamID)
{
  m.set_attribute(DAQuiri::Setting::text("stream_id", "someid"));
  m.push_spill(s);
  EXPECT_TRUE(m.metadata().get_attribute("start_time").time().is_not_a_date_time());

  s = DAQuiri::Spill("someid", DAQuiri::StatusType::start);
  m.push_spill(s);
  EXPECT_FALSE(m.metadata().get_attribute("start_time").time().is_not_a_date_time());
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
  EXPECT_TRUE(m.metadata().get_attribute("real_time").duration().is_not_a_date_time());

  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 1 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration().total_microseconds(), 0);

  s = DAQuiri::Spill("", DAQuiri::StatusType::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 2 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration().total_microseconds(), 1);

  s = DAQuiri::Spill("", DAQuiri::StatusType::stop);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 3 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration().total_microseconds(), 2);

  s = DAQuiri::Spill("", DAQuiri::StatusType::start);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 7 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration().total_microseconds(), 2);

  s = DAQuiri::Spill("", DAQuiri::StatusType::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 10 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration().total_microseconds(), 5);

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("real_time").duration().total_microseconds(), 5);
}

TEST_F(Spectrum, LiveTimeDefaultsToRealTime)
{
  EXPECT_TRUE(m.metadata().get_attribute("live_time").duration().is_not_a_date_time());

  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 1 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 0);

  s = DAQuiri::Spill("", DAQuiri::StatusType::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 2 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 1);

  s = DAQuiri::Spill("", DAQuiri::StatusType::stop);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 3 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 2);

  s = DAQuiri::Spill("", DAQuiri::StatusType::start);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 7 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 2);

  s = DAQuiri::Spill("", DAQuiri::StatusType::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("native_time", 10 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 5);

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 5);
}

TEST_F(Spectrum, LiveTime)
{
  EXPECT_TRUE(m.metadata().get_attribute("live_time").duration().is_not_a_date_time());

  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 1 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 0);

  s = DAQuiri::Spill("", DAQuiri::StatusType::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 2 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 1);

  s = DAQuiri::Spill("", DAQuiri::StatusType::stop);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 3 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 2);

  s = DAQuiri::Spill("", DAQuiri::StatusType::start);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 7 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 2);

  s = DAQuiri::Spill("", DAQuiri::StatusType::running);
  s.state.branches.add_a(DAQuiri::Setting::floating("live_time", 10 * pow(10, 3)));
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 5);

  m.flush();
  EXPECT_EQ(m.metadata().get_attribute("live_time").duration().total_microseconds(), 5);
}

TEST_F(Spectrum, PeriodicClear)
{
  auto enabled = DAQuiri::Setting::boolean("enabled", true);
  enabled.set_indices({0});
  m.set_attribute(enabled);

  auto clear_at = DAQuiri::Setting("clear_at", boost::posix_time::seconds(2));
  clear_at.set_indices({0});
  m.set_attribute(clear_at);

  s.events.reserve(3, DAQuiri::Event(DAQuiri::EventModel()));
  ++s.events;
  ++s.events;
  ++s.events;
  s.events.finalize();

  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 3);

  s.time += boost::posix_time::seconds(1);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 6);

  s.time += boost::posix_time::seconds(1);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 9);

  s.time += boost::posix_time::seconds(1);
  m.push_spill(s);
  EXPECT_EQ(m.metadata().get_attribute("total_count").get_number(), 3);
}
