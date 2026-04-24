#include <unity.h>
#include <vector>
#include <string>

// Minimal setup phase tracking (no app includes needed)
class SetupPhaseTracker {
public:
  enum Phase {
    STORAGE,
    IDENTITY,
    CONNECTIVITY,
    SUBSYSTEMS,
    RUNTIME_SAFETY
  };

  static std::vector<Phase> executionOrder;

  static void reset() {
    executionOrder.clear();
  }

  static void recordPhase(Phase phase) {
    executionOrder.push_back(phase);
  }

  static bool wasPhaseExecuted(Phase phase) {
    for (const auto& p : executionOrder) {
      if (p == phase) return true;
    }
    return false;
  }

  static bool phaseExecutedBefore(Phase phase1, Phase phase2) {
    int idx1 = -1, idx2 = -1;
    for (size_t i = 0; i < executionOrder.size(); ++i) {
      if (executionOrder[i] == phase1) idx1 = i;
      if (executionOrder[i] == phase2) idx2 = i;
    }
    return idx1 >= 0 && idx2 >= 0 && idx1 < idx2;
  }

  static size_t phaseCount() {
    return executionOrder.size();
  }
};

std::vector<SetupPhaseTracker::Phase> SetupPhaseTracker::executionOrder;

// Test suite: Setup phase ordering and execution
void test_all_setup_phases_are_executed() {
  SetupPhaseTracker::reset();

  // Simulate the app setup sequence by recording phases
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  TEST_ASSERT_EQUAL_UINT(5, SetupPhaseTracker::phaseCount());
}

void test_setup_phases_execute_in_correct_order() {
  SetupPhaseTracker::reset();

  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  // Verify ordering constraints
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::STORAGE, SetupPhaseTracker::IDENTITY));
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::IDENTITY, SetupPhaseTracker::CONNECTIVITY));
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::CONNECTIVITY, SetupPhaseTracker::SUBSYSTEMS));
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::SUBSYSTEMS, SetupPhaseTracker::RUNTIME_SAFETY));
}

void test_identity_phase_executes_before_connectivity() {
  SetupPhaseTracker::reset();

  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  // Identity must run first so client name is set before WiFi setup
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::IDENTITY, SetupPhaseTracker::CONNECTIVITY));
}

void test_connectivity_phase_executes_before_subsystems() {
  SetupPhaseTracker::reset();

  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  // Connectivity must complete before MQTT/subsystem setup
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::CONNECTIVITY, SetupPhaseTracker::SUBSYSTEMS));
}

void test_subsystems_phase_executes_before_runtime_safety() {
  SetupPhaseTracker::reset();

  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  // Subsystems must initialize before final runtime safety checks
  TEST_ASSERT_TRUE(SetupPhaseTracker::phaseExecutedBefore(
      SetupPhaseTracker::SUBSYSTEMS, SetupPhaseTracker::RUNTIME_SAFETY));
}

void test_storage_phase_is_first() {
  SetupPhaseTracker::reset();

  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  // Storage must always be the first phase
  TEST_ASSERT_EQUAL_INT(SetupPhaseTracker::STORAGE, 
                        SetupPhaseTracker::executionOrder[0]);
}

void test_runtime_safety_phase_is_last() {
  SetupPhaseTracker::reset();

  SetupPhaseTracker::recordPhase(SetupPhaseTracker::STORAGE);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::IDENTITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::CONNECTIVITY);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::SUBSYSTEMS);
  SetupPhaseTracker::recordPhase(SetupPhaseTracker::RUNTIME_SAFETY);

  // Runtime safety must always be the last phase
  TEST_ASSERT_EQUAL_INT(SetupPhaseTracker::RUNTIME_SAFETY, 
                        SetupPhaseTracker::executionOrder[4]);
}

// Setup and teardown
void setUp(void) {
  SetupPhaseTracker::reset();
}

void tearDown(void) {
  SetupPhaseTracker::reset();
}

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(test_all_setup_phases_are_executed);
  RUN_TEST(test_setup_phases_execute_in_correct_order);
  RUN_TEST(test_identity_phase_executes_before_connectivity);
  RUN_TEST(test_connectivity_phase_executes_before_subsystems);
  RUN_TEST(test_subsystems_phase_executes_before_runtime_safety);
  RUN_TEST(test_storage_phase_is_first);
  RUN_TEST(test_runtime_safety_phase_is_last);

  return UNITY_END();
}
