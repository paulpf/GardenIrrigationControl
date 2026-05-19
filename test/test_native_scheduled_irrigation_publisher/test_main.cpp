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

void test_publish_state_publishes_retained_state_topic(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].addSchedule(makeSchedule());
  ScheduledIrrigationPublisher schedulePublisher(publisher, scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  TEST_ASSERT_EQUAL_INT(
      1, publisher.countRetainedTopic("device/irrigationZone1/schedule1/state"));
}

void test_publish_state_reports_configured_schedule_values(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].addSchedule(makeSchedule());
  ScheduledIrrigationPublisher schedulePublisher(publisher, scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  std::string payload =
      publisher.retainedPayload("device/irrigationZone1/schedule1/state");
  TEST_ASSERT_TRUE(payload.find("\"configured\":true") != std::string::npos);
  TEST_ASSERT_TRUE(payload.find("\"enabled\":true") != std::string::npos);
  TEST_ASSERT_TRUE(payload.find("\"startTime\":\"06:30\"") != std::string::npos);
  TEST_ASSERT_TRUE(payload.find("\"daysOfWeek\":31") != std::string::npos);
  TEST_ASSERT_TRUE(payload.find("\"durationMinutes\":12") != std::string::npos);
}

void test_publish_state_reports_unconfigured_slot_as_not_configured(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher schedulePublisher(publisher, scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  std::string payload =
      publisher.retainedPayload("device/irrigationZone1/schedule1/state");
  TEST_ASSERT_TRUE(payload.find("\"configured\":false") != std::string::npos);
}

void test_publish_state_preserves_unconfigured_holes_between_slots(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].updateSchedule(1, makeSchedule()); // slot 2
  ScheduledIrrigationPublisher schedulePublisher(publisher, scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  std::string slot1 =
      publisher.retainedPayload("device/irrigationZone1/schedule1/state");
  std::string slot2 =
      publisher.retainedPayload("device/irrigationZone1/schedule2/state");

  TEST_ASSERT_TRUE(slot1.find("\"configured\":false") != std::string::npos);
  TEST_ASSERT_TRUE(slot2.find("\"configured\":true") != std::string::npos);
}

void test_publish_state_is_throttled_to_avoid_mqtt_spam(void)
{
  FakeMessagePublisher publisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher schedulePublisher(publisher, scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);
  const int firstPublishCount = (int)publisher.retainedEntries.size();
  TEST_ASSERT_TRUE(firstPublishCount > 0);

  schedulePublisher.publishState(2000);
  TEST_ASSERT_EQUAL_INT(firstPublishCount, (int)publisher.retainedEntries.size());

  schedulePublisher.publishState(61000);
  TEST_ASSERT_TRUE((int)publisher.retainedEntries.size() > firstPublishCount);
}

void test_publish_state_is_suppressed_when_mqtt_disconnected(void)
{
  FakeMessagePublisher publisher;
  publisher.connected = false;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher schedulePublisher(publisher, scheduledIrrigations);

  schedulePublisher.setup("device");
  schedulePublisher.publishState(1000);

  TEST_ASSERT_TRUE(publisher.entries.empty());
  TEST_ASSERT_TRUE(publisher.retainedEntries.empty());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_publish_state_publishes_retained_state_topic);
  RUN_TEST(test_publish_state_reports_configured_schedule_values);
  RUN_TEST(test_publish_state_reports_unconfigured_slot_as_not_configured);
  RUN_TEST(test_publish_state_preserves_unconfigured_holes_between_slots);
  RUN_TEST(test_publish_state_is_throttled_to_avoid_mqtt_spam);
  RUN_TEST(test_publish_state_is_suppressed_when_mqtt_disconnected);

  return UNITY_END();
}
