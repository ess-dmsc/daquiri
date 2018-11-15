#pragma once

#include <core/consumer_metadata.h>
#include <core/spill.h>
#include <core/dataspace.h>

namespace DAQuiri {

class Consumer;

using ConsumerPtr = std::shared_ptr<Consumer>;

class Consumer
{
  protected:
    mutable mutex_st mutex_;
    ConsumerMetadata metadata_;
    DataspacePtr data_;
    bool changed_ {false};

  public:
    Consumer();
    Consumer(const Consumer& other)
      : metadata_(other.metadata_)
      , changed_ {true} // \todo: really?
    {
        if (other.data_)
            data_ = DataspacePtr(other.data_->clone());
    }
    virtual Consumer* clone() const = 0;
    virtual ~Consumer() {}

    //named constructors, used by factory
    void from_prototype(const ConsumerMetadata&);

    void load(hdf5::node::Group&, bool withdata);
    void save(hdf5::node::Group&) const;

    //data acquisition
    void push_spill(const Spill&);
    void flush();

    ConsumerMetadata metadata() const;
    DataspacePtr data() const;

    void reset_changed();
    bool changed() const;

    //Convenience functions for most common metadata
    std::string type() const;
    uint16_t dimensions() const;
    std::string debug(std::string prepend = "", bool verbose = true) const;

    //Change metadata
    void set_attribute(const Setting &setting, bool greedy = false);
    void set_attributes(const Setting &settings);
    void set_detectors(const std::vector<Detector>& dets);

  protected:
    //////////////////////////////////////////
    //////////THIS IS THE MEAT////////////////
    ///implement these to make custom types///
    //////////////////////////////////////////

    virtual std::string my_type() const = 0;
    virtual void _apply_attributes();
    virtual void _init_from_file();
    virtual void _recalc_axes() = 0;

    virtual void _set_detectors(const std::vector<Detector>& dets);

    virtual void _push_spill(const Spill&);

    virtual bool _accept_spill(const Spill& spill) = 0;
    virtual bool _accept_events(const Spill& spill) = 0;

    virtual void _push_stats_pre(const Spill&) {}
    virtual void _push_event(const Event&) = 0;
    virtual void _push_stats_post(const Spill&) {}

    virtual void _flush() {}

  private:
    std::string stream_id_;
};

}
