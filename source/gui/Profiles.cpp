#include "Profiles.h"
#include "engine.h"
#include <QSettings>
#include <QDir>
#include "json_file.h"

#define PROFILE_FILE_NAME "profile.set"
#define PROFILE_PATH_DEFAULT "essproj/daquiri_config"

namespace Profiles
{

bool has_settings_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return (settings.contains("settings_directory") &&
          !settings.value("settings_directory", "").toString().isEmpty());
}

QString default_settings_dir()
{
  return QDir::homePath() + "/" + PROFILE_PATH_DEFAULT;
}

QString settings_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return settings.value("settings_directory", default_settings_dir()).toString();
}

void select_settings_dir(QString dir)
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

QString profiles_dir()
{
  return settings_dir() + "/profiles";
}

QString current_profile_name()
{
  QSettings settings;
  settings.beginGroup("Program");
  return settings.value("current_profile","").toString();
}

QString profile_dir(QString name)
{
  if (name.isEmpty())
    return "";
  return profiles_dir() + "/" + name;
}

QString current_profile_dir()
{
  return profile_dir(current_profile_name());
}

nlohmann::json get_profile(QString name)
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
    catch(...) {}
  }
  return profile;
}

nlohmann::json current_profile()
{
  return get_profile(current_profile_name());
}

void save_profile(const nlohmann::json& data)
{
  auto name = current_profile_name();
  if (!name.isEmpty())
    to_json_file(data, current_profile_dir().toStdString()
                 + "/" + PROFILE_FILE_NAME);
}

void select_profile(QString name, bool boot)
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("current_profile", name);
  settings.setValue("boot_on_startup", boot);
}

bool profile_exists(QString name)
{
  return QDir(profile_dir(name)).exists();
}

void create_profile(QString name)
{
  auto dir = profile_dir(name);
  if (!QDir(dir).exists())
    QDir().mkdir(dir);

  auto profile = DAQuiri::Engine::default_settings();
  profile.condense();
  profile.strip_metadata();
  to_json_file(profile, dir.toStdString() + "/" + PROFILE_FILE_NAME);
}

void remove_profile(QString name)
{
  QDir path(profile_dir(name));
  if (path.exists())
    path.removeRecursively();
}


}
