#include <Arduino.h>
#include <memory>

const int HWBTN_1_PIN = 23;
const int RELAY_CH1_PIN = 22;
const unsigned long RELAY_ON_DURATION = 5000; // 5 seconds

volatile bool btn1Pressed = false;
unsigned long lastDebounceTime = 0;  // the last time the button was pressed
unsigned long relayOnStartTime = 0; // the time the relay was turned on
const int debounceDelay = 500;    // the debounce time in milliseconds
bool relaisState = LOW;

void PrintToConsole(String message)
{
  String timeStamp = String(millis());
  Serial.println(timeStamp + " | " + message);
}

void IRAM_ATTR handleBtn1Pressed() 
{
  PrintToConsole("Interrupt triggered");
  unsigned long now = millis();
  if (now - lastDebounceTime > debounceDelay) 
  {
    lastDebounceTime = now;
    btn1Pressed = true;
    PrintToConsole("Button pressed");
  }
}

void switchRelayON()
{
  relaisState = HIGH;
  digitalWrite(RELAY_CH1_PIN, LOW);
  PrintToConsole("Relais switched on");
}

void switchRelayOFF()
{
  relaisState = LOW;
  digitalWrite(RELAY_CH1_PIN, HIGH);
  PrintToConsole("Relais switched off");
}

void setup() 
{
  Serial.begin(115200);
  pinMode(HWBTN_1_PIN, INPUT_PULLDOWN); // Use INPUT_PULLUP instead of INPUT_PULLDOWN
  attachInterrupt(digitalPinToInterrupt(HWBTN_1_PIN), handleBtn1Pressed, RISING); // Trigger on FALLING edge
  pinMode(RELAY_CH1_PIN, OUTPUT);
  switchRelayOFF();
}

void loop() 
{
  if (btn1Pressed) 
  {
    btn1Pressed = false;
    PrintToConsole("Button pressed quit!");

    // Switch relay off immediately if it is on
    if (relaisState == HIGH) 
    {
      switchRelayOFF();
    }
    else
    {
      switchRelayON();
      relayOnStartTime = millis();
    }    
  }
  
  if (relaisState == HIGH && (millis() - relayOnStartTime >= RELAY_ON_DURATION)) 
  {
    switchRelayOFF();
  }

  delay(10);
}