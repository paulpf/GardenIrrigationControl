#include <unity.h>

#define private public
#include "../../src/config/config.h"
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

void test_relay_setup_drives_low_active_output_high_before_output_mode()
{
  IrrigationZone zone;

  zone.setup(ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN, "device/irrigationZone1");

  TEST_ASSERT_EQUAL_INT(OUTPUT, arduinoFakeGetPinMode(ZONE1_RELAY_PIN));
  TEST_ASSERT_EQUAL_INT(HIGH, arduinoFakeGetDigitalWriteState(ZONE1_RELAY_PIN));

  const auto &writes = arduinoFakeGetDigitalWriteHistory();
  TEST_ASSERT_GREATER_OR_EQUAL(2, writes.size());
  TEST_ASSERT_EQUAL_INT(ZONE1_RELAY_PIN, writes[writes.size() - 2].pin);
  TEST_ASSERT_EQUAL_INT(HIGH, writes[writes.size() - 2].value);
  TEST_ASSERT_EQUAL_INT(ZONE1_RELAY_PIN, writes[writes.size() - 1].pin);
  TEST_ASSERT_EQUAL_INT(HIGH, writes[writes.size() - 1].value);
}

void test_switch_relay_outputs_low_when_active_and_high_when_inactive()
{
  IrrigationZone zone;
  zone.setup(ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN, "device/irrigationZone1");
  arduinoFakeDigitalWriteHistoryStorage().clear();

  zone.switchRelay(true);

  TEST_ASSERT_TRUE(zone.getRelayState());
  TEST_ASSERT_EQUAL_INT(LOW, arduinoFakeGetDigitalWriteState(ZONE1_RELAY_PIN));
  TEST_ASSERT_EQUAL_INT(ZONE1_RELAY_PIN,
                        arduinoFakeGetDigitalWriteHistory().back().pin);
  TEST_ASSERT_EQUAL_INT(LOW, arduinoFakeGetDigitalWriteHistory().back().value);

  zone.switchRelay(false);

  TEST_ASSERT_FALSE(zone.getRelayState());
  TEST_ASSERT_EQUAL_INT(HIGH, arduinoFakeGetDigitalWriteState(ZONE1_RELAY_PIN));
  TEST_ASSERT_EQUAL_INT(ZONE1_RELAY_PIN,
                        arduinoFakeGetDigitalWriteHistory().back().pin);
  TEST_ASSERT_EQUAL_INT(HIGH, arduinoFakeGetDigitalWriteHistory().back().value);
}

void test_configured_relay_pins_all_start_inactive_high()
{
  const int buttonPins[] = {ZONE1_BUTTON_PIN, ZONE2_BUTTON_PIN,
                            ZONE3_BUTTON_PIN, ZONE4_BUTTON_PIN,
                            ZONE5_BUTTON_PIN, ZONE6_BUTTON_PIN,
                            ZONE7_BUTTON_PIN, ZONE8_BUTTON_PIN,
                            ZONE9_BUTTON_PIN};
  const int relayPins[] = {ZONE1_RELAY_PIN, ZONE2_RELAY_PIN,
                           ZONE3_RELAY_PIN, ZONE4_RELAY_PIN,
                           ZONE5_RELAY_PIN, ZONE6_RELAY_PIN,
                           ZONE7_RELAY_PIN, ZONE8_RELAY_PIN,
                           ZONE9_RELAY_PIN};

  for (int i = 0; i < 9; ++i)
  {
    arduinoFakeReset();
    IrrigationZone zone;
    zone.setup(buttonPins[i], relayPins[i], String("device/zone") + String(i));

    TEST_ASSERT_EQUAL_INT(OUTPUT, arduinoFakeGetPinMode(relayPins[i]));
    TEST_ASSERT_EQUAL_INT(HIGH, arduinoFakeGetDigitalWriteState(relayPins[i]));
  }
}

void test_button_loop_turns_relay_output_on_and_timer_expiry_turns_it_off()
{
  IrrigationZone zone;
  zone.setup(ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN, "device/irrigationZone1");
  zone.loadSettingsFromStorage(0);
  arduinoFakeDigitalWriteHistoryStorage().clear();

  arduinoFakeSetDigitalPinState(ZONE1_BUTTON_PIN, HIGH);
  arduinoFakeSetMillis(BUTTON_DEBOUNCE_TIME + 1);
  zone.onHwBtnPressed();
  zone.loop();

  TEST_ASSERT_TRUE(zone.getRelayState());
  TEST_ASSERT_EQUAL_INT(LOW, arduinoFakeGetDigitalWriteState(ZONE1_RELAY_PIN));

  arduinoFakeSetMillis(BUTTON_DEBOUNCE_TIME + DEFAULT_DURATION_TIME + 2);
  zone.loop();

  TEST_ASSERT_FALSE(zone.getRelayState());
  TEST_ASSERT_EQUAL_INT(HIGH, arduinoFakeGetDigitalWriteState(ZONE1_RELAY_PIN));
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_relay_setup_drives_low_active_output_high_before_output_mode);
  RUN_TEST(test_switch_relay_outputs_low_when_active_and_high_when_inactive);
  RUN_TEST(test_configured_relay_pins_all_start_inactive_high);
  RUN_TEST(test_button_loop_turns_relay_output_on_and_timer_expiry_turns_it_off);

  return UNITY_END();
}
