#include <widgets/QTimeExtensions.h>
#include <core/util/time.h>

#include <core/util/time_extensions.h>
#include <core/util/custom_logger.h>

// \todo time zones

QDateTime fromTimePoint(std::chrono::time_point<std::chrono::high_resolution_clock> tp)
{
  using namespace date;

  auto daypoint = floor<days>(tp);
  auto ymd = year_month_day(daypoint);   // calendar date

  auto tt2 = std::chrono::duration_cast<std::chrono::milliseconds>(tp - daypoint);

  QDate qdate(static_cast<int>(ymd.year()),
              static_cast<unsigned int>(ymd.month()),
              static_cast<unsigned int>(ymd.day()));

  QTime qtime = QTime(0,0,0).addMSecs(tt2.count());
  return QDateTime(qdate, qtime, Qt::UTC);
}

std::chrono::time_point<std::chrono::high_resolution_clock> toTimePoint(QDateTime qdt)
{
  using namespace date;
  auto ret = date::sys_days(date::year{qdt.date().year()}/qdt.date().month()/qdt.date().day())
          + std::chrono::milliseconds(qdt.time().msecsSinceStartOfDay());
  return ret;
}
