#include <Arduino.h>
#include <unity.h>

#include "config.h"

namespace
{
const int outputPins[] = {ZONE1_RELAY_PIN, ZONE2_RELAY_PIN, ZONE3_RELAY_PIN,
                          ZONE4_RELAY_PIN, ZONE5_RELAY_PIN, ZONE6_RELAY_PIN,
                          ZONE7_RELAY_PIN, ZONE8_RELAY_PIN, ZONE9_RELAY_PIN};

const int inputPins[] = {ZONE1_BUTTON_PIN, ZONE2_BUTTON_PIN, ZONE3_BUTTON_PIN,
                         ZONE4_BUTTON_PIN, ZONE5_BUTTON_PIN, ZONE6_BUTTON_PIN,
                         ZONE7_BUTTON_PIN, ZONE8_BUTTON_PIN, ZONE9_BUTTON_PIN};

bool isInputOnlyPin(int pin)
{
  return pin == 34 || pin == 35 || pin == 39;
}

void configureOutputsHigh()
{
  for (int pin : outputPins)
  {
    digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}

void configureInputs()
{
  for (int pin : inputPins)
  {
    pinMode(pin, isInputOnlyPin(pin) ? INPUT : INPUT_PULLDOWN);
  }
}

bool waitForInputLevel(int pin, int expectedLevel, unsigned long timeoutMs)
{
  const unsigned long start = millis();
  while (millis() - start < timeoutMs)
  {
    if (digitalRead(pin) == expectedLevel)
    {
      return true;
    }
    delay(10);
  }
  return false;
}
} // namespace

void setUp(void)
{
  configureOutputsHigh();
  configureInputs();
}

void tearDown(void)
{
  configureOutputsHigh();
}

void test_output_pins_can_drive_low_and_high()
{
  for (int pin : outputPins)
  {
    pinMode(pin, OUTPUT);

    digitalWrite(pin, LOW);
    delay(20);
    TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, digitalRead(pin),
                                  "ESP32 output pin should read back LOW");

    digitalWrite(pin, HIGH);
    delay(20);
    TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, digitalRead(pin),
                                  "ESP32 output pin should read back HIGH");
  }
}

void test_input_pins_are_low_when_idle()
{
  for (int pin : inputPins)
  {
    TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, digitalRead(pin),
                                  "ESP32 input pin should be LOW when idle");
  }
}

void test_input_pins_detect_high_when_button_or_jumper_is_applied()
{
  Serial.println();
  Serial.println("Manual input test:");
  Serial.println("For each listed input, press the matching button or jumper");
  Serial.println("the GPIO to 3V3 within 8 seconds.");

  for (int pin : inputPins)
  {
    Serial.print("Waiting for HIGH on GPIO ");
    Serial.println(pin);

    TEST_ASSERT_TRUE_MESSAGE(
        waitForInputLevel(pin, HIGH, 8000),
        "ESP32 input pin did not become HIGH within timeout");

    Serial.print("Release GPIO ");
    Serial.println(pin);
    TEST_ASSERT_TRUE_MESSAGE(
        waitForInputLevel(pin, LOW, 8000),
        "ESP32 input pin did not return LOW within timeout");
  }
}

void setup()
{
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_output_pins_can_drive_low_and_high);
  RUN_TEST(test_input_pins_are_low_when_idle);
  RUN_TEST(test_input_pins_detect_high_when_button_or_jumper_is_applied);

  UNITY_END();
}

void loop()
{
}
