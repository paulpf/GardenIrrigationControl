#include <unity.h>

#include "../support/Arduino.h"
#include "../support/fakes/FakeMessagePublisher.h"

#include "../../src/domain/scheduled_irrigation.cpp"
#include "../../src/services/scheduledirrigationpublisher.cpp"
#include "../../src/services/scheduledirrigationcommandhandler.cpp"

static int savedZoneIndex = -1;
static int savedScheduleIndex = -1;
static IrrigationSchedule savedSchedule;
static int saveScheduleCount = 0;
static int clearedZoneIndex = -1;
static int clearedScheduleIndex = -1;
static int clearScheduleCount = 0;

void StorageManager::saveSchedule(int zoneIndex, int scheduleIndex,
                                  const IrrigationSchedule &schedule)
{
  savedZoneIndex = zoneIndex;
  savedScheduleIndex = scheduleIndex;
  savedSchedule = schedule;
  saveScheduleCount++;
}

void StorageManager::clearSchedule(int zoneIndex, int scheduleIndex)
{
  clearedZoneIndex = zoneIndex;
  clearedScheduleIndex = scheduleIndex;
  clearScheduleCount++;
}

void Trace::log(TraceLevel level, String message)
{
  (void)level;
  (void)message;
}

static void resetStorageCalls()
{
  savedZoneIndex = -1;
  savedScheduleIndex = -1;
  savedSchedule = IrrigationSchedule();
  saveScheduleCount = 0;
  clearedZoneIndex = -1;
  clearedScheduleIndex = -1;
  clearScheduleCount = 0;
}

static ScheduledIrrigationCommandHandler makeHandler(
    ScheduledIrrigation (&scheduledIrrigations)[MAX_IRRIGATION_ZONES],
    ScheduledIrrigationPublisher &publisher, IrrigationConfig &config)
{
  ScheduledIrrigationCommandHandler handler(scheduledIrrigations, publisher,
                                            config);
  handler.setup("device");
  return handler;
}

void setUp(void)
{
  arduinoFakeReset();
  resetStorageCalls();
}

void tearDown(void)
{
}

void test_json_set_creates_schedule_and_publishes_state(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigationZone1/schedule1/set",
      "{\"enabled\":true,\"startTime\":\"06:30\",\"durationMinutes\":15,\"daysOfWeek\":31}"));

  TEST_ASSERT_TRUE(scheduledIrrigations[0].isScheduleConfigured(0));
  TEST_ASSERT_EQUAL_INT(1, saveScheduleCount);
  TEST_ASSERT_EQUAL_INT(0, savedZoneIndex);
  TEST_ASSERT_EQUAL_INT(0, savedScheduleIndex);
  TEST_ASSERT_FALSE(messagePublisher.retainedEntries.empty());
}

void test_json_set_updates_start_time(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[1].setup(1);
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigationZone2/schedule2/set", "{\"startTime\":\"19:45\"}"));

  TEST_ASSERT_TRUE(scheduledIrrigations[1].isScheduleConfigured(1));
  TEST_ASSERT_EQUAL_INT(1, saveScheduleCount);
  TEST_ASSERT_EQUAL_INT(1, savedZoneIndex);
  TEST_ASSERT_EQUAL_INT(1, savedScheduleIndex);
  TEST_ASSERT_EQUAL_INT(19, savedSchedule.hour);
  TEST_ASSERT_EQUAL_INT(45, savedSchedule.minute);
}

void test_json_configured_false_clears_schedule(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].updateSchedule(0, IrrigationSchedule());
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigationZone1/schedule1/set", "{\"configured\":false}"));

  TEST_ASSERT_FALSE(scheduledIrrigations[0].isScheduleConfigured(0));
  TEST_ASSERT_EQUAL_INT(1, clearScheduleCount);
  TEST_ASSERT_EQUAL_INT(0, clearedZoneIndex);
  TEST_ASSERT_EQUAL_INT(0, clearedScheduleIndex);
}

void test_delete_topic_clears_schedule(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  scheduledIrrigations[0].updateSchedule(0, IrrigationSchedule());
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigationZone1/schedule1/delete/set", "true"));

  TEST_ASSERT_FALSE(scheduledIrrigations[0].isScheduleConfigured(0));
  TEST_ASSERT_EQUAL_INT(1, clearScheduleCount);
  TEST_ASSERT_EQUAL_INT(0, clearedZoneIndex);
  TEST_ASSERT_EQUAL_INT(0, clearedScheduleIndex);
  TEST_ASSERT_FALSE(messagePublisher.retainedEntries.empty());
}

void test_invalid_json_payload_is_rejected(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigationZone1/schedule1/set", "not-valid-json"));

  TEST_ASSERT_FALSE(scheduledIrrigations[0].isScheduleConfigured(0));
  TEST_ASSERT_EQUAL_INT(0, saveScheduleCount);
  TEST_ASSERT_TRUE(messagePublisher.retainedEntries.empty());
}

void test_empty_json_object_is_noop(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  scheduledIrrigations[0].setup(0);
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigationZone1/schedule1/set", "{}"));

  TEST_ASSERT_FALSE(scheduledIrrigations[0].isScheduleConfigured(0));
  TEST_ASSERT_EQUAL_INT(0, saveScheduleCount);
  TEST_ASSERT_EQUAL_INT(0, clearScheduleCount);
}

void test_unrelated_topic_is_not_handled(void)
{
  FakeMessagePublisher messagePublisher;
  ScheduledIrrigation scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher publisher(messagePublisher, scheduledIrrigations);
  publisher.setup("device");
  IrrigationConfig config;
  ScheduledIrrigationCommandHandler handler =
      makeHandler(scheduledIrrigations, publisher, config);

  TEST_ASSERT_FALSE(handler.handleCommand("device/other/topic", "true"));
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_json_set_creates_schedule_and_publishes_state);
  RUN_TEST(test_json_set_updates_start_time);
  RUN_TEST(test_json_configured_false_clears_schedule);
  RUN_TEST(test_delete_topic_clears_schedule);
  RUN_TEST(test_invalid_json_payload_is_rejected);
  RUN_TEST(test_empty_json_object_is_noop);
  RUN_TEST(test_unrelated_topic_is_not_handled);

  return UNITY_END();
}
