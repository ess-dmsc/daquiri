#pragma once

#include <QDateTime>
#include <boost/date_time.hpp>

QDateTime fromBoostPtime(boost::posix_time::ptime);

boost::posix_time::ptime fromQDateTime(QDateTime);
