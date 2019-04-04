#include <gui/Profiles.h>
#include <core/engine.h>
#include <QSettings>
#include <QDir>
#include <core/util/json_file.h>

#include <core/util/logger.h>

#define DEFAULT_SETTINGS_PATH "essdaq/daquiri"
#define PROFILES_SUBDIR "/profiles"
#define PROFILE_FILE_NAME "profile.set"

bool Profiles::has_settings_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return (settings.contains("settings_directory") &&
      !settings.value("settings_directory", "").toString().isEmpty());
}

QString Profiles::default_settings_dir()
{
  return QDir::homePath() + "/" + DEFAULT_SETTINGS_PATH;
}

QString Profiles::settings_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return settings.value("settings_directory", default_settings_dir()).toString();
}

void Profiles::select_settings_dir(QString dir)
{
  if (dir.isEmpty())
    return;
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("settings_directory",
                    QDir(dir).absolutePath());

  QDir profpath(profiles_dir());
  if (!profpath.exists())
    profpath.mkpath(".");
}

bool Profiles::is_valid_settings_dir(QString dir)
{
  QDir profpath(dir + PROFILES_SUBDIR);
  if (!profpath.exists())
    return false;
  profpath.setFilter(QDir::Dirs |
      QDir::NoDot |
      QDir::NoDotDot |
      QDir::NoSymLinks);

  QFileInfoList list = profpath.entryInfoList();
  for (int i = 0; i < list.size(); ++i)
  {
    QFileInfo fileInfo = list.at(i);
    try
    {
      if (!from_json_file(fileInfo.absoluteFilePath().toStdString()
                              + "/" + PROFILE_FILE_NAME).empty())
        return true;
    }
    catch (...) {}
  }
  return false;
}

QString Profiles::profiles_dir()
{
  return settings_dir() + PROFILES_SUBDIR;
}

QString Profiles::current_profile_name()
{
  return current_profile_name_;
}

bool Profiles::is_valid_profile(QString name)
{
  return !get_profile(name).empty();
}

QString Profiles::profile_dir(QString name)
{
  if (name.isEmpty())
    return "";
  return profiles_dir() + "/" + name;
}

QString Profiles::current_profile_dir()
{
  return profile_dir(current_profile_name());
}

nlohmann::json Profiles::get_profile(QString name)
{
  nlohmann::json profile;
  if (!name.isEmpty())
  {
    auto dir = profiles_dir() + "/" + name;
    try
    {
      profile = from_json_file(dir.toStdString()
                                   + "/" + PROFILE_FILE_NAME);
    }
    catch (...) {}
  }
  return profile;
}

nlohmann::json Profiles::get_current_profile()
{
  return get_profile(current_profile_name());
}

void Profiles::save_profile(const nlohmann::json& data)
{
  auto name = current_profile_name();
  if (!name.isEmpty())
    to_json_file(data, current_profile_dir().toStdString()
        + "/" + PROFILE_FILE_NAME);
}

void Profiles::select_profile(QString name, bool boot)
{
  current_profile_name_ = name;
  auto_boot_ = boot;
}

bool Profiles::profile_exists(QString name)
{
  return QDir(profile_dir(name)).exists();
}

void Profiles::create_profile(QString name)
{
  auto dir = profile_dir(name);
  if (!QDir(dir).exists())
    QDir().mkdir(dir);

  auto profile = DAQuiri::Engine::default_settings();
  profile.condense();
  profile.strip_metadata();
  to_json_file(profile, dir.toStdString() + "/" + PROFILE_FILE_NAME);
}

void Profiles::remove_profile(QString name)
{
  QDir path(profile_dir(name));
  if (path.exists())
    path.removeRecursively();
}

bool Profiles::auto_boot() const
{
  return auto_boot_;
}

void Profiles::auto_boot(bool boot)
{
  auto_boot_ = boot;
}

