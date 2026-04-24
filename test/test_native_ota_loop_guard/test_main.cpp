#include <unity.h>

#include "../../src/otaloopguard.h"

class FakeOtaLoopControl : public IOtaLoopControl
{
public:
  int loopCalls = 0;
  bool updating = false;

  void loop() override
  {
    loopCalls++;
  }

  bool isUpdating() const override
  {
    return updating;
  }
};

void setUp(void)
{
}

void tearDown(void)
{
}

void test_process_calls_ota_loop_every_time(void)
{
  FakeOtaLoopControl ota;
  OtaLoopGuard guard(ota);

  guard.process();
  guard.process();

  TEST_ASSERT_EQUAL_INT(2, ota.loopCalls);
}

void test_process_returns_false_when_no_update_is_running(void)
{
  FakeOtaLoopControl ota;
  ota.updating = false;
  OtaLoopGuard guard(ota);

  const bool shouldSkip = guard.process();

  TEST_ASSERT_FALSE(shouldSkip);
}

void test_process_returns_true_when_update_is_running(void)
{
  FakeOtaLoopControl ota;
  ota.updating = true;
  OtaLoopGuard guard(ota);

  const bool shouldSkip = guard.process();

  TEST_ASSERT_TRUE(shouldSkip);
}

int main(int /*argc*/, char ** /*argv*/)
{
  UNITY_BEGIN();

  RUN_TEST(test_process_calls_ota_loop_every_time);
  RUN_TEST(test_process_returns_false_when_no_update_is_running);
  RUN_TEST(test_process_returns_true_when_update_is_running);

  return UNITY_END();
}
