#pragma once

#include <QString>
#include "json.hpp"

namespace Profiles
{

QString settings_dir();
void select_settings_dir(QString dir);

QString profiles_dir();
QString current_profile_name();

QString profile_dir(QString name);
QString current_profile_dir();

nlohmann::json get_profile(QString name);
nlohmann::json get_current_profile();

void select_profile(QString name, bool boot);
void save_profile(const nlohmann::json&);

void create_profile(QString name);
void remove_profile(QString name);
bool profile_exists(QString name);

}
