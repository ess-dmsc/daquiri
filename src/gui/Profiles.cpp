#include "Profiles.h"
#include <QSettings>
#include <QDir>
#include "json_file.h"

namespace Profiles
{

QString settings_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return settings.value("settings_directory",
                        QDir::homePath() + "/daquiri/settings").toString();
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
  auto dir = Profiles::profiles_dir() + "/" + name;
  nlohmann::json profile;
  if (!dir.isEmpty())
  {
    try
    {
      profile = from_json_file(dir.toStdString() + "/profile.set");
    }
    catch(...) {}
  }
  return profile;
}

nlohmann::json current_profile()
{
  return get_profile(current_profile_name());
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

void save_profile(const nlohmann::json& data)
{
  auto dir = current_profile_dir();
  if (!dir.isEmpty())
    to_json_file(data, dir.toStdString() + "/profile.set");
}

void select_profile(QString name, bool boot)
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("current_profile", name);
  settings.setValue("boot_on_startup", boot);
}

}
