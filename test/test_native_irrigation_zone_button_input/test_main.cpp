#include <unity.h>

#define private public
#include "../../src/domain/irrigation_zone.h"
#undef private

#include "../../src/domain/irrigation_zone.cpp"

void StorageManager::begin()
{
}

void StorageManager::saveDurationTime(int, int)
{
}

int StorageManager::loadDurationTime(int, int defaultDurationMs)
{
  return defaultDurationMs;
}

void StorageManager::saveButtonState(int, bool)
{
}

bool StorageManager::loadButtonState(int)
{
  return false;
}

void StorageManager::saveRelayState(int, bool)
{
}

bool StorageManager::loadRelayState(int)
{
  return false;
}

void StorageManager::clearAllSettings()
{
}

void StorageManager::factoryReset(int)
{
}

String StorageManager::getKey(const char *prefix, int zoneIndex)
{
  return String(prefix) + String(zoneIndex);
}

void Trace::log(TraceLevel, String)
{
}

void Trace::plotBoolState(String, bool, int)
{
}

void Trace::plotLoopTime(String, int, unsigned long)
{
}

void setUp(void)
{
  arduinoFakeReset();
  IrrigationZone::setGlobalStartInhibit(false);
}

void tearDown(void)
{
}

void test_transient_interrupt_does_not_toggle_zone_state()
{
  IrrigationZone zone;
  zone.setup(35, 25, "device/irrigationZone8");
  zone.loadSettingsFromStorage(7);

  arduinoFakeSetMillis(BUTTON_DEBOUNCE_TIME + 1);
  zone.onHwBtnPressed();
  arduinoFakeSetDigitalPinState(35, LOW);

  zone.loop();

  TEST_ASSERT_FALSE(zone.getBtnState());
  TEST_ASSERT_FALSE(zone.getRelayState());
}

void test_valid_high_level_interrupt_toggles_zone_state()
{
  IrrigationZone zone;
  zone.setup(35, 25, "device/irrigationZone8");
  zone.loadSettingsFromStorage(7);

  arduinoFakeSetDigitalPinState(35, HIGH);
  arduinoFakeSetMillis(BUTTON_DEBOUNCE_TIME + 1);
  zone.onHwBtnPressed();

  zone.loop();

  TEST_ASSERT_TRUE(zone.getBtnState());
  TEST_ASSERT_TRUE(zone.getRelayState());
}

void test_input_only_button_pins_use_plain_input_mode()
{
  IrrigationZone zone;

  TEST_ASSERT_EQUAL_INT(INPUT, zone.getHwButtonPinMode(34));
  TEST_ASSERT_EQUAL_INT(INPUT, zone.getHwButtonPinMode(35));
  TEST_ASSERT_EQUAL_INT(INPUT, zone.getHwButtonPinMode(39));
  TEST_ASSERT_EQUAL_INT(INPUT_PULLDOWN, zone.getHwButtonPinMode(33));
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_transient_interrupt_does_not_toggle_zone_state);
  RUN_TEST(test_valid_high_level_interrupt_toggles_zone_state);
  RUN_TEST(test_input_only_button_pins_use_plain_input_mode);

  return UNITY_END();
}