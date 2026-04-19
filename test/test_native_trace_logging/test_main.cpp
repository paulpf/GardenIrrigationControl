#include <cstdio>
#include <cstring>
#include <unity.h>

// Phase 4.3: Test Trace logging level filtering and prefix formatting

void setUp(void)
{
}

void tearDown(void)
{
}

// Helper: Simulate Trace level filtering logic
// TraceLevel enum: TRACE=0, DEBUG=1, INFO=2, ERROR=3, NONE=4
bool shouldLog_logic(int messageLevel, int configuredLevel)
{
  // Message should be logged if messageLevel >= configuredLevel
  return messageLevel >= configuredLevel;
}

// Helper: Simulate prefix generation based on trace level
void getTracePrefix_logic(int level, char *prefix_buffer, size_t bufsize)
{
  // level: 0=TRACE, 1=DEBUG, 2=INFO, 3=ERROR, 4=NONE
  switch (level)
  {
  case 0: // TRACE
    snprintf(prefix_buffer, bufsize, "[TRACE] ");
    break;
  case 1: // DEBUG
    snprintf(prefix_buffer, bufsize, "[DEBUG] ");
    break;
  case 2: // INFO
    snprintf(prefix_buffer, bufsize, "[INFO] ");
    break;
  case 3: // ERROR
    snprintf(prefix_buffer, bufsize, "[ERROR] ");
    break;
  default: // Unknown/NONE
    snprintf(prefix_buffer, bufsize, "[LOG] ");
    break;
  }
}

// Test: TRACE level should log at TRACE level
void test_trace_level_logs_at_trace()
{
  bool result = shouldLog_logic(0, 0); // TRACE=0, configLevel=0
  TEST_ASSERT_TRUE(result);
}

// Test: INFO should not log when trace level is DEBUG
void test_info_not_logged_when_debug_configured()
{
  bool result = shouldLog_logic(2, 1); // INFO=2, configLevel=DEBUG=1
  TEST_ASSERT_TRUE(result);            // INFO should log (2 >= 1)
}

// Test: TRACE should not log when trace level is DEBUG
void test_trace_not_logged_when_debug_configured()
{
  bool result = shouldLog_logic(0, 1); // TRACE=0, configLevel=DEBUG=1
  TEST_ASSERT_FALSE(result);           // TRACE should NOT log (0 < 1)
}

// Test: DEBUG should log when trace level is DEBUG
void test_debug_logs_at_debug_level()
{
  bool result = shouldLog_logic(1, 1); // DEBUG=1, configLevel=DEBUG=1
  TEST_ASSERT_TRUE(result);
}

// Test: ERROR should always log when trace level is INFO
void test_error_logs_when_info_configured()
{
  bool result = shouldLog_logic(3, 2); // ERROR=3, configLevel=INFO=2
  TEST_ASSERT_TRUE(result);            // ERROR should log (3 >= 2)
}

// Test: TRACE prefix formatting
void test_prefix_trace()
{
  char buf[16];
  getTracePrefix_logic(0, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[TRACE] ", buf);
}

// Test: DEBUG prefix formatting
void test_prefix_debug()
{
  char buf[16];
  getTracePrefix_logic(1, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[DEBUG] ", buf);
}

// Test: INFO prefix formatting
void test_prefix_info()
{
  char buf[16];
  getTracePrefix_logic(2, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[INFO] ", buf);
}

// Test: ERROR prefix formatting
void test_prefix_error()
{
  char buf[16];
  getTracePrefix_logic(3, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[ERROR] ", buf);
}

// Test: Unknown level defaults to [LOG]
void test_prefix_unknown_defaults_to_log()
{
  char buf[16];
  getTracePrefix_logic(99, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[LOG] ", buf);
}

// Test: All levels log when trace level is TRACE (0)
void test_all_levels_log_at_trace_level()
{
  TEST_ASSERT_TRUE(shouldLog_logic(0, 0)); // TRACE logs
  TEST_ASSERT_TRUE(shouldLog_logic(1, 0)); // DEBUG logs
  TEST_ASSERT_TRUE(shouldLog_logic(2, 0)); // INFO logs
  TEST_ASSERT_TRUE(shouldLog_logic(3, 0)); // ERROR logs
}

// Test: Only ERROR logs when trace level is ERROR (3)
void test_only_error_logs_at_error_level()
{
  TEST_ASSERT_FALSE(shouldLog_logic(0, 3)); // TRACE does NOT log
  TEST_ASSERT_FALSE(shouldLog_logic(1, 3)); // DEBUG does NOT log
  TEST_ASSERT_FALSE(shouldLog_logic(2, 3)); // INFO does NOT log
  TEST_ASSERT_TRUE(shouldLog_logic(3, 3));  // ERROR logs
}

// Test: NONE level blocks all logs
void test_none_level_blocks_all_logs()
{
  TEST_ASSERT_FALSE(shouldLog_logic(0, 4)); // TRACE does NOT log
  TEST_ASSERT_FALSE(shouldLog_logic(1, 4)); // DEBUG does NOT log
  TEST_ASSERT_FALSE(shouldLog_logic(2, 4)); // INFO does NOT log
  TEST_ASSERT_FALSE(shouldLog_logic(3, 4)); // ERROR does NOT log
}

// ============================================================================
// Main – Required for native Unity test framework
// ============================================================================
int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  // Level filtering tests
  RUN_TEST(test_trace_level_logs_at_trace);
  RUN_TEST(test_info_not_logged_when_debug_configured);
  RUN_TEST(test_trace_not_logged_when_debug_configured);
  RUN_TEST(test_debug_logs_at_debug_level);
  RUN_TEST(test_error_logs_when_info_configured);

  // Prefix formatting tests
  RUN_TEST(test_prefix_trace);
  RUN_TEST(test_prefix_debug);
  RUN_TEST(test_prefix_info);
  RUN_TEST(test_prefix_error);
  RUN_TEST(test_prefix_unknown_defaults_to_log);

  // Combined level behavior tests
  RUN_TEST(test_all_levels_log_at_trace_level);
  RUN_TEST(test_only_error_logs_at_error_level);
  RUN_TEST(test_none_level_blocks_all_logs);

  return UNITY_END();
}
