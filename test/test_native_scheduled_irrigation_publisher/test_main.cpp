#include <unity.h>

#include "../support/Arduino.h"
#include "../support/fakes/FakeMessagePublisher.h"

#include "../../src/domain/scheduled_irrigation.cpp"
#include "../../src/services/scheduledirrigationpublisher.cpp"

void Trace::log(TraceLevel level, String message)
{
  (void)level;
  (void)message;
}

static IrrigationSchedule makeSchedule()
{
  IrrigationSchedule schedule;
  schedule.enabled = true;
  schedule.hour = 6;
  schedule.minute = 30;
  schedule.daysOfWeek = 31;
  schedule.durationMinutes = 12;
  return schedule;
}

void setUp(void)
{
  arduinoFakeReset();
}

void tearDown(void)
{
}

void test_publish_state_creates_iobroker_friendly_schedule_topics(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].addSchedule(makeSchedule());
  ScheduledIrrigationPublisher schedulePublisher(publisher,
                                                 scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigationZone1/schedule1/configured"));
  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigationZone1/schedule1/enabled"));
  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigationZone1/schedule1/hour"));
  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigationZone1/schedule1/minute"));
  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigationZone1/schedule1/daysOfWeek"));
  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic(
                               "device/irrigationZone1/schedule1/"
                               "durationMinutes"));
}

void test_publish_state_reports_configured_schedule_values(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].addSchedule(makeSchedule());
  ScheduledIrrigationPublisher schedulePublisher(publisher,
                                                 scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  TEST_ASSERT_EQUAL_STRING("true", publisher.entries[0].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("true", publisher.entries[1].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("6", publisher.entries[2].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("30", publisher.entries[3].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("31", publisher.entries[4].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("12", publisher.entries[5].payload.c_str());
}

void test_publish_state_reports_unconfigured_schedule_slots_as_defaults(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher schedulePublisher(publisher,
                                                 scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  TEST_ASSERT_EQUAL_STRING("false", publisher.entries[0].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("false", publisher.entries[1].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("6", publisher.entries[2].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("0", publisher.entries[3].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("62", publisher.entries[4].payload.c_str());
  TEST_ASSERT_EQUAL_STRING("5", publisher.entries[5].payload.c_str());
}

void test_publish_state_is_throttled_to_avoid_mqtt_spam(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher schedulePublisher(publisher,
                                                 scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);
  const int firstPublishCount = (int)publisher.entries.size();
  schedulePublisher.publishState(2000);
  TEST_ASSERT_EQUAL_INT(firstPublishCount, (int)publisher.entries.size());

  schedulePublisher.publishState(61000);
  TEST_ASSERT_EQUAL_INT(firstPublishCount * 2, (int)publisher.entries.size());
}

void test_publish_state_is_suppressed_when_mqtt_disconnected(void)
{
  FakeMessagePublisher publisher;
  publisher.connected = false;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher schedulePublisher(publisher,
                                                 scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  TEST_ASSERT_TRUE(publisher.entries.empty());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_publish_state_creates_iobroker_friendly_schedule_topics);
  RUN_TEST(test_publish_state_reports_configured_schedule_values);
  RUN_TEST(test_publish_state_reports_unconfigured_schedule_slots_as_defaults);
  RUN_TEST(test_publish_state_is_throttled_to_avoid_mqtt_spam);
  RUN_TEST(test_publish_state_is_suppressed_when_mqtt_disconnected);

  return UNITY_END();
}
