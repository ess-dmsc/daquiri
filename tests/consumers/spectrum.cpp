#include "gtest_color_print.h"
#include "spectrum.h"

class MockSpectrum : public DAQuiri::Spectrum
{
  public:
    MockSpectrum() {}
    MockSpectrum *clone() const override { return new MockSpectrum(*this); }

  protected:
    std::string my_type() const override { return "MockSpectrum"; }

    void _recalc_axes() override {}

    //event processing
    void _push_event(const DAQuiri::Event &event) override {}
    bool _accept_events(const DAQuiri::Spill &spill) override {return false;}
};

class Spectrum : public TestBase
{
  protected:
    MockSpectrum m;
};


TEST_F(Spectrum, Init)
{
  EXPECT_FALSE(m.changed());
  EXPECT_EQ(m.type(), "MockSpectrum");
}

