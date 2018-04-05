#include "project.h"
#include "consumer_factory.h"
#include "custom_logger.h"

#include "h5json.h"

#include "print_exception.h"

namespace DAQuiri {

Project::Project(const Project &other)
{
  ready_ = true;
  changed_ = true;
  identity_ = other.identity_;
  consumers_ = other.consumers_;
  spills_ = other.spills_;
  for (auto consumer : other.consumers_)
    consumers_.add_a(ConsumerFactory::singleton().create_copy(consumer));
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
  if (!consumers_.empty()
      || !spills_.empty())
    changed_ = true;

  consumers_.clear();
  spills_.clear();
  has_data_ = false;
}

void Project::flush()
{
  UNIQUE_LOCK_EVENTUALLY

  if (!consumers_.empty())
  {
    for (auto &q: consumers_)
    {
      //DBG << "closing " << q->name();
      q->flush();
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

bool Project::changed() const
{
  UNIQUE_LOCK_EVENTUALLY
  for (auto &q : consumers_)
    if (q->changed())
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
  return consumers_.empty();
}

bool Project::has_data() const
{
  return has_data_;
}

void Project::reset()
{
  UNIQUE_LOCK_EVENTUALLY

  if (consumers_.empty()
      && spills_.empty())
    return;

  for (auto &q : consumers_)
    q = ConsumerFactory::singleton().create_from_prototype(q->metadata().prototype());

  spills_.clear();

  changed_ = true;
  ready_ = true;
  has_data_ = false;
  cond_.notify_all();
}

void Project::up(size_t i)
{
  UNIQUE_LOCK_EVENTUALLY
  consumers_.up(i);
  changed_ = true;
  ready_ = true;
  // cond_.notify_one();
}

void Project::down(size_t i)
{
  UNIQUE_LOCK_EVENTUALLY
  consumers_.down(i);
  changed_ = true;
  ready_ = true;
  // cond_.notify_one();
}

std::vector<std::string> Project::types() const
{
  UNIQUE_LOCK_EVENTUALLY
  std::set<std::string> my_types;
  for (auto &q: consumers_)
    my_types.insert(q->type());
  std::vector<std::string> output(my_types.begin(), my_types.end());
  return output;
}

ConsumerPtr Project::get_consumer(size_t idx)
{
  UNIQUE_LOCK_EVENTUALLY
  //threadsafe so long as consumer implemented as thread-safe
  if (idx < consumers_.size())
    return consumers_.get(idx);
  else
    return nullptr;
}

Container<ConsumerPtr> Project::get_consumers(int32_t dimensions)
{
  UNIQUE_LOCK_EVENTUALLY
  //threadsafe so long as consumer implemented as thread-safe

  if (dimensions == -1)
    return consumers_;

  Container<ConsumerPtr> ret;
  for (auto &q: consumers_)
    if (q->dimensions() == dimensions)
      ret.add_a(q);
  return ret;
}

//client should activate replot after loading all files, as loading multiple
// consumer might create a long queue of plot update signals
size_t Project::add_consumer(ConsumerPtr consumer)
{
  if (!consumer)
    return 0;
  UNIQUE_LOCK_EVENTUALLY

  consumers_.add_a(consumer);
  changed_ = true;
  ready_ = true;
  // cond_.notify_one();
  return consumers_.size() - 1;
}

size_t Project::add_consumer(ConsumerMetadata prototype)
{
  UNIQUE_LOCK_EVENTUALLY

  ConsumerPtr consumer = ConsumerFactory::singleton().create_from_prototype(prototype);
  if (!consumer)
    return 0;
  consumers_.add_a(consumer);
  changed_ = true;
  ready_ = true;
  // cond_.notify_one();
  return consumers_.size() - 1;
}

void Project::replace(size_t idx, ConsumerMetadata prototype)
{
  UNIQUE_LOCK_EVENTUALLY

  ConsumerPtr consumer = ConsumerFactory::singleton().create_from_prototype(prototype);
  if (!consumer)
    return;
  consumers_.replace(idx, consumer);

  changed_ = true;
  ready_ = true;

  // cond_.notify_one();
}


void Project::delete_consumer(size_t idx)
{
  UNIQUE_LOCK_EVENTUALLY

  if (idx >= consumers_.size())
    return;

  consumers_.remove(idx);
  changed_ = true;
  ready_ = true;
  // cond_.notify_one();
}

void Project::set_prototypes(const Container<ConsumerMetadata> &prototypes)
{
  UNIQUE_LOCK_EVENTUALLY

  clear_helper();

  for (const auto &definition : prototypes)
  {
//    DBG << "Creating consumer " << prototypes.get(i).debug();

    ConsumerPtr consumer = ConsumerFactory::singleton().create_from_prototype(definition);
    if (consumer)
    {
      consumers_.add_a(consumer);
//      DBG << "Added consumer " << consumer->debug();
    }
  }

  changed_ = true;
  ready_ = true;
  has_data_ = false;
  cond_.notify_all();
}

Container<ConsumerMetadata> Project::get_prototypes() const
{
  UNIQUE_LOCK_EVENTUALLY

  Container<ConsumerMetadata> ret;
  for (auto &q : consumers_)
    ret.add_a(q->metadata().prototype());
  return ret;
}


void Project::add_spill(SpillPtr one_spill)
{
  UNIQUE_LOCK_EVENTUALLY

  for (auto &q: consumers_)
    q->push_spill(*one_spill);

//  if (!one_spill->detectors.empty()
//      || !one_spill->state.branches.empty())
//    spills_.push_back(*one_spill);

  changed_ = true;
  has_data_ = true;
  ready_ = true;
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

    if (!consumers_.empty())
    {
      auto sg = hdf5::require_group(group, "consumers");

      int i = 0;
      for (auto &q : consumers_)
      {
        auto ssg = hdf5::require_group(sg, vector_idx_minlen(i++, consumers_.size() - 1));
        q->save(ssg);
      }
    }

    changed_ = false;
    ready_ = true;

    for (auto &q : consumers_)
      q->reset_changed();

    unique_lock lock(mutex_);
    identity_ = file_name;
    cond_.notify_all();

  }
  catch (std::exception &e)
  {
    ERR << "<Project> Failed to write '"
        << file_name << "'\n"
        << hdf5::error::print_nested(e);
  }
}

void Project::open(std::string file_name, bool with_consumers, bool with_full_consumers)
{
  if (!hdf5::file::is_hdf5_file(file_name))
    return;
//  try
//  {
    auto file = hdf5::file::open(file_name, hdf5::file::AccessFlags::READONLY);
    auto f = file.root();
    auto group = hdf5::node::Group(f["project"]);

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

    if (!with_consumers)
      return;

    if (hdf5::has_group(group, "consumers"))
      for (auto n : hdf5::node::Group(group["consumers"]).nodes)
      {
        if (n.type() != hdf5::node::Type::GROUP)
          continue;
        auto sg = hdf5::node::Group(n);

        ConsumerPtr consumer = ConsumerFactory::singleton().create_from_h5(sg, with_full_consumers);
        if (!consumer)
          WARN << "<Project> Could not parse consumer";
        else
          consumers_.add_a(consumer);
      }

    changed_ = false;
    ready_ = true;

    identity_ = file_name;
    cond_.notify_all();
//  }
//  catch (std::exception &e)
//  {
//    ERR << "<Project> Failed to read '"
//        << file_name << "'\n"
//        << hdf5::error::print_nested(e);
//    ERR << "<Project> Failed to read h5 " << file_name;
//  }
}

void Project::save_metadata(std::string file_name)
{
  json proj_json;
  proj_json["daquiri_git_version"] = std::string(GIT_VERSION);

  for (auto &s :spills_)
    proj_json["spills"].push_back(json(s));

  for (auto &q : consumers_)
  {
    auto jmeta = json(q->metadata());
    proj_json["consumers"].push_back(jmeta);
  }

  std::ofstream jfs(file_name, std::ofstream::out | std::ofstream::trunc);
  jfs << proj_json.dump(1);
  jfs.close();
}


void Project::save_split(std::string base_name)
{
  save_metadata(base_name + "_metadata.json");
  size_t i=0;
  for (auto &q : consumers_)
  {
    std::ofstream ofs(base_name + "_" + vector_idx_minlen(i++, consumers_.size()-1) + ".csv",
                      std::ofstream::out | std::ofstream::trunc);
    q->data()->save(ofs);
    ofs.close();
  }
}

std::ostream &operator<<(std::ostream &stream, const Project &project)
{
  stream << "Project \"" << project.identity_ << "\"\n";
  stream << "        " << (project.changed_ ? "CHANGED" : "NOT-CHANGED");
  stream << " " << (project.ready_ ? "READY" : " NOT-READY");
  for (const auto &s : project.spills_)
    stream << s.to_string() << "\n";
  for (const auto &s : project.consumers_)
    stream << "Consumer " << s->debug("", false) << "\n";
  return stream;
}

}
