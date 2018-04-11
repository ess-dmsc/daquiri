#include "project.h"
#include "consumer_factory.h"
#include "custom_logger.h"

#include "h5json.h"

#include "ascii_tree.h"

namespace DAQuiri {

Project::Project(const Project& other)
{
  ready_ = true;
  changed_ = true;
  consumers_ = other.consumers_;
  spills_ = other.spills_;
  for (auto consumer : other.consumers_)
    consumers_.add_a(ConsumerFactory::singleton().create_copy(consumer));
}

std::list<Spill> Project::spills() const
{
  UNIQUE_LOCK_EVENTUALLY
  return spills_;
}

void Project::clear()
{
  UNIQUE_LOCK_EVENTUALLY
  _clear();
  cond_.notify_all();
}

void Project::_clear()
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
    for (auto& q: consumers_)
      q->flush();
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
  for (auto& q : consumers_)
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

  for (auto& q : consumers_)
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

ConsumerPtr Project::get_consumer(size_t idx)
{
  UNIQUE_LOCK_EVENTUALLY
  if (idx < consumers_.size())
    return consumers_.get(idx);
  else
    return nullptr;
}

Container<ConsumerPtr> Project::get_consumers(int32_t dimensions) const
{
  UNIQUE_LOCK_EVENTUALLY

  if (dimensions == -1)
    return consumers_;

  Container<ConsumerPtr> ret;
  for (auto& q: consumers_)
    if (q->dimensions() == dimensions)
      ret.add_a(q);
  return ret;
}

void Project::add_consumer(ConsumerPtr consumer)
{
  UNIQUE_LOCK_EVENTUALLY

  _add_consumer(consumer);

  ready_ = true;
  // cond_.notify_one();
}

void Project::_add_consumer(ConsumerPtr consumer)
{
  //private, no lock needed
  if (!consumer)
    return;

  consumers_.add_a(consumer);
  if (!consumer->data()->empty())
    has_data_ = true;
  changed_ = true;
}

void Project::replace(size_t idx, ConsumerPtr consumer)
{
  if (!consumer)
    return;
  UNIQUE_LOCK_EVENTUALLY

  consumers_.replace(idx, consumer);
  if (!consumer->data()->empty())
    has_data_ = true;

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

void Project::add_spill(SpillPtr one_spill)
{
  UNIQUE_LOCK_EVENTUALLY

  for (auto& q: consumers_)
    q->push_spill(*one_spill);

  one_spill->raw.clear();
  one_spill->events = EventBuffer();
  spills_.push_back(*one_spill);

  changed_ = true;
  has_data_ = true;
  ready_ = true;
  cond_.notify_all();
}

void Project::save(std::string file_name)
{
  try
  {
    auto file = hdf5::file::create(file_name, hdf5::file::AccessFlags::TRUNCATE);
    auto f = file.root();
    auto group = f.create_group("project");

    group.attributes.create<std::string>("git_version").write(std::string(GIT_VERSION));

    UNIQUE_LOCK_EVENTUALLY

    if (!spills_.empty())
    {
      auto sg = group.create_group("spills");
      int i = 0;
      for (auto& s :spills_)
      {
        auto ssg = sg.create_group(vector_idx_minlen(i++, spills_.size() - 1));
        hdf5::from_json(json(s), ssg);
      }
    }

    if (!consumers_.empty())
    {
      auto sg = group.create_group("consumers");
      int i = 0;
      for (auto& q : consumers_)
      {
        auto ssg = sg.create_group(vector_idx_minlen(i++, consumers_.size() - 1));
        q->save(ssg);
      }
    }

    changed_ = false;
    ready_ = true;

    for (auto& q : consumers_)
      q->reset_changed();

    cond_.notify_all();
  }
  catch (...)
  {
    std::stringstream ss;
    ss << "DAQuiri::Project failed to save file '" << file_name << "'";
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

void Project::open(std::string file_name, bool with_consumers, bool with_full_consumers)
{

  if (!hdf5::file::is_hdf5_file(file_name))
    return;
  try
  {
    auto file = hdf5::file::open(file_name, hdf5::file::AccessFlags::READONLY);
    auto f = file.root();
    auto group = f.get_group("project");

    UNIQUE_LOCK_EVENTUALLY

    _clear();

    if (group.has_group("spills"))
    {
      for (auto n : group.get_group("spills").nodes)
      {
        if (n.type() != hdf5::node::Type::GROUP)
          continue;
        auto g = hdf5::node::Group(n);

        json j;
        hdf5::to_json(j, g);
        Spill sp = j;
        spills_.push_back(sp);
      }
    }

    has_data_ = (spills_.size() > 0);

    if (!with_consumers)
      return;

    if (group.has_group("consumers"))
      for (auto n : group.get_group("consumers").nodes)
      {
        if (n.type() != hdf5::node::Type::GROUP)
          continue;
        auto sg = hdf5::node::Group(n);

        ConsumerPtr consumer = ConsumerFactory::singleton().create_from_h5(sg, with_full_consumers);
        if (!consumer)
          WARN << "<Project> Could not parse consumer";
        else
          _add_consumer(consumer);
      }


    changed_ = false;

    ready_ = true;
    cond_.notify_all();
  }
  catch (...)
  {
    std::stringstream ss;
    ss << "DAQuiri::Project failed to open file '" << file_name << "'";
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

void Project::_save_metadata(std::string file_name)
{
  //private, no lock needed

  json proj_json;
  proj_json["daquiri_git_version"] = std::string(GIT_VERSION);

  for (auto& s :spills_)
    proj_json["spills"].push_back(json(s));

  for (auto& q : consumers_)
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
  UNIQUE_LOCK_EVENTUALLY

  _save_metadata(base_name + "_metadata.json");
  size_t i = 0;
  for (auto& q : consumers_)
  {
    std::ofstream ofs(base_name + "_" + vector_idx_minlen(i++, consumers_.size() - 1) + ".csv",
                      std::ofstream::out | std::ofstream::trunc);
    q->data()->save(ofs);
    ofs.close();
  }
}

std::ostream& operator<<(std::ostream& stream, const Project& project)
{
  stream << "---===DAQuiri Project===---\n";
  stream << "      " << (project.changed() ? "CHANGED" : "UNCHANGED");
  stream << " " << (project.has_data() ? "WITH-DATA" : " NO-DATA");
//  stream << " " << (project.ready_ ? "READY" : " NOT-READY");
  stream << "\n";
  auto spills = project.spills();
  if (spills.size())
  {
    stream << "SPILLS\n";
    size_t i = 0;
    for (const auto& s : spills)
    {
      if (++i < spills.size())
        stream << k_branch_mid_B << s.debug(k_branch_pre_B);
      else
        stream << k_branch_end_B << s.debug("  ");
    }
  }
  auto consumers = project.get_consumers();
  if (consumers.size())
  {
    stream << "CONSUMERS\n";
    size_t i = 0;
    for (const auto& s : consumers)
    {
      if (++i < consumers.size())
        stream << k_branch_mid_B << s->debug(k_branch_pre_B, false);
      else
        stream << k_branch_end_B << s->debug("  ", false);
    }
  }
  return stream;
}

}
