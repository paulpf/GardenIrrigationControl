#include <unity.h>

#include "../../src/services/connectivitycoordinator.h"

class FakeWifiConnectivity : public IWifiConnectivity
{
public:
  bool connected = false;
  bool connectedEventPending = false;
  bool disconnectedEventPending = false;

  bool consumeConnectedEvent() override
  {
    const bool value = connectedEventPending;
    connectedEventPending = false;
    return value;
  }

  bool consumeDisconnectedEvent() override
  {
    const bool value = disconnectedEventPending;
    disconnectedEventPending = false;
    return value;
  }

  bool isConnected() const override
  {
    return connected;
  }
};

class FakeMqttConnectionControl : public IMqttConnectionControl
{
public:
  int requestConnectCalls = 0;
  int forceDisconnectCalls = 0;

  void requestConnect() override
  {
    requestConnectCalls++;
  }

  void forceDisconnect() override
  {
    forceDisconnectCalls++;
  }
};

void setUp(void)
{
}

void tearDown(void)
{
}

void test_disconnected_wifi_event_forces_mqtt_disconnect(void)
{
  FakeWifiConnectivity wifi;
  FakeMqttConnectionControl mqtt;
  ConnectivityCoordinator coordinator(wifi, mqtt);

  wifi.disconnectedEventPending = true;

  coordinator.handleEvents();

  TEST_ASSERT_EQUAL_INT(1, mqtt.forceDisconnectCalls);
  TEST_ASSERT_EQUAL_INT(0, mqtt.requestConnectCalls);
}

void test_connected_wifi_event_requests_mqtt_connect(void)
{
  FakeWifiConnectivity wifi;
  FakeMqttConnectionControl mqtt;
  ConnectivityCoordinator coordinator(wifi, mqtt);

  wifi.connectedEventPending = true;

  coordinator.handleEvents();

  TEST_ASSERT_EQUAL_INT(1, mqtt.requestConnectCalls);
  TEST_ASSERT_EQUAL_INT(0, mqtt.forceDisconnectCalls);
}

void test_ensure_mqtt_connect_requests_connect_when_wifi_is_connected(void)
{
  FakeWifiConnectivity wifi;
  FakeMqttConnectionControl mqtt;
  ConnectivityCoordinator coordinator(wifi, mqtt);

  wifi.connected = true;

  coordinator.ensureMqttConnected();

  TEST_ASSERT_EQUAL_INT(1, mqtt.requestConnectCalls);
}

void test_ensure_mqtt_connect_does_not_request_when_wifi_is_disconnected(void)
{
  FakeWifiConnectivity wifi;
  FakeMqttConnectionControl mqtt;
  ConnectivityCoordinator coordinator(wifi, mqtt);

  wifi.connected = false;

  coordinator.ensureMqttConnected();

  TEST_ASSERT_EQUAL_INT(0, mqtt.requestConnectCalls);
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_disconnected_wifi_event_forces_mqtt_disconnect);
  RUN_TEST(test_connected_wifi_event_requests_mqtt_connect);
  RUN_TEST(test_ensure_mqtt_connect_requests_connect_when_wifi_is_connected);
  RUN_TEST(test_ensure_mqtt_connect_does_not_request_when_wifi_is_disconnected);

  return UNITY_END();
}
