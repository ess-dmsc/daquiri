#include "QTimeExtensions.h"\

QDateTime fromBoostPtime(boost::posix_time::ptime bpt)
{
  std::string bpt_iso = boost::posix_time::to_iso_extended_string(bpt);
  std::replace(bpt_iso.begin(), bpt_iso.end(), '-', ' ');
  std::replace(bpt_iso.begin(), bpt_iso.end(), 'T', ' ');
  std::replace(bpt_iso.begin(), bpt_iso.end(), ':', ' ');
  std::replace(bpt_iso.begin(), bpt_iso.end(), '.', ' ');

  std::stringstream iss;
  iss.str(bpt_iso);

  int year, month, day, hour, minute, second;
  double ms = 0;
  iss >> year >> month >> day >> hour >> minute >> second >> ms;

  while (ms > 999)
    ms = ms / 10;
  ms = round(ms);

  QDate date;
  date.setDate(year, month, day);

  QTime time;
  time.setHMS(hour, minute, second, static_cast<int>(ms));

  QDateTime ret;
  ret.setDate(date);
  ret.setTime(time);

  return ret;
}

boost::posix_time::ptime fromQDateTime(QDateTime qdt)
{
  std::string dts = qdt.toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString();
  boost::posix_time::ptime bpt = boost::posix_time::time_from_string(dts);
  return bpt;
}
