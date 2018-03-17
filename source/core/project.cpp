#include "project.h"
#include "consumer_factory.h"
#include "custom_logger.h"

#include "h5json.h"

#include "print_exception.h"

namespace DAQuiri {

Project::Project(const Project &other)
{
  ready_ = true;
  newdata_ = true;
  changed_ = true;
  identity_ = other.identity_;
  current_index_ = other.current_index_;
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
  if (!sinks_.empty()
      )
//      || !spills_.empty())
    changed_ = true;

  sinks_.clear();
//  spills_.clear();
  current_index_ = 0;
}

void Project::flush()
{
  UNIQUE_LOCK_EVENTUALLY

  if (!sinks_.empty())
  {
    for (auto &q: sinks_)
    {
      //DBG << "closing " << q->name();
      q.second->flush();
    }
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

void Project::set_prototypes(const Container<ConsumerMetadata> &prototypes)
{
  UNIQUE_LOCK_EVENTUALLY

  clear_helper();

  for (const auto& definition : prototypes)
  {
//    DBG << "Creating sink " << prototypes.get(i).debug();

    ConsumerPtr sink = ConsumerFactory::singleton().create_from_prototype(definition);
    if (sink) {
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

//  if (!one_spill->detectors.empty()
//      || !one_spill->state.branches.empty())
//    spills_.push_back(*one_spill);

  if (!one_spill->empty())
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
  try
  {
    auto file = hdf5::file::create(file_name, hdf5::file::AccessFlags::TRUNCATE);
    auto f = file.root();
    auto group = hdf5::require_group(f, "project");

    group.attributes.create<std::string>("git_version").write(std::string(GIT_VERSION));

//  if (!spills_.empty()) {
//    auto sg = hdf5::require_group(group, "spills");

//    int i = 0;
//    size_t len = std::to_string(spills_.size() - 1).size();
//    for (auto &s :spills_) {
//      std::string name = std::to_string(i++);
//      if (name.size() < len)
//        name = std::string(len - name.size(), '0').append(name);

//      auto ssg = hdf5::require_group(sg, name);

//      hdf5::from_json(json(s), ssg);
//    }
//  }

    if (!sinks_.empty()) {
      auto sg = hdf5::require_group(group, "sinks");

      int i = 0;
      size_t len = std::to_string(sinks_.size() - 1).size();
      for (auto &q : sinks_) {
        std::string name = std::to_string(i++);
        if (name.size() < len)
          name = std::string(len - name.size(), '0').append(name);

        auto ssg = hdf5::require_group(sg, name);

        ssg.attributes.create<int64_t>("index").write(q.first);

        if (q.second->dimensions() == 1)
          q.second->save(ssg);
      }
    }

    changed_ = false;
    ready_ = true;
    newdata_ = true;

    for (auto &q : sinks_)
      q.second->reset_changed();

    unique_lock lock(mutex_);
    identity_ = file_name;
    cond_.notify_all();

  }
  catch (std::exception& e) {
    ERR << "<Project> Failed to write '"
        << file_name << "'\n"
        << hdf5::error::print_nested(e);
  }
}

void Project::open(std::string file_name, bool with_sinks, bool with_full_sinks)
{
  if (!hdf5::file::is_hdf5_file(file_name))
    return;
  try
  {
    auto file = hdf5::file::open(file_name, hdf5::file::AccessFlags::READONLY);
    auto f = file.root();
    auto group = hdf5::require_group(f, "project");

    UNIQUE_LOCK_EVENTUALLY
    clear_helper();

//  if (hdf5::has_group(group, "spills"))
//  {
//    auto sgroup = hdf5::node::Group(group["spills"]);
//    for (auto n : sgroup.nodes)
//    {
//      if (n.type() != hdf5::node::Type::GROUP)
//        continue;
//      auto g = hdf5::node::Group(n);

//      json j;
//      hdf5::to_json(j, g);
//      Spill sp = j;
//      spills_.push_back(sp);
//    }
//  }

    if (!with_sinks)
      return;

    if (hdf5::has_group(group, "sinks"))
      for (auto n : hdf5::node::Group(group["sinks"]).nodes)
      {
        if (n.type() != hdf5::node::Type::GROUP)
          continue;
        auto sg = hdf5::node::Group(n);

        if (sg.attributes.exists("index"))
          sg.attributes["index"].read(current_index_);
        else {
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

    unique_lock lock(mutex_);
    identity_ = file_name;
    cond_.notify_all();
  }
  catch (std::exception &e) {
    ERR << "<Project> Failed to read '"
        << file_name << "'\n"
        << hdf5::error::print_nested(e);
    ERR << "<Project> Failed to read h5 " << file_name;
  }
}

void Project::save_split(std::string file_name)
{
//  if (!sinks_.empty()) {
//    auto sg = hdf5::require_group(group, "sinks");
//
//    int i = 0;
//    size_t len = std::to_string(sinks_.size() - 1).size();
//    for (auto &q : sinks_) {
//      std::string name = std::to_string(i++);
//      if (name.size() < len)
//        name = std::string(len - name.size(), '0').append(name);
//
//      auto ssg = hdf5::require_group(sg, name);
//
//      ssg.attributes.create<int64_t>("index").write(q.first);
//
//      if (q.second->dimensions() == 1)
//        q.second->save(ssg);
//    }
//  }

  changed_ = false;
  ready_ = true;
  newdata_ = true;
}

std::ostream &operator<<(std::ostream &stream, const Project &project)
{
  stream << "Project \"" << project.identity_ << "\"\n";
  stream << "        " << (project.changed_ ? "CHANGED" : "NOT-CHANGED");
  stream << " " << (project.ready_ ? "READY" : " NOT-READY");
  stream << " " << (project.newdata_ ? "NEWDATA" : " NOT-NEWDATA") << "\n";
  stream << "        " << "next index = " << project.current_index_ << "\n";
  for (auto s : project.spills_)
    stream << s.to_string() << "\n";
  for (auto s : project.sinks_)
    stream << "Sink[" << s.first << "] " << s.second->debug("", false);
}

}
