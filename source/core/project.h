#pragma once

#include "consumer.h"
#include <condition_variable>

namespace DAQuiri {

class Project;

using ProjectPtr = std::shared_ptr<Project>;

class Project
{
  public:
    Project() {}

    Project(const Project&);

    ////control//////
    bool wait_ready();  //wait for cond variable
    void activate();    //force release of cond var

    // general info
    bool empty() const;
    bool has_data() const;
    void clear();
    void reset();

    bool changed() const;
    void mark_changed();

    // consumers access
    size_t add_consumer(ConsumerPtr consumer);
    void replace(size_t idx, ConsumerMetadata prototype);
    void delete_consumer(size_t idx);
    void up(size_t);
    void down(size_t);
    ConsumerPtr get_consumer(size_t idx);
    Container<ConsumerPtr> get_consumers(int32_t dimensions = -1);

    // daq ops
    void add_spill(SpillPtr one_spill); // feeds events to all consumers
    void flush();

    // File ops
    void save();
    void save_as(std::string file_name);
    void save_split(std::string base_name);
    void save_metadata(std::string file_name);
    void open(std::string file_name,
              bool with_consumers = true,
              bool with_full_consumers = true);


    // report on contents
    std::vector<std::string> types() const;
    std::string identity() const;

    std::list<Spill> spills() const;


    friend std::ostream& operator<<(std::ostream& stream,
                                    const Project& project);

  protected:
    // control
    mutable mutex mutex_;
    condition_variable cond_;
    mutable bool ready_ {false};

    // data
    Container<ConsumerPtr> consumers_;
    std::list<Spill> spills_;

    // saveability
    std::string identity_ {"New project"};
    mutable bool changed_ {false};
    bool has_data_ {false};

    // helpers
    void clear_helper();
};

}
