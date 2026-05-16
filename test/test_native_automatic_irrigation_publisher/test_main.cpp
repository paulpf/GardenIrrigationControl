#include <unity.h>

#include "../support/Arduino.h"
#include "../support/fakes/FakeMessagePublisher.h"

#include "../../src/services/automaticirrigationcontroller.cpp"
#include "../../src/services/automaticirrigationpublisher.cpp"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_publish_state_is_suppressed_when_mqtt_disconnected(void)
{
  FakeMessagePublisher publisher;
  publisher.connected = false;
  AutomaticIrrigationController controller;
  controller.setup();
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);

  automaticPublisher.setup("device");
  automaticPublisher.publishState();

  TEST_ASSERT_TRUE(publisher.entries.empty());
}

void test_publish_state_reports_disabled_by_default(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  controller.setup();
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);

  automaticPublisher.setup("device");
  automaticPublisher.publishState();

  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigation/automatic/enabled"));
  TEST_ASSERT_EQUAL_STRING("false", publisher.entries[0].payload.c_str());
}

void test_publish_state_reports_enabled_state(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  controller.setup();
  controller.setEnabled(true);
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);

  automaticPublisher.setup("device");
  automaticPublisher.publishState();

  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigation/automatic/enabled"));
  TEST_ASSERT_EQUAL_STRING("true", publisher.entries[0].payload.c_str());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_publish_state_is_suppressed_when_mqtt_disconnected);
  RUN_TEST(test_publish_state_reports_disabled_by_default);
  RUN_TEST(test_publish_state_reports_enabled_state);

  return UNITY_END();
}
