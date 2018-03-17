#pragma once

#include "consumer.h"
#include <condition_variable>

namespace DAQuiri {

class Project;
using ProjectPtr = std::shared_ptr<Project>;

class Project {
protected:
  //control
  mutable mutex mutex_;
  condition_variable cond_;
  mutable bool ready_ {false};
  mutable bool newdata_ {false};
  int64_t current_index_ {0};

  //data
  std::map<int64_t, ConsumerPtr> sinks_;
  std::list<Spill> spills_;

  //saveability
  std::string   identity_ {"New project"};
  mutable bool  changed_  {false};

public:
  Project() {}
  Project(const Project&);

  ////control//////
  void clear();
  void activate();    //force release of cond var

  //populate one of these ways
  void set_prototypes(const Container<ConsumerMetadata>&);
  int64_t add_sink(ConsumerPtr sink);
  int64_t add_sink(ConsumerMetadata prototype);

  void save();
  void save_as(std::string file_name);

  void save_split(std::string file_name);

  void open(std::string file_name,
            bool with_sinks = true,
            bool with_full_sinks = true);

  void delete_sink(int64_t idx);

  //acquisition feeds events to all sinks
  void add_spill(SpillPtr one_spill);
  void flush();

  //status inquiry
  bool wait_ready();  //wait for cond variable
  bool new_data();    //any new since last readout?
  bool empty() const;

  //report on contents
  std::vector<std::string> types() const;
  std::string identity() const;

  bool changed() const;
  void mark_changed();

  std::list<Spill> spills() const;
  
  //get sinks
  ConsumerPtr get_sink(int64_t idx);
  std::map<int64_t, ConsumerPtr> get_sinks(int32_t dimensions = -1);
  std::map<int64_t, ConsumerPtr> get_sinks(std::string type);

  friend std::ostream &operator<<(std::ostream &stream,
                                  const Project &project);

private:
  void clear_helper();
};

}
