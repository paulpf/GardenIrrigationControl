#include <unity.h>

#include "../../src/services/automaticirrigationcontroller.cpp"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_setup_defaults_to_disabled(void)
{
  AutomaticIrrigationController controller;

  controller.setup();

  TEST_ASSERT_FALSE(controller.isEnabled());
  TEST_ASSERT_FALSE(controller.canRunSchedules());
}

void test_setup_can_accept_explicit_default(void)
{
  AutomaticIrrigationController controller;

  controller.setup(true);

  TEST_ASSERT_TRUE(controller.isEnabled());
  TEST_ASSERT_TRUE(controller.canRunSchedules());
}

void test_set_enabled_controls_schedule_gate(void)
{
  AutomaticIrrigationController controller;
  controller.setup();

  controller.setEnabled(true);
  TEST_ASSERT_TRUE(controller.canRunSchedules());

  controller.setEnabled(false);
  TEST_ASSERT_FALSE(controller.canRunSchedules());
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_setup_defaults_to_disabled);
  RUN_TEST(test_setup_can_accept_explicit_default);
  RUN_TEST(test_set_enabled_controls_schedule_gate);

  return UNITY_END();
}
