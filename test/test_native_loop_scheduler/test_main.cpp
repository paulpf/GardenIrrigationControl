#include <unity.h>

#include "../../src/loopscheduler.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_should_not_run_before_interval(void)
{
  LoopScheduler scheduler;
  unsigned long previous = 1000;

  const bool shouldRun = scheduler.shouldRun(1099, previous, 100);

  TEST_ASSERT_FALSE(shouldRun);
  TEST_ASSERT_EQUAL_UINT32(1000, previous);
}

void test_should_run_exactly_at_interval_boundary(void)
{
  LoopScheduler scheduler;
  unsigned long previous = 1000;

  const bool shouldRun = scheduler.shouldRun(1100, previous, 100);

  TEST_ASSERT_TRUE(shouldRun);
  TEST_ASSERT_EQUAL_UINT32(1100, previous);
}

void test_multiple_intervals_are_independent(void)
{
  LoopScheduler scheduler;
  unsigned long shortPrev = 1000;
  unsigned long longPrev = 1000;

  const bool shortRun = scheduler.shouldRun(1050, shortPrev, 50);
  const bool longRun = scheduler.shouldRun(1050, longPrev, 1000);

  TEST_ASSERT_TRUE(shortRun);
  TEST_ASSERT_FALSE(longRun);
  TEST_ASSERT_EQUAL_UINT32(1050, shortPrev);
  TEST_ASSERT_EQUAL_UINT32(1000, longPrev);
}

void test_overflow_safe_time_delta(void)
{
  LoopScheduler scheduler;
  unsigned long previous = 0xFFFFFFF0UL;

  const bool shouldRun = scheduler.shouldRun(0x00000014UL, previous, 20UL);

  TEST_ASSERT_TRUE(shouldRun);
  TEST_ASSERT_EQUAL_UINT32(0x00000014UL, previous);
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_should_not_run_before_interval);
  RUN_TEST(test_should_run_exactly_at_interval_boundary);
  RUN_TEST(test_multiple_intervals_are_independent);
  RUN_TEST(test_overflow_safe_time_delta);

  return UNITY_END();
}
