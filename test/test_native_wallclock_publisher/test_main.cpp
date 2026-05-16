#include <unity.h>

#include "../support/Arduino.h"
#include "../support/fakes/FakeMessagePublisher.h"

#include "../../src/services/wallclockpublisher.cpp"

class FakeWallClock : public IWallClock
{
public:
  bool valid = true;
  WallClockTime current;

  void setup() override
  {
  }

  bool isTimeValid() const override
  {
    return valid;
  }

  bool getLocalTime(WallClockTime &time) const override
  {
    if (!valid)
    {
      return false;
    }

    time = current;
    return true;
  }
};

void setUp(void)
{
}

void tearDown(void)
{
}

void test_publish_time_suppressed_when_mqtt_disconnected(void)
{
  FakeMessagePublisher publisher;
  publisher.connected = false;
  FakeWallClock clock;
  WallClockPublisher wallClockPublisher(publisher, clock);

  wallClockPublisher.setup("device");
  wallClockPublisher.publishTime();

  TEST_ASSERT_TRUE(publisher.entries.empty());
}

void test_publish_time_reports_invalid_time(void)
{
  FakeMessagePublisher publisher;
  FakeWallClock clock;
  clock.valid = false;
  WallClockPublisher wallClockPublisher(publisher, clock);

  wallClockPublisher.setup("device");
  wallClockPublisher.publishTime();

  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/system/time/valid"));
  TEST_ASSERT_EQUAL_STRING("false", publisher.entries[0].payload.c_str());
  TEST_ASSERT_EQUAL_INT(0, publisher.countTopic("device/system/time/local"));
}

void test_publish_time_reports_local_time_and_weekday(void)
{
  FakeMessagePublisher publisher;
  FakeWallClock clock;
  clock.current = {2026, 5, 18, 6, 30, 7, 0};
  WallClockPublisher wallClockPublisher(publisher, clock);

  wallClockPublisher.setup("device");
  wallClockPublisher.publishTime();

  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/system/time/valid"));
  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/system/time/local"));
  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/system/time/dayOfWeek"));
  TEST_ASSERT_EQUAL_STRING("true", publisher.entries[0].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("2026-05-18T06:30:07",
                           publisher.entries[1].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("0", publisher.entries[2].payload.c_str());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_publish_time_suppressed_when_mqtt_disconnected);
  RUN_TEST(test_publish_time_reports_invalid_time);
  RUN_TEST(test_publish_time_reports_local_time_and_weekday);

  return UNITY_END();
}
