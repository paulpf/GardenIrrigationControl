#include <unity.h>

#include "config.h"

struct WaterLevelState
{
  float percent = 0.0f;
  float overflowLiters = 0.0f;
  bool lowWaterLockoutActive = false;
  bool criticalOverflowAlarmActive = false;
  bool overflowActive = false;
  bool safetyLockActive = false;
};

struct TransitionEvents
{
  bool lowWaterLockoutChanged = false;
  bool criticalOverflowAlarmChanged = false;
  bool overflowChanged = false;
  bool safetyLockChanged = false;
};

static TransitionEvents updateStateTransitions(WaterLevelState &state)
{
  TransitionEvents events;

  const bool shouldEnableLowWaterLockout =
      state.percent < WATER_LEVEL_CRITICAL_PERCENT;
  const bool shouldReleaseLowWaterLockout =
      state.percent >= WATER_LEVEL_LOCKOUT_RELEASE_PERCENT;

  if (!state.lowWaterLockoutActive && shouldEnableLowWaterLockout)
  {
    state.lowWaterLockoutActive = true;
    events.lowWaterLockoutChanged = true;
  }
  else if (state.lowWaterLockoutActive && shouldReleaseLowWaterLockout)
  {
    state.lowWaterLockoutActive = false;
    events.lowWaterLockoutChanged = true;
  }

  const bool shouldEnableCriticalOverflowAlarm =
      state.overflowLiters >= WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;
  const bool shouldReleaseCriticalOverflowAlarm =
      state.overflowLiters <= WATER_LEVEL_CRITICAL_OVERFLOW_RELEASE_LITERS;

  if (!state.criticalOverflowAlarmActive && shouldEnableCriticalOverflowAlarm)
  {
    state.criticalOverflowAlarmActive = true;
    events.criticalOverflowAlarmChanged = true;
  }
  else if (state.criticalOverflowAlarmActive &&
           shouldReleaseCriticalOverflowAlarm)
  {
    state.criticalOverflowAlarmActive = false;
    events.criticalOverflowAlarmChanged = true;
  }

  if (!state.overflowActive && state.percent > WATER_LEVEL_OVERFLOW_PERCENT)
  {
    state.overflowActive = true;
    events.overflowChanged = true;
  }
  else if (state.overflowActive &&
           state.percent <= WATER_LEVEL_OVERFLOW_CLEAR_PERCENT)
  {
    state.overflowActive = false;
    events.overflowChanged = true;
  }

  const bool shouldSafetyLock =
      state.lowWaterLockoutActive || state.criticalOverflowAlarmActive;
  if (state.safetyLockActive != shouldSafetyLock)
  {
    state.safetyLockActive = shouldSafetyLock;
    events.safetyLockChanged = true;
  }

  return events;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_low_water_lockout_activates_below_threshold(void)
{
  WaterLevelState state;
  state.percent = WATER_LEVEL_CRITICAL_PERCENT - 0.1f;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_TRUE(events.lowWaterLockoutChanged);
  TEST_ASSERT_TRUE(events.safetyLockChanged);
}

void test_low_water_lockout_releases_at_release_threshold(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.safetyLockActive = true;
  state.percent = WATER_LEVEL_LOCKOUT_RELEASE_PERCENT;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_FALSE(state.lowWaterLockoutActive);
  TEST_ASSERT_FALSE(state.safetyLockActive);
  TEST_ASSERT_TRUE(events.lowWaterLockoutChanged);
  TEST_ASSERT_TRUE(events.safetyLockChanged);
}

void test_low_water_hysteresis_no_flapping_inside_window(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.safetyLockActive = true;
  state.percent =
      (WATER_LEVEL_CRITICAL_PERCENT + WATER_LEVEL_LOCKOUT_RELEASE_PERCENT) /
      2.0f;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_FALSE(events.lowWaterLockoutChanged);
  TEST_ASSERT_FALSE(events.safetyLockChanged);
}

void test_critical_overflow_alarm_activates_at_buffer_limit(void)
{
  WaterLevelState state;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_TRUE(state.criticalOverflowAlarmActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_TRUE(events.criticalOverflowAlarmChanged);
  TEST_ASSERT_TRUE(events.safetyLockChanged);
}

void test_critical_overflow_alarm_releases_at_release_limit(void)
{
  WaterLevelState state;
  state.criticalOverflowAlarmActive = true;
  state.safetyLockActive = true;
  state.percent = 50.0f;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_RELEASE_LITERS;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_FALSE(state.criticalOverflowAlarmActive);
  TEST_ASSERT_FALSE(state.safetyLockActive);
  TEST_ASSERT_TRUE(events.criticalOverflowAlarmChanged);
  TEST_ASSERT_TRUE(events.safetyLockChanged);
}

void test_overflow_visual_state_activates_above_100_percent(void)
{
  WaterLevelState state;
  state.percent = WATER_LEVEL_OVERFLOW_PERCENT + 0.1f;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_TRUE(state.overflowActive);
  TEST_ASSERT_TRUE(events.overflowChanged);
}

void test_overflow_visual_state_clears_on_clear_threshold(void)
{
  WaterLevelState state;
  state.overflowActive = true;
  state.percent = WATER_LEVEL_OVERFLOW_CLEAR_PERCENT;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_FALSE(state.overflowActive);
  TEST_ASSERT_TRUE(events.overflowChanged);
}

void test_no_event_when_state_does_not_change(void)
{
  WaterLevelState state;
  state.percent = 50.0f;
  state.overflowLiters = 0.0f;

  TransitionEvents events = updateStateTransitions(state);

  TEST_ASSERT_FALSE(events.lowWaterLockoutChanged);
  TEST_ASSERT_FALSE(events.criticalOverflowAlarmChanged);
  TEST_ASSERT_FALSE(events.overflowChanged);
  TEST_ASSERT_FALSE(events.safetyLockChanged);
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_low_water_lockout_activates_below_threshold);
  RUN_TEST(test_low_water_lockout_releases_at_release_threshold);
  RUN_TEST(test_low_water_hysteresis_no_flapping_inside_window);
  RUN_TEST(test_critical_overflow_alarm_activates_at_buffer_limit);
  RUN_TEST(test_critical_overflow_alarm_releases_at_release_limit);
  RUN_TEST(test_overflow_visual_state_activates_above_100_percent);
  RUN_TEST(test_overflow_visual_state_clears_on_clear_threshold);
  RUN_TEST(test_no_event_when_state_does_not_change);

  return UNITY_END();
}
