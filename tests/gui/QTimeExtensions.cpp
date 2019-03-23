#include <QTest>
#include <gui/widgets/QTimeExtensions.h>
#include <core/util/time_extensions.h>
#include <core/util/custom_logger.h>
#include <date/date.h>

#include <QDebug>

// \todo time zones

class QTimeExtensions : public QObject {
Q_OBJECT
private slots:
  void initTestCase() {
    CustomLogger::initLogger(Log::Severity::Debug, nullptr, "gui_tests.log");
    qDebug("called before everything else");
  }

  void from_time_point() {
    hr_time_t t =
        date::sys_days(date::year{2011} / 12 / 13)
            + std::chrono::hours{14} + std::chrono::minutes{15} +
            std::chrono::seconds{16} + std::chrono::milliseconds{777};

    auto converted = fromTimePoint(t);
    QVERIFY(converted.date().year() == 2011);
    QVERIFY(converted.date().month() == 12);
    QVERIFY(converted.date().day() == 13);
    QVERIFY(converted.time().hour() == 14);
    QVERIFY(converted.time().minute() == 15);
    QVERIFY(converted.time().second() == 16);
    QVERIFY(converted.time().msec() == 777);
  }

  void to_time_point() {
    QDateTime qdt(QDate(2011,12,13), QTime(14,15,16,777));

    auto converted = toTimePoint(qdt);

    using namespace date;
    auto daypoint = floor<days>(converted);
    auto ymd = year_month_day(daypoint);   // calendar date
    auto tod = make_time(converted - daypoint); // Yields time_of_day type

    auto tod2 = std::chrono::duration_cast<std::chrono::milliseconds>(converted
        - date::floor<std::chrono::seconds>(converted)); // Yields time_of_day type

// Obtain individual components as integers
    auto y   = int(ymd.year());
    auto m   = unsigned(ymd.month());
    auto d   = unsigned(ymd.day());
    auto h   = tod.hours().count();
    auto min = tod.minutes().count();
    auto s   = tod.seconds().count();

    QVERIFY(y == 2011);
    QVERIFY(m == 12);
    QVERIFY(d == 13);
    QVERIFY(h == 14);
    QVERIFY(min == 15);
    QVERIFY(s == 16);
    QVERIFY(tod2.count() == 777);
  }

  void from_to_tp() {
    hr_time_t in = date::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
    QVERIFY(toTimePoint(fromTimePoint(in)) == in);
  }

  void to_from_tp() {
    QDateTime in = QDateTime::currentDateTimeUtc();
    QVERIFY(fromTimePoint(toTimePoint(in)) == in);
  }


};

QTEST_MAIN(QTimeExtensions)
#include "QTimeExtensions.moc"
