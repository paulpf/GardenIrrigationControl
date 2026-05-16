#include <string>
#include <unity.h>

#include "../support/Arduino.h"
#include "../support/fakes/FakeMessagePublisher.h"

#include "../../src/services/diagnosticsreporter.cpp"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_publish_report_uses_system_diagnostics_topic(void)
{
  FakeMessagePublisher publisher;
  DiagnosticsReporter reporter(publisher);

  reporter.setup("device");
  reporter.record(TraceLevel::ERROR, "WiFi connection timeout", 1234);
  reporter.publishReport();

  TEST_ASSERT_EQUAL_INT(1, publisher.countTopic("device/system/diagnostics"));
  TEST_ASSERT_EQUAL_STRING(
      "{\"count\":1,\"events\":[{\"level\":\"error\",\"uptimeMs\":1234,"
      "\"message\":\"WiFi connection timeout\"}]}",
      publisher.entries[0].payload.c_str());
}

void test_publish_report_is_suppressed_when_mqtt_is_disconnected(void)
{
  FakeMessagePublisher publisher;
  publisher.connected = false;
  DiagnosticsReporter reporter(publisher);

  reporter.setup("device");
  reporter.record(TraceLevel::ERROR, "WiFi connection timeout", 1234);
  reporter.publishReport();

  TEST_ASSERT_TRUE(publisher.entries.empty());
}

void test_ring_buffer_keeps_only_latest_events(void)
{
  FakeMessagePublisher publisher;
  DiagnosticsReporter reporter(publisher);
  reporter.setup("device");

  for (int i = 0; i < 10; i++)
  {
    reporter.record(TraceLevel::WARNING, String("event") + String(i), i);
  }

  reporter.publishReport();

  TEST_ASSERT_EQUAL_INT(8, reporter.count());
  TEST_ASSERT_TRUE(publisher.entries[0].payload.find("event0") ==
                   std::string::npos);
  TEST_ASSERT_TRUE(publisher.entries[0].payload.find("event1") ==
                   std::string::npos);
  TEST_ASSERT_TRUE(publisher.entries[0].payload.find("event2") !=
                   std::string::npos);
  TEST_ASSERT_TRUE(publisher.entries[0].payload.find("event9") !=
                   std::string::npos);
}

void test_report_escapes_json_message_text(void)
{
  FakeMessagePublisher publisher;
  DiagnosticsReporter reporter(publisher);

  reporter.setup("device");
  reporter.record(TraceLevel::ERROR, "bad \"quote\" and \\ slash", 1);
  reporter.publishReport();

  TEST_ASSERT_TRUE(publisher.entries[0].payload.find(
                       "bad \\\"quote\\\" and \\\\ slash") !=
                   std::string::npos);
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_publish_report_uses_system_diagnostics_topic);
  RUN_TEST(test_publish_report_is_suppressed_when_mqtt_is_disconnected);
  RUN_TEST(test_ring_buffer_keeps_only_latest_events);
  RUN_TEST(test_report_escapes_json_message_text);

  return UNITY_END();
}
