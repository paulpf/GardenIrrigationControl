#include <unity.h>

#include "config.h"
#include "waterlevelconfig.h"

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
  enum class Type
  {
    LowWaterLockout,
    CriticalOverflowAlarm,
    Overflow,
    SafetyLock,
  };

  struct Event
  {
    Type type;
    bool active;
  };

  static const int MAX_EVENTS = 4;
  Event events[MAX_EVENTS];
  int count = 0;

  void add(Type type, bool active)
  {
    if (count < MAX_EVENTS)
    {
      events[count++] = {type, active};
    }
  }
};

static bool hasEvent(const TransitionEvents &events,
                     TransitionEvents::Type type, bool active)
{
  for (int i = 0; i < events.count; i++)
  {
    if (events.events[i].type == type && events.events[i].active == active)
    {
      return true;
    }
  }

  return false;
}

static TransitionEvents updateStateTransitions(WaterLevelState &state,
                                               const WaterLevelConfig &cfg)
{
  TransitionEvents events;

  const bool shouldEnableLowWaterLockout =
      state.percent < cfg.criticalPercent;
  const bool shouldReleaseLowWaterLockout =
      state.percent >= cfg.lockoutReleasePercent;

  if (!state.lowWaterLockoutActive && shouldEnableLowWaterLockout)
  {
    state.lowWaterLockoutActive = true;
    events.add(TransitionEvents::Type::LowWaterLockout,
               state.lowWaterLockoutActive);
  }
  else if (state.lowWaterLockoutActive && shouldReleaseLowWaterLockout)
  {
    state.lowWaterLockoutActive = false;
    events.add(TransitionEvents::Type::LowWaterLockout,
               state.lowWaterLockoutActive);
  }

  const bool shouldEnableCriticalOverflowAlarm =
      state.overflowLiters >= cfg.criticalOverflowBufferLiters;
  const bool shouldReleaseCriticalOverflowAlarm =
      state.overflowLiters <= cfg.criticalOverflowReleaseLiters;

  if (!state.criticalOverflowAlarmActive && shouldEnableCriticalOverflowAlarm)
  {
    state.criticalOverflowAlarmActive = true;
    events.add(TransitionEvents::Type::CriticalOverflowAlarm,
               state.criticalOverflowAlarmActive);
  }
  else if (state.criticalOverflowAlarmActive &&
           shouldReleaseCriticalOverflowAlarm)
  {
    state.criticalOverflowAlarmActive = false;
    events.add(TransitionEvents::Type::CriticalOverflowAlarm,
               state.criticalOverflowAlarmActive);
  }

  if (!state.overflowActive && state.percent > cfg.overflowPercent)
  {
    state.overflowActive = true;
    events.add(TransitionEvents::Type::Overflow, state.overflowActive);
  }
  else if (state.overflowActive &&
           state.percent <= cfg.overflowClearPercent)
  {
    state.overflowActive = false;
    events.add(TransitionEvents::Type::Overflow, state.overflowActive);
  }

  const bool shouldSafetyLock =
      state.lowWaterLockoutActive || state.criticalOverflowAlarmActive;
  if (state.safetyLockActive != shouldSafetyLock)
  {
    state.safetyLockActive = shouldSafetyLock;
    events.add(TransitionEvents::Type::SafetyLock, state.safetyLockActive);
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

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(2, events.count);
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::LowWaterLockout,
                            true));
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::SafetyLock, true));
}

void test_low_water_lockout_releases_at_release_threshold(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.safetyLockActive = true;
  state.percent = WATER_LEVEL_LOCKOUT_RELEASE_PERCENT;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_FALSE(state.lowWaterLockoutActive);
  TEST_ASSERT_FALSE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(2, events.count);
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::LowWaterLockout,
                            false));
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::SafetyLock,
                            false));
}

void test_low_water_hysteresis_no_flapping_inside_window(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.safetyLockActive = true;
  state.percent =
      (WATER_LEVEL_CRITICAL_PERCENT + WATER_LEVEL_LOCKOUT_RELEASE_PERCENT) /
      2.0f;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(0, events.count);
}

void test_critical_overflow_alarm_activates_at_buffer_limit(void)
{
  WaterLevelState state;
  state.percent = 50.0f;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_TRUE(state.criticalOverflowAlarmActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(2, events.count);
  TEST_ASSERT_TRUE(hasEvent(events,
                            TransitionEvents::Type::CriticalOverflowAlarm,
                            true));
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::SafetyLock, true));
}

void test_critical_overflow_alarm_releases_at_release_limit(void)
{
  WaterLevelState state;
  state.criticalOverflowAlarmActive = true;
  state.safetyLockActive = true;
  state.percent = 50.0f;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_RELEASE_LITERS;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_FALSE(state.criticalOverflowAlarmActive);
  TEST_ASSERT_FALSE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(2, events.count);
  TEST_ASSERT_TRUE(hasEvent(events,
                            TransitionEvents::Type::CriticalOverflowAlarm,
                            false));
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::SafetyLock,
                            false));
}

void test_overflow_visual_state_activates_above_100_percent(void)
{
  WaterLevelState state;
  state.percent = WATER_LEVEL_OVERFLOW_PERCENT + 0.1f;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_TRUE(state.overflowActive);
  TEST_ASSERT_EQUAL_INT(1, events.count);
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::Overflow, true));
}

