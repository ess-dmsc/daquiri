#pragma once

#include <QString>
#include <nlohmann/json.hpp>

class Profiles
{
 public:

  static Profiles& singleton()
  {
    static Profiles singleton_instance;
    return singleton_instance;
  }

  static bool has_settings_dir();
  static QString settings_dir();
  static QString default_settings_dir();
  static void select_settings_dir(QString dir);
  static QString profiles_dir();
  static bool is_valid_settings_dir(QString dir);

  void select_profile(QString name, bool boot);
  void auto_boot(bool boot);

  bool auto_boot() const;
  QString current_profile_name();
  QString current_profile_dir();
  nlohmann::json get_current_profile();
  void save_profile(const nlohmann::json&);

  static QString profile_dir(QString name);
  static nlohmann::json get_profile(QString name);

  static void create_profile(QString name);
  static void remove_profile(QString name);
  static bool profile_exists(QString name);
  static bool is_valid_profile(QString name);

 private:
  QString current_profile_name_;
  bool auto_boot_{false};

  //singleton assurance
  Profiles() = default;
  Profiles(Profiles const&);
  void operator=(Profiles const&);
};
