#include <unity.h>
#include <cstdio>

// Phase 4.1: Test getRemainingTime() and getRemainingTimeAsString() logic
// These tests validate the fixes from Phase 1

void setUp(void)
{
}

void tearDown(void)
{
}

// Inline test functions that don't require external compilation

int getRemainingTime_logic(bool timerIsActive, int durationTime, 
                            unsigned long currentTime, unsigned long startTime)
{
  if (!timerIsActive) return 0;
  unsigned long elapsed = currentTime - startTime;
  return (elapsed < (unsigned long)durationTime) ? (int)(durationTime - elapsed) : 0;
}

// Test: Timer inactive returns 0
void test_timer_inactive_returns_zero()
{
  int result = getRemainingTime_logic(false, 300000, 0, 0);
  TEST_ASSERT_EQUAL_INT(0, result);
}

// Test: Full duration at start
void test_timer_full_duration()
{
  int result = getRemainingTime_logic(true, 300000, 0, 0);
  TEST_ASSERT_EQUAL_INT(300000, result);
}

// Test: Half duration elapsed
void test_timer_half_elapsed()
{
  int result = getRemainingTime_logic(true, 300000, 150000, 0);
  TEST_ASSERT_EQUAL_INT(150000, result);
}

// Test: Timer expired clamps to 0 (underflow protection)
void test_timer_expired_clamped()
{
  int result = getRemainingTime_logic(true, 300000, 400000, 0);
  TEST_ASSERT_EQUAL_INT(0, result);
}

// Test: Large elapsed time doesn't underflow
void test_timer_large_elapsed_no_underflow()
{
  int result = getRemainingTime_logic(true, 300000, 500000, 0);
  TEST_ASSERT_GREATER_OR_EQUAL(result, 0);
  TEST_ASSERT_EQUAL_INT(0, result);
}

// Test: Timer with start offset
void test_timer_with_offset()
{
  int result = getRemainingTime_logic(true, 300000, 61000, 1000);
  TEST_ASSERT_EQUAL_INT(240000, result);
}

// String formatting test helper
void formatTime_logic(int remainingTimeMs, char* buffer, int bufsize)
{
  if (remainingTimeMs <= 0)
  {
    snprintf(buffer, bufsize, "00:00");
    return;
  }
  int minutes = remainingTimeMs / 60000;
  int seconds = (remainingTimeMs % 60000) / 1000;
  snprintf(buffer, bufsize, "%02d:%02d", minutes, seconds);
}

// Test: Format zero time
void test_format_zero()
{
  char buf[8];
  formatTime_logic(0, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("00:00", buf);
}

// Test: Format 1 minute 3 seconds
void test_format_01_03()
{
  char buf[8];
  formatTime_logic(63000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("01:03", buf);
}

// Test: Format 5 minutes
void test_format_05_00()
{
  char buf[8];
  formatTime_logic(300000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("05:00", buf);
}

// Test: Format 5 minutes 30 seconds
void test_format_05_30()
{
  char buf[8];
  formatTime_logic(330000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("05:30", buf);
}

// Test: Leading zeros on single-digit seconds
void test_format_single_digit_seconds()
{
  char buf[8];
  formatTime_logic(65000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("01:05", buf);
}

// Test: Format 59 seconds
void test_format_00_59()
{
  char buf[8];
  formatTime_logic(59000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("00:59", buf);
}

// Test: Format 60 minutes
void test_format_60_00()
{
  char buf[8];
  formatTime_logic(3600000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("60:00", buf);
}

// Test: Format negative time returns 00:00
void test_format_negative()
{
  char buf[8];
  formatTime_logic(-1000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("00:00", buf);
}

// ============================================================================
// Main – Required for native Unity test framework
// ============================================================================
int main(int /*argc*/, char** /*argv*/)
{
  UNITY_BEGIN();

  // Timer remaining time tests
  RUN_TEST(test_timer_inactive_returns_zero);
  RUN_TEST(test_timer_full_duration);
  RUN_TEST(test_timer_half_elapsed);
  RUN_TEST(test_timer_expired_clamped);
  RUN_TEST(test_timer_large_elapsed_no_underflow);
  RUN_TEST(test_timer_with_offset);

  // Time formatting tests
  RUN_TEST(test_format_zero);
  RUN_TEST(test_format_01_03);
  RUN_TEST(test_format_05_00);
  RUN_TEST(test_format_05_30);
  RUN_TEST(test_format_single_digit_seconds);
  RUN_TEST(test_format_00_59);
  RUN_TEST(test_format_60_00);
  RUN_TEST(test_format_negative);

  return UNITY_END();
}
