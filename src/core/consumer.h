#pragma once

#include "consumer_metadata.h"
#include "spill.h"
#include "dataspace.h"

namespace DAQuiri {

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
    , data_ (other.data_->clone())
    , changed_ {true}
  {}
  virtual Consumer* clone() const = 0;
  virtual ~Consumer() {}

  //named constructors, used by factory
  bool from_prototype(const ConsumerMetadata&);

  bool load(H5CC::Group&, bool withdata);
  bool save(H5CC::Group&) const;

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
  virtual bool _initialize();
  virtual void _init_from_file();
  virtual void _recalc_axes() = 0;

  virtual void _set_detectors(const std::vector<Detector>& dets) = 0;
  virtual void _push_spill(const Spill&);
  virtual void _push_event(const Event&) = 0;
  virtual void _push_stats(const Status&) = 0;
  virtual void _flush() {}
};

typedef std::shared_ptr<Consumer> ConsumerPtr;

}
