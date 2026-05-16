#include "Arduino.h"
#include "unity.h"

#include "../../src/domain/scheduled_irrigation.cpp"

void Trace::log(TraceLevel level, String message)
{
  (void)level;
  (void)message;
}

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

static IrrigationSchedule makeSchedule(int hour, int minute, int dayOfWeek)
{
  IrrigationSchedule schedule;
  schedule.enabled = true;
  schedule.hour = hour;
  schedule.minute = minute;
  schedule.durationMinutes = 7;
  schedule.daysOfWeek = static_cast<uint8_t>(1U << dayOfWeek);
  return schedule;
}

void setUp(void)
{
  arduinoFakeReset();
}

void tearDown(void)
{
}

void test_default_schedule_is_disabled(void)
{
  IrrigationSchedule schedule;
  TEST_ASSERT_FALSE(schedule.enabled);
}

void test_should_run_now_matches_enabled_schedule_time_and_day(void)
{
  FakeWallClock clock;
  clock.current = {2026, 5, 18, 6, 30, 0, 0}; // Monday

  ScheduledIrrigation scheduler;
  scheduler.setup(0, &clock);
  scheduler.addSchedule(makeSchedule(6, 30, 0));

  TEST_ASSERT_TRUE(scheduler.shouldRunNow());
}

void test_should_not_run_when_day_does_not_match(void)
{
  FakeWallClock clock;
  clock.current = {2026, 5, 19, 6, 30, 0, 1}; // Tuesday

  ScheduledIrrigation scheduler;
  scheduler.setup(0, &clock);
  scheduler.addSchedule(makeSchedule(6, 30, 0));

  TEST_ASSERT_FALSE(scheduler.shouldRunNow());
}

void test_should_not_run_without_valid_time(void)
{
  FakeWallClock clock;
  clock.valid = false;
  clock.current = {2026, 5, 18, 6, 30, 0, 0};

  ScheduledIrrigation scheduler;
  scheduler.setup(0, &clock);
  scheduler.addSchedule(makeSchedule(6, 30, 0));

  TEST_ASSERT_FALSE(scheduler.shouldRunNow());
}

void test_disabled_schedule_does_not_run(void)
{
  FakeWallClock clock;
  clock.current = {2026, 5, 18, 6, 30, 0, 0};

  IrrigationSchedule schedule = makeSchedule(6, 30, 0);
  schedule.enabled = false;

  ScheduledIrrigation scheduler;
  scheduler.setup(0, &clock);
  scheduler.addSchedule(schedule);

  TEST_ASSERT_FALSE(scheduler.shouldRunNow());
}

void test_consume_due_schedule_returns_schedule_once_per_wall_clock_minute(void)
{
  FakeWallClock clock;
  clock.current = {2026, 5, 18, 6, 30, 0, 0};

  ScheduledIrrigation scheduler;
  scheduler.setup(0, &clock);
  scheduler.addSchedule(makeSchedule(6, 30, 0));

  IrrigationSchedule dueSchedule;
  TEST_ASSERT_TRUE(scheduler.consumeDueSchedule(1000, dueSchedule));
  TEST_ASSERT_EQUAL_INT(7, dueSchedule.durationMinutes);
  TEST_ASSERT_FALSE(scheduler.consumeDueSchedule(2000, dueSchedule));
}

void test_legacy_should_run_now_signature_uses_wall_clock(void)
{
  FakeWallClock clock;
  clock.current = {2026, 5, 18, 6, 30, 0, 0};

  ScheduledIrrigation scheduler;
  scheduler.setup(0, &clock);
  scheduler.addSchedule(makeSchedule(6, 30, 0));

  TEST_ASSERT_TRUE(scheduler.shouldRunNow(123456));
}

void test_scheduler_without_wall_clock_never_runs(void)
{
  ScheduledIrrigation scheduler;
  scheduler.setup(0);
  scheduler.addSchedule(makeSchedule(6, 30, 0));

  TEST_ASSERT_FALSE(scheduler.shouldRunNow());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_default_schedule_is_disabled);
  RUN_TEST(test_should_run_now_matches_enabled_schedule_time_and_day);
  RUN_TEST(test_should_not_run_when_day_does_not_match);
  RUN_TEST(test_should_not_run_without_valid_time);
  RUN_TEST(test_disabled_schedule_does_not_run);
  RUN_TEST(test_consume_due_schedule_returns_schedule_once_per_wall_clock_minute);
  RUN_TEST(test_legacy_should_run_now_signature_uses_wall_clock);
  RUN_TEST(test_scheduler_without_wall_clock_never_runs);

  return UNITY_END();
}