void test_overflow_visual_state_clears_on_clear_threshold(void)
{
  WaterLevelState state;
  state.overflowActive = true;
  state.percent = WATER_LEVEL_OVERFLOW_CLEAR_PERCENT;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_FALSE(state.overflowActive);
  TEST_ASSERT_EQUAL_INT(1, events.count);
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::Overflow, false));
}

void test_no_event_when_state_does_not_change(void)
{
  WaterLevelState state;
  state.percent = 50.0f;
  state.overflowLiters = 0.0f;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_EQUAL_INT(0, events.count);
}

// Safety-Lock bleibt aktiv, wenn nur einer von zwei Gruenden wegfaellt
void test_safety_lock_remains_active_when_only_partial_condition_released(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.criticalOverflowAlarmActive = true;
  state.safetyLockActive = true;

  // Overflow-Alarm wird freigegeben, Low-Water bleibt aktiv
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_RELEASE_LITERS - 0.1f;
  state.percent = WATER_LEVEL_CRITICAL_PERCENT - 0.1f;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_FALSE(state.criticalOverflowAlarmActive);
  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(1, events.count);
  TEST_ASSERT_TRUE(hasEvent(events,
                            TransitionEvents::Type::CriticalOverflowAlarm,
                            false));
}

// Mehrere Transition-Events koennen im selben Zyklus ausloesen
void test_multiple_transition_events_fire_in_same_cycle(void)
{
  WaterLevelState state;

  // Low-Water UND kritischer Ueberlauf gleichzeitig
  state.percent = WATER_LEVEL_CRITICAL_PERCENT - 0.1f;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_EQUAL_INT(3, events.count);
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::LowWaterLockout,
                            true));
  TEST_ASSERT_TRUE(hasEvent(events,
                            TransitionEvents::Type::CriticalOverflowAlarm,
                            true));
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::SafetyLock, true));
  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.criticalOverflowAlarmActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
}

// Injizierte Config-Schwellwerte werden korrekt verwendet (TDD-Pruefung)
// Benutzt criticalPercent=20, lockoutRelease=25 statt der Produktionswerte 10/12.
// Mit percent=15: < 20 → Lockout aktiviert (mit altem Code: 15 < 10 = false → wuerde FEHLSCHLAGEN)
void test_custom_config_thresholds_are_respected(void)
{
  WaterLevelConfig cfg;
  cfg.criticalPercent = 20.0f;
  cfg.lockoutReleasePercent = 25.0f;

  WaterLevelState state;
  state.percent = 15.0f; // liegt zwischen 10% (Prod-Critical) und 20% (Custom-Critical)

  TransitionEvents events = updateStateTransitions(state, cfg);

  // Mit Custom-Config muss Lockout aktiv sein, weil 15 < 20
  TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
  TEST_ASSERT_TRUE(state.safetyLockActive);
  TEST_ASSERT_EQUAL_INT(2, events.count);
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::LowWaterLockout,
                            true));
  TEST_ASSERT_TRUE(hasEvent(events, TransitionEvents::Type::SafetyLock, true));
}

// Hysterese haelt den Lockout ueber mehrere aufeinanderfolgende Zyklen
void test_hysteresis_lockout_holds_across_multiple_cycles(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.safetyLockActive = true;

  // Wert im Hysteresebereich: oberhalb von Critical, unterhalb von Release
  state.percent = WATER_LEVEL_CRITICAL_PERCENT + 0.1f;

  for (int i = 0; i < 5; i++)
  {
    TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

    TEST_ASSERT_TRUE(state.lowWaterLockoutActive);
    TEST_ASSERT_TRUE(state.safetyLockActive);
    TEST_ASSERT_EQUAL_INT(0, events.count);
  }
}

void test_event_order_is_stable_in_multi_event_cycle(void)
{
  WaterLevelState state;
  state.percent = WATER_LEVEL_CRITICAL_PERCENT - 0.1f;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_EQUAL_INT(3, events.count);
  TEST_ASSERT_EQUAL_INT((int)TransitionEvents::Type::LowWaterLockout,
                        (int)events.events[0].type);
  TEST_ASSERT_EQUAL_INT((int)TransitionEvents::Type::CriticalOverflowAlarm,
                        (int)events.events[1].type);
  TEST_ASSERT_EQUAL_INT((int)TransitionEvents::Type::SafetyLock,
                        (int)events.events[2].type);
}

void test_no_duplicate_events_when_lock_states_remain_active(void)
{
  WaterLevelState state;
  state.lowWaterLockoutActive = true;
  state.criticalOverflowAlarmActive = true;
  state.safetyLockActive = true;
  state.percent = WATER_LEVEL_CRITICAL_PERCENT - 0.1f;
  state.overflowLiters = WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS + 10.0f;

  TransitionEvents events = updateStateTransitions(state, WaterLevelConfig{});

  TEST_ASSERT_EQUAL_INT(0, events.count);
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
  RUN_TEST(test_safety_lock_remains_active_when_only_partial_condition_released);
  RUN_TEST(test_multiple_transition_events_fire_in_same_cycle);
  RUN_TEST(test_hysteresis_lockout_holds_across_multiple_cycles);
  RUN_TEST(test_custom_config_thresholds_are_respected);
  RUN_TEST(test_event_order_is_stable_in_multi_event_cycle);
  RUN_TEST(test_no_duplicate_events_when_lock_states_remain_active);

  return UNITY_END();
}
