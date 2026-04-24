#include <unity.h>

#include <vector>

#include "../../src/contracts/itimeprovider.h"
#include "../../src/services/wificonnectionawaiter.h"

class FakeTimeProvider : public ITimeProvider
{
public:
  unsigned long now = 0;
  int delayCalls = 0;

  unsigned long millis() const override
  {
    return now;
  }

  void delayMs(unsigned long ms) override
  {
    delayCalls++;
    now += ms;
  }
};

class FakeWifiConnectivity : public IWifiConnectivity
{
public:
  std::vector<bool> sequence;
  mutable size_t index = 0;

  bool consumeConnectedEvent() override
  {
    return false;
  }

  bool consumeDisconnectedEvent() override
  {
    return false;
  }

  bool isConnected() const override
  {
    if (sequence.empty())
    {
      return false;
    }

    const size_t current =
        (index < sequence.size()) ? index : sequence.size() - 1;
    const bool value = sequence[current];
    if (index < sequence.size())
    {
      index++;
    }

    return value;
  }
};

void setUp(void)
{
}

void tearDown(void)
{
}

void test_wait_returns_true_immediately_when_already_connected(void)
{
  FakeWifiConnectivity wifi;
  wifi.sequence = {true};
  FakeTimeProvider timeProvider;

  WifiConnectionAwaiter awaiter(wifi, timeProvider);

  const bool connected = awaiter.waitForConnection(5000);

  TEST_ASSERT_TRUE(connected);
  TEST_ASSERT_EQUAL_INT(0, timeProvider.delayCalls);
}

void test_wait_returns_true_when_connection_happens_before_timeout(void)
{
  FakeWifiConnectivity wifi;
  wifi.sequence = {false, false, true};
  FakeTimeProvider timeProvider;

  WifiConnectionAwaiter awaiter(wifi, timeProvider);

  const bool connected = awaiter.waitForConnection(500);

  TEST_ASSERT_TRUE(connected);
  TEST_ASSERT_EQUAL_INT(2, timeProvider.delayCalls);
}

void test_wait_returns_false_when_timeout_is_exceeded(void)
{
  FakeWifiConnectivity wifi;
  wifi.sequence = {false};
  FakeTimeProvider timeProvider;

  WifiConnectionAwaiter awaiter(wifi, timeProvider);

  const bool connected = awaiter.waitForConnection(500);

  TEST_ASSERT_FALSE(connected);
  TEST_ASSERT_EQUAL_INT(6, timeProvider.delayCalls);
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_wait_returns_true_immediately_when_already_connected);
  RUN_TEST(test_wait_returns_true_when_connection_happens_before_timeout);
  RUN_TEST(test_wait_returns_false_when_timeout_is_exceeded);

  return UNITY_END();
}
