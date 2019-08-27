#pragma once

#include <condition_variable>
#include <core/consumer.h>

namespace DAQuiri {

class Project;

using ProjectPtr = std::shared_ptr<Project>;

class Project {
public:
  Project() {}

  Project(const Project &);

  ////control//////
  bool wait_ready(); // wait for cond variable
  void activate();   // force release of cond var

  // general info
  bool empty() const;
  bool has_data() const;
  void clear();
  void reset();

  bool changed() const;
  void mark_changed();

  // to consume data
  void add_spill(SpillPtr one_spill); // feeds events to all consumers
  void flush();

  // consumers access
  void add_consumer(ConsumerPtr consumer);
  void replace(size_t idx, ConsumerPtr consumer);
  void delete_consumer(size_t idx);
  void up(size_t);
  void down(size_t);
  ConsumerPtr get_consumer(size_t idx);
  Container<ConsumerPtr> get_consumers() const;

  // spill access
  void save_spills(bool);
  bool save_spills() const;
  std::list<Spill> spills() const;

  // File ops
  void save(std::string file_name);
  void save_split(std::string base_name);
  void open(std::string file_name, bool with_consumers = true,
            bool with_full_consumers = true);

  friend std::ostream &operator<<(std::ostream &stream, const Project &project);

protected:
  // control
  mutable mutex mutex_;
  condition_variable cond_;
  mutable bool ready_{false};

  // data
  Container<ConsumerPtr> consumers_;

  // spills
  std::list<Spill> spills_;
  bool save_spills_{false};

  // aggregate info
  bool changed_{false};
  bool has_data_{false};

  // helpers
  void _clear();
  void _save_metadata(std::string file_name);
  void _add_consumer(ConsumerPtr consumer);
};

} // namespace DAQuiri
