// test/test_native_duration_formatter/test_main.cpp
//
// Commit 1 – Smoke tests.
// These tests validate the test harness itself and the Arduino String stub.
// No production logic is imported yet (nothing in lib/domain).
// They will be enriched in subsequent commits.

#include <unity.h>
#include "../support/ArduinoFake.h"

void setUp(void) {}
void tearDown(void) {}

// ---------------------------------------------------------------------------
// Helper: replicate the duration->MM:SS logic from IrrigationZone::getRemainingTimeAsString()
// verbatim from the current production code so we can test it in isolation.
// NOTE: this is a COPY – intentionally not linked to the real class.
// Commit 2 will extract the real function into lib/domain and remove this copy.
// ---------------------------------------------------------------------------
static String formatRemainingTime_CurrentImpl(int remainingTimeMs) {
    if (remainingTimeMs <= 0) return "00:00";

    int minutes = remainingTimeMs / 60000;
    int seconds = (remainingTimeMs % 60000) / 1000;

    String formattedTime = String(minutes) + ":" + String(seconds);

    // Current (buggy) code path: appends "0" instead of prepending
    if (seconds < 10) {
        formattedTime += "0";
    }
    return formattedTime;
}

// ---------------------------------------------------------------------------
// Helper: the CORRECT implementation (will replace the above in Commit 3)
// ---------------------------------------------------------------------------
static String formatRemainingTime_Fixed(int remainingTimeMs) {
    if (remainingTimeMs <= 0) return "00:00";

    int minutes = remainingTimeMs / 60000;
    int seconds = (remainingTimeMs % 60000) / 1000;

    char buf[8];
    snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
    return String(buf);
}

// ---------------------------------------------------------------------------
// Tests – Harness smoke
// ---------------------------------------------------------------------------

void test_arduino_string_basic() {
    String s("hello");
    TEST_ASSERT_EQUAL_STRING("hello", s.c_str());
}

void test_arduino_string_int_conversion() {
    String s(42);
    TEST_ASSERT_EQUAL_STRING("42", s.c_str());
}

void test_arduino_string_concat() {
    String a("foo");
    String b("bar");
    String c = a + ":" + b;
    TEST_ASSERT_EQUAL_STRING("foo:bar", c.c_str());
}

// ---------------------------------------------------------------------------
// Tests – Duration formatting (documents current vs. correct behaviour)
// ---------------------------------------------------------------------------

// ---- Expired timer --------------------------------------------------------
void test_format_zero_returns_zeros() {
    TEST_ASSERT_EQUAL_STRING("00:00", formatRemainingTime_Fixed(0).c_str());
}

void test_format_negative_returns_zeros() {
    TEST_ASSERT_EQUAL_STRING("00:00", formatRemainingTime_Fixed(-1).c_str());
}

// ---- Double-digit seconds (no bug here, both impls agree) -----------------
void test_format_double_digit_seconds() {
    // 1 min 30 sec = 90 000 ms
    TEST_ASSERT_EQUAL_STRING("1:30", formatRemainingTime_Fixed(90000).c_str());
}

void test_format_exactly_one_minute() {
    TEST_ASSERT_EQUAL_STRING("1:00", formatRemainingTime_Fixed(60000).c_str());
}

// ---- Single-digit seconds (BUG in current impl) ---------------------------
// Documents that the current implementation produces "5:30" instead of "5:03"
void test_current_impl_has_seconds_suffix_bug() {
    // 5 min 3 sec = 303 000 ms
    String result = formatRemainingTime_CurrentImpl(303000);
    // Current code appends "0" -> produces "5:30" (wrong)
    TEST_ASSERT_EQUAL_STRING("5:30", result.c_str());  // documents the bug
}

void test_fixed_impl_single_digit_seconds() {
    // Same input -> correct output "5:03"
    TEST_ASSERT_EQUAL_STRING("5:03", formatRemainingTime_Fixed(303000).c_str());
}

void test_fixed_impl_one_second() {
    TEST_ASSERT_EQUAL_STRING("0:01", formatRemainingTime_Fixed(1000).c_str());
}

void test_fixed_impl_nine_seconds() {
    TEST_ASSERT_EQUAL_STRING("0:09", formatRemainingTime_Fixed(9000).c_str());
}

void test_fixed_impl_ten_seconds() {
    TEST_ASSERT_EQUAL_STRING("0:10", formatRemainingTime_Fixed(10000).c_str());
}

// ---- Large duration -------------------------------------------------------
void test_format_large_duration() {
    // 59 min 59 sec = 3 599 000 ms
    TEST_ASSERT_EQUAL_STRING("59:59", formatRemainingTime_Fixed(3599000).c_str());
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int /*argc*/, char** /*argv*/) {
    UNITY_BEGIN();

    // Harness smoke
    RUN_TEST(test_arduino_string_basic);
    RUN_TEST(test_arduino_string_int_conversion);
    RUN_TEST(test_arduino_string_concat);

    // Duration formatting
    RUN_TEST(test_format_zero_returns_zeros);
    RUN_TEST(test_format_negative_returns_zeros);
    RUN_TEST(test_format_double_digit_seconds);
    RUN_TEST(test_format_exactly_one_minute);
    RUN_TEST(test_current_impl_has_seconds_suffix_bug);   // documents bug
    RUN_TEST(test_fixed_impl_single_digit_seconds);
    RUN_TEST(test_fixed_impl_one_second);
    RUN_TEST(test_fixed_impl_nine_seconds);
    RUN_TEST(test_fixed_impl_ten_seconds);
    RUN_TEST(test_format_large_duration);

    return UNITY_END();
}
