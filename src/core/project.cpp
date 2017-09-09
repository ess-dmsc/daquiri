#include "project.h"
#include "consumer_factory.h"
#include "custom_logger.h"

#include "H5CC_File.h"
#include "h5json.h"
#include "print_exception.h"

namespace DAQuiri {

Project::Project(const Project& other)
{
  ready_ = true;
  newdata_ = true;
  changed_ = true;
  identity_ = other.identity_;
  current_index_= other.current_index_;
  sinks_ = other.sinks_;
  spills_ = other.spills_;
  for (auto sink : other.sinks_)
    sinks_[sink.first] = ConsumerFactory::singleton().create_copy(sink.second);
  DBG << "<Project> deep copy performed";
}


std::string Project::identity() const
{
  UNIQUE_LOCK_EVENTUALLY
  return identity_;
}

std::list<Spill> Project::spills() const
{
  UNIQUE_LOCK_EVENTUALLY
  return spills_;
}

void Project::clear()
{
  UNIQUE_LOCK_EVENTUALLY
  clear_helper();
  cond_.notify_all();
}

void Project::clear_helper()
{
  //private, no lock needed
  if (!sinks_.empty() || !spills_.empty())
    changed_ = true;

  sinks_.clear();
  spills_.clear();
  current_index_ = 0;
}

void Project::flush()
{
  UNIQUE_LOCK_EVENTUALLY

  if (!sinks_.empty())
    for (auto &q: sinks_) {
      //DBG << "closing " << q->name();
      q.second->flush();
    }
}

void Project::activate()
{
  UNIQUE_LOCK_EVENTUALLY
  ready_ = true;
  cond_.notify_all();
}

bool Project::wait_ready()
{
  UNIQUE_LOCK_EVENTUALLY
  while (!ready_)
    cond_.wait(ulock);
  ready_ = false;
  return true;
}

bool Project::new_data()
{
  UNIQUE_LOCK_EVENTUALLY
  bool ret = newdata_;
  newdata_ = false;
  return ret;
}

bool Project::changed() const
{
  UNIQUE_LOCK_EVENTUALLY
  for (auto &q : sinks_)
    if (q.second->changed())
      changed_ = true;

  return changed_;
}

void Project::mark_changed()
{
  UNIQUE_LOCK_EVENTUALLY
  changed_ = true;
}

bool Project::empty() const
{
  UNIQUE_LOCK_EVENTUALLY
  return sinks_.empty();
}

std::vector<std::string> Project::types() const
{
  UNIQUE_LOCK_EVENTUALLY
  std::set<std::string> my_types;
  for (auto &q: sinks_)
    my_types.insert(q.second->type());
  std::vector<std::string> output(my_types.begin(), my_types.end());
  return output;
}

ConsumerPtr Project::get_sink(int64_t idx)
{
  UNIQUE_LOCK_EVENTUALLY
  //threadsafe so long as sink implemented as thread-safe
  if (sinks_.count(idx))
    return sinks_.at(idx);
  else
    return nullptr;
}

std::map<int64_t, ConsumerPtr> Project::get_sinks(int32_t dimensions)
{
  UNIQUE_LOCK_EVENTUALLY
  //threadsafe so long as sink implemented as thread-safe
  
  if (dimensions == -1)
    return sinks_;

  std::map<int64_t, ConsumerPtr> ret;
  for (auto &q: sinks_)
    if (q.second->dimensions() == dimensions)
      ret.insert(q);
  return ret;
}

std::map<int64_t, ConsumerPtr> Project::get_sinks(std::string type)
{
  UNIQUE_LOCK_EVENTUALLY
  //threadsafe so long as sink implemented as thread-safe
  
  std::map<int64_t, ConsumerPtr> ret;
  for (auto &q: sinks_)
    if (q.second->type() == type)
      ret.insert(q);
  return ret;
}



//client should activate replot after loading all files, as loading multiple
// sink might create a long queue of plot update signals
int64_t Project::add_sink(ConsumerPtr sink)
{
  if (!sink)
    return 0;
  UNIQUE_LOCK_EVENTUALLY

  sinks_[++current_index_] = sink;
  changed_ = true;
  ready_ = true;
  newdata_ = true;
  // cond_.notify_one();
  return current_index_;
}

int64_t Project::add_sink(ConsumerMetadata prototype)
{
  UNIQUE_LOCK_EVENTUALLY

  ConsumerPtr sink = ConsumerFactory::singleton().create_from_prototype(prototype);
  if (!sink)
    return 0;
  sinks_[++current_index_] = sink;
  changed_ = true;
  ready_ = true;
  newdata_ = false;
  // cond_.notify_one();
  return current_index_;
}

void Project::delete_sink(int64_t idx)
{
  UNIQUE_LOCK_EVENTUALLY

  if (!sinks_.count(idx))
    return;

  sinks_.erase(idx);
  changed_ = true;
  ready_ = true;
  newdata_ = false;
  // cond_.notify_one();

  if (sinks_.empty())
    current_index_ = 0;
}

void Project::set_prototypes(const Container<ConsumerMetadata>& prototypes)
{
  UNIQUE_LOCK_EVENTUALLY

  clear_helper();

  for (size_t i=0; i < prototypes.size(); i++)
  {
//    DBG << "Creating sink " << prototypes.get(i).debug();

    ConsumerPtr sink = ConsumerFactory::singleton().create_from_prototype(prototypes.get(i));
    if (sink)
    {
      sinks_[++current_index_] = sink;
//      DBG << "Added sink " << sink->debug();
    }
  }

  changed_ = true;
  ready_ = true;
  newdata_ = false;
  cond_.notify_all();
}

void Project::add_spill(SpillPtr one_spill)
{
  UNIQUE_LOCK_EVENTUALLY

  for (auto &q: sinks_)
    q.second->push_spill(*one_spill);

  if (!one_spill->detectors.empty()
      || !one_spill->state.branches.empty())
    spills_.push_back(*one_spill);

  if ((!one_spill->stats.empty())
      || (!one_spill->events.empty())
      || (!one_spill->data.empty())
      || (!one_spill->state.branches.empty())
      || (!one_spill->detectors.empty()))
    changed_ = true;
  
  ready_ = true;
  newdata_ = true;
  cond_.notify_all();
}


void Project::save()
{
  UNIQUE_LOCK_EVENTUALLY

  if (/*changed_ && */(identity_ != "New project"))
    save_as(identity_);
}

void Project::save_as(std::string file_name)
{
  write_h5(file_name);
}

void Project::open(std::string file_name, bool with_sinks, bool with_full_sinks)
{
  if (H5::H5File::isHdf5(file_name))
    read_h5(file_name, with_sinks, with_full_sinks);
}

void Project::to_h5(H5CC::Group &group) const
{
  group.write_attribute("git_version", std::string(GIT_VERSION));

  if (!spills_.empty())
  {
    auto sg = group.require_group("spills");
    int i=0;
    size_t len = std::to_string(spills_.size() - 1).size();
    for (auto &s :spills_)
    {
      std::string name = std::to_string(i++);
      if (name.size() < len)
        name = std::string(len - name.size(), '0').append(name);

      auto ssg = sg.require_group(name);
      from_json(json(s), ssg);
    }
  }

  if (!sinks_.empty())
  {
    auto sg = group.require_group("sinks");
    int i=0;
    size_t len = std::to_string(sinks_.size() - 1).size();
    for (auto &q : sinks_)
    {
      std::string name = std::to_string(i++);
      if (name.size() < len)
        name = std::string(len - name.size(), '0').append(name);

      auto ssg = sg.require_group(name);
      ssg.write_attribute("index", q.first);
      q.second->save(ssg);
    }
  }

  changed_ = false;
  ready_ = true;
  newdata_ = true;
}

void Project::from_h5(H5CC::Group &group, bool with_sinks, bool with_full_sinks)
{
  UNIQUE_LOCK_EVENTUALLY
  clear_helper();

  if (group.has_group("spills"))
  {
    auto sgroup = group.open_group("spills");
    for (auto g : sgroup.groups())
    {
      json j;
      to_json(j, sgroup.open_group(g));
      Spill sp = j;
      spills_.push_back(sp);
    }
  }

  if (!with_sinks)
    return;

  if (group.has_group("sinks"))
    for (auto g : group.open_group("sinks").groups())
    {
      auto sg = group.open_group("sinks").open_group(g);

      if (sg.has_attribute("index"))
        current_index_ = sg.read_attribute<int64_t>("index");
      else
      {
        WARN << "<Project> Consumer has no index";
        continue;
      }

      ConsumerPtr sink = ConsumerFactory::singleton().create_from_h5(sg, with_full_sinks);
      if (!sink)
        WARN << "<Project> Could not parse sink";
      else
        sinks_[current_index_] = sink;
    }

  current_index_++;

  changed_ = false;
  ready_ = true;
  newdata_ = true;
}

void Project::write_h5(std::string file_name)
{
  try
  {
    H5CC::File f(file_name, H5CC::Access::rw_truncate);
    auto group = f.require_group("project");
    to_h5(group);

    for (auto &q : sinks_)
      q.second->reset_changed();

    unique_lock lock(mutex_);
    identity_ = file_name;
    cond_.notify_all();
  }
  catch (...)
  {
    ERR << "<Project> Failed to write h5 " << file_name;
    printException();
  }
}

void Project::read_h5(std::string file_name,
                      bool with_sinks,
                      bool with_full_sinks)
{
//  try
//  {
    H5CC::File f(file_name, H5CC::Access::r_existing);
    auto group = f.require_group("project");
    from_h5(group, with_sinks, with_full_sinks);

    unique_lock lock(mutex_);
    identity_ = file_name;
    cond_.notify_all();
//  }
//  catch (...)
//  {
//    ERR << "<Project> Failed to read h5 " << file_name;
//    printException();
//  }
}

}
