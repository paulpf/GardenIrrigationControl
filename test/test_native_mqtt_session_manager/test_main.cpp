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

// Phase 4.2 Additional Tests: Edge cases and state transitions
void test_multiple_connection_attempts_without_reset()
{
  MqttSessionManager session(1000, 5);

  // First attempt fails
  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(1, session.reconnectAttempts());

  // Second attempt fails
  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(2, session.reconnectAttempts());
  TEST_ASSERT_FALSE(session.isConnected());
}

void test_retry_interval_honored_on_failure()
{
  MqttSessionManager session(5000, 3);

  session.onConnectAttemptStarted();
  session.onConnectFailure();

  // Should not retry before interval
  TEST_ASSERT_FALSE(session.shouldAttemptConnect(4999));
  // Should retry after interval
  TEST_ASSERT_TRUE(session.shouldAttemptConnect(5000));
}

void test_state_machine_disconnected_to_connecting_to_connected()
{
  MqttSessionManager session;

  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());

  session.onConnectAttemptStarted();
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_CONNECTING_STATE,
                        session.state());

  session.onConnectSuccess();
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_CONNECTED_STATE,
                        session.state());
}

void test_state_machine_connected_to_disconnected_on_connection_lost()
{
  MqttSessionManager session;

  session.onConnectAttemptStarted();
  session.onConnectSuccess();
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_CONNECTED_STATE,
                        session.state());

  session.onConnectionLost();
  TEST_ASSERT_EQUAL_INT(MqttSessionManager::MQTT_DISCONNECTED_STATE,
                        session.state());
  TEST_ASSERT_FALSE(session.isConnected());
}

void test_failed_max_retries_resets_attempt_counter()
{
  MqttSessionManager session(1000, 2);

  // Fail twice (max)
  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(1, session.reconnectAttempts());

  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(0, session.reconnectAttempts()); // Reset to 0
}

void test_successful_connect_resets_failed_attempts()
{
  MqttSessionManager session(1000, 5);

  // Fail once
  session.onConnectAttemptStarted();
  session.onConnectFailure();
  TEST_ASSERT_EQUAL_INT(1, session.reconnectAttempts());

  // Succeed on next attempt
  session.onConnectAttemptStarted();
  session.onConnectSuccess();
  TEST_ASSERT_EQUAL_INT(0, session.reconnectAttempts()); // Reset to 0
  TEST_ASSERT_TRUE(session.isConnected());
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_initial_state_is_disconnected);
  RUN_TEST(test_should_attempt_connect_after_retry_interval);
  RUN_TEST(test_connect_success_transitions_to_connected_and_resets_attempts);
  RUN_TEST(test_connect_failures_increment_and_reset_after_max_attempts);
  RUN_TEST(test_connection_lost_and_force_disconnect_set_disconnected_state);

  // Phase 4.2 Additional tests
  RUN_TEST(test_multiple_connection_attempts_without_reset);
  RUN_TEST(test_retry_interval_honored_on_failure);
  RUN_TEST(test_state_machine_disconnected_to_connecting_to_connected);
  RUN_TEST(test_state_machine_connected_to_disconnected_on_connection_lost);
  RUN_TEST(test_failed_max_retries_resets_attempt_counter);
  RUN_TEST(test_successful_connect_resets_failed_attempts);

  return UNITY_END();
}
