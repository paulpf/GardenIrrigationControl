#include <type_traits>
#include <unity.h>

#include "../../src/app/gardencontrollerbootstrap.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bootstrap_application_signature_returns_reference()
{
  using ReturnType =
      decltype(((GardenControllerBootstrap *)nullptr)->application());
  const bool returnsReference =
      std::is_same<ReturnType, GardenControllerApp &>::value;

  TEST_ASSERT_TRUE(returnsReference);
}

void test_bootstrap_is_default_constructible()
{
  const bool constructible =
      std::is_default_constructible<GardenControllerBootstrap>::value;
  TEST_ASSERT_TRUE(constructible);
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_bootstrap_application_signature_returns_reference);
  RUN_TEST(test_bootstrap_is_default_constructible);

  return UNITY_END();
}
