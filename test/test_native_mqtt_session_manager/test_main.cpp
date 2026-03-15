#include <unity.h>

#include "../../src/mqttsessionmanager.cpp"
#include "../../src/mqttsessionmanager.h"

void setUp(void)
{
}
void tearDown(void)
{
}

void test_initial_state_is_disconnected()
{
  MqttSessionManager session;
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());
  TEST_ASSERT_FALSE(session.isConnected());
  TEST_ASSERT_EQUAL_INT(0, session.reconnectAttempts());
}

void test_should_attempt_connect_after_retry_interval()
{
  MqttSessionManager session(5000, 5);

  TEST_ASSERT_FALSE(session.shouldAttemptConnect(1000));
  TEST_ASSERT_TRUE(session.shouldAttemptConnect(5000));
  TEST_ASSERT_FALSE(session.shouldAttemptConnect(5001));
  TEST_ASSERT_TRUE(session.shouldAttemptConnect(10000));
}

void test_connect_success_transitions_to_connected_and_resets_attempts()
{
  MqttSessionManager session(5000, 5);

  TEST_ASSERT_TRUE(session.shouldAttemptConnect(5000));
  session.onConnectAttemptStarted();
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_CONNECTING_STATE,
                        session.state());

  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(1, session.reconnectAttempts());
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());

  session.onConnectAttemptStarted();
  session.onConnectSuccess();
  TEST_ASSERT_TRUE(session.isConnected());
  TEST_ASSERT_EQUAL_INT(0, session.reconnectAttempts());
}

void test_connect_failures_increment_and_reset_after_max_attempts()
{
  MqttSessionManager session(5000, 3);

  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(1, session.reconnectAttempts());

  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(2, session.reconnectAttempts());

  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(0, session.reconnectAttempts());
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());
}

void test_connection_lost_and_force_disconnect_set_disconnected_state()
{
  MqttSessionManager session;

  session.onConnectAttemptStarted();
  session.onConnectSuccess();
  TEST_ASSERT_TRUE(session.isConnected());

  session.onConnectionLost();
  TEST_ASSERT_FALSE(session.isConnected());
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());

  session.onConnectAttemptStarted();
  session.onConnectSuccess();
  TEST_ASSERT_TRUE(session.isConnected());

  session.forceDisconnect();
  TEST_ASSERT_FALSE(session.isConnected());
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_initial_state_is_disconnected);
  RUN_TEST(test_should_attempt_connect_after_retry_interval);
  RUN_TEST(test_connect_success_transitions_to_connected_and_resets_attempts);
  RUN_TEST(test_connect_failures_increment_and_reset_after_max_attempts);
  RUN_TEST(test_connection_lost_and_force_disconnect_set_disconnected_state);

  return UNITY_END();
}
