#pragma once

#include "consumer.h"
#include <condition_variable>

namespace DAQuiri {

class Project;

using ProjectPtr = std::shared_ptr<Project>;

class Project
{
  protected:
    //control
    mutable mutex mutex_;
    condition_variable cond_;
    mutable bool ready_ {false};

    //data
    Container<ConsumerPtr> consumers_;
    std::list<Spill> spills_;

    //saveability
    std::string identity_ {"New project"};
    mutable bool changed_ {false};

  public:
    Project() {}

    Project(const Project&);

    ////control//////
    void clear();
    void activate();    //force release of cond var

    // populate one of these ways
    void set_prototypes(const Container<ConsumerMetadata>&);
    Container<ConsumerMetadata> get_prototypes() const;
    size_t add_consumer(ConsumerPtr consumer);
    size_t add_consumer(ConsumerMetadata prototype);
    void replace(size_t idx, ConsumerMetadata prototype);
    void delete_consumer(size_t idx);
    void up(size_t);
    void down(size_t);

    // File ops
    void save();
    void save_as(std::string file_name);
    void save_split(std::string base_name);
    void save_metadata(std::string file_name);
    void open(std::string file_name,
              bool with_consumers = true,
              bool with_full_consumers = true);

    //acquisition feeds events to all consumers
    void add_spill(SpillPtr one_spill);
    void flush();

    //status inquiry
    bool wait_ready();  //wait for cond variable
    bool empty() const;

    //report on contents
    std::vector<std::string> types() const;
    std::string identity() const;

    bool changed() const;
    void mark_changed();

    std::list<Spill> spills() const;

    //get consumers
    ConsumerPtr get_consumer(size_t idx);
    Container<ConsumerPtr> get_consumers(int32_t dimensions = -1);

    friend std::ostream& operator<<(std::ostream& stream,
                                    const Project& project);

  private:
    void clear_helper();
};

}
