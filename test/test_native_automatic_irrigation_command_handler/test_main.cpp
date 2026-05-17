#include <unity.h>

#include "../support/Arduino.h"
#include "../support/fakes/FakeMessagePublisher.h"

#include "../../src/services/automaticirrigationcontroller.cpp"
#include "../../src/services/automaticirrigationpublisher.cpp"
#include "../../src/services/automaticirrigationcommandhandler.cpp"

void Trace::log(TraceLevel level, String message)
{
  (void)level;
  (void)message;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_handle_enabled_true_command_updates_controller_and_publishes_state(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  controller.setup(false);
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);
  AutomaticIrrigationCommandHandler handler(controller, automaticPublisher);

  automaticPublisher.setup("device");
  handler.setup("device");

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigation/automatic/enabled/set", "true"));

  TEST_ASSERT_TRUE(controller.isEnabled());
  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigation/automatic/enabled"));
  TEST_ASSERT_EQUAL_STRING("true", publisher.entries[0].payload.c_str());
}

void test_handle_enabled_false_command_updates_controller_and_publishes_state(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  controller.setup(true);
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);
  AutomaticIrrigationCommandHandler handler(controller, automaticPublisher);

  automaticPublisher.setup("device");
  handler.setup("device");

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigation/automatic/enabled/set", "false"));

  TEST_ASSERT_FALSE(controller.isEnabled());
  TEST_ASSERT_EQUAL_INT(
      1, publisher.countTopic("device/irrigation/automatic/enabled"));
  TEST_ASSERT_EQUAL_STRING("false", publisher.entries[0].payload.c_str());
}

void test_invalid_payload_is_handled_without_changing_state(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  controller.setup(false);
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);
  AutomaticIrrigationCommandHandler handler(controller, automaticPublisher);

  automaticPublisher.setup("device");
  handler.setup("device");

  TEST_ASSERT_TRUE(handler.handleCommand(
      "device/irrigation/automatic/enabled/set", "maybe"));

  TEST_ASSERT_FALSE(controller.isEnabled());
  TEST_ASSERT_TRUE(publisher.entries.empty());
}

void test_unrelated_topic_is_not_handled(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  controller.setup(false);
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);
  AutomaticIrrigationCommandHandler handler(controller, automaticPublisher);

  automaticPublisher.setup("device");
  handler.setup("device");

  TEST_ASSERT_FALSE(
      handler.handleCommand("device/irrigation/automatic/enabled", "true"));
  TEST_ASSERT_FALSE(controller.isEnabled());
  TEST_ASSERT_TRUE(publisher.entries.empty());
}

void test_get_enabled_set_topic_returns_command_topic(void)
{
  FakeMessagePublisher publisher;
  AutomaticIrrigationController controller;
  AutomaticIrrigationPublisher automaticPublisher(publisher, controller);
  AutomaticIrrigationCommandHandler handler(controller, automaticPublisher);

  handler.setup("device");

  TEST_ASSERT_EQUAL_STRING("device/irrigation/automatic/enabled/set",
                           handler.getEnabledSetTopic());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_handle_enabled_true_command_updates_controller_and_publishes_state);
  RUN_TEST(test_handle_enabled_false_command_updates_controller_and_publishes_state);
  RUN_TEST(test_invalid_payload_is_handled_without_changing_state);
  RUN_TEST(test_unrelated_topic_is_not_handled);
  RUN_TEST(test_get_enabled_set_topic_returns_command_topic);

  return UNITY_END();
}
