#include <unity.h>

#include "../support/fakes/FakeMessagePublisher.h"
#include "../support/fakes/FakeWaterLevelSensor.h"

#include "../../src/waterlevelmanager.cpp"

bool IrrigationZone::_globalStartInhibit = false;

void IrrigationZone::setGlobalStartInhibit(bool inhibit)
{
  _globalStartInhibit = inhibit;
}

bool IrrigationZone::isGlobalStartInhibited()
{
  return _globalStartInhibit;
}

void Trace::log(TraceLevel, String)
{
}

void Trace::plotBoolState(String, bool, int)
{
}

void Trace::plotLoopTime(String, int, unsigned long)
{
}

void setUp(void)
{
  IrrigationZone::setGlobalStartInhibit(false);
}

void tearDown(void)
{
}

void test_bool_state_topics_publish_only_on_real_state_changes(void)
{
  WaterLevelConfig config;
  config.adcMin = 0;
  config.adcMax = 100;
  config.capacityLiters = 100.0f;
  config.criticalPercent = 10.0f;
  config.lockoutReleasePercent = 12.0f;
  config.readIntervalMs = 1;

  FakeMessagePublisher publisher;
  FakeWaterLevelSensor sensor;
  sensor.rawValue = 5;

  WaterLevelManager manager(publisher, sensor, config);
  manager.setup("device");

  manager.loop(1);
  manager.loop(2);

  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/waterlevel/lockout"));
  TEST_ASSERT_EQUAL_INT(1,
                        publisher.countTopic("device/waterlevel/safety_lock"));
  TEST_ASSERT_EQUAL_INT(2, publisher.countTopic("device/waterlevel/percent"));
  TEST_ASSERT_EQUAL_INT(2, publisher.countTopic("device/waterlevel/status"));
}

void test_critical_alarm_and_overflow_topics_do_not_duplicate_without_changes(void)
{
  WaterLevelConfig config;
  config.adcMin = 0;
  config.adcMax = 100;
  config.capacityLiters = 100.0f;
  config.overflowPercent = 100.0f;
  config.overflowClearPercent = 100.0f;
  config.criticalOverflowBufferLiters = 10.0f;
  config.criticalOverflowReleaseLiters = 5.0f;
  config.readIntervalMs = 1;

  FakeMessagePublisher publisher;
  FakeWaterLevelSensor sensor;
  sensor.rawValue = 120;

  WaterLevelManager manager(publisher, sensor, config);
  manager.setup("device");

  manager.loop(1);
  manager.loop(2);

  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/waterlevel/critical_high_alarm"));
  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/waterlevel/overflow"));
  TEST_ASSERT_EQUAL_INT(1,
                        publisher.countTopic("device/waterlevel/safety_lock"));
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_bool_state_topics_publish_only_on_real_state_changes);
  RUN_TEST(test_critical_alarm_and_overflow_topics_do_not_duplicate_without_changes);

  return UNITY_END();
}
