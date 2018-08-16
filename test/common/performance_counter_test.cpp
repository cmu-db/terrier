#include <functional>
#include <random>
#include <vector>
#include "common/json.h"
#include "gtest/gtest.h"
#include "util/multi_threaded_test_util.h"
#include "util/performance_counter_test_util.h"

namespace terrier {

/**
 * Helper class for testing the four attributes of a CacheCounter.
 */
class CacheCounterTestObject {
 private:
  std::atomic<uint64_t> num_insert{0};
  std::atomic<uint32_t> num_hit{0};
  std::atomic<uint16_t> num_failure{0};
  std::atomic<uint8_t> num_user{0};
  CacheCounter *cc;

  /**
   * Calls the gtest EXPECT_EQ function for all the relevant variables.
   */
  void Equal() const {
    EXPECT_EQ(cc->Get_num_insert(), num_insert);
    EXPECT_EQ(cc->Get_num_hit(), num_hit);
    EXPECT_EQ(cc->Get_num_failure(), num_failure);
    EXPECT_EQ(cc->Get_num_user(), num_user);
  }

  std::vector<double> work_probs = std::vector<double>(8, 0.125);
  std::vector<std::function<void()>> workloads = {
      [&] {cc->Inc_num_insert(); num_insert++; Equal();},
      [&] {cc->Inc_num_hit(); num_hit++; Equal();},
      [&] {cc->Inc_num_failure(); num_failure++; Equal();},
      [&] {cc->Inc_num_user(); num_user++; Equal();},
      [&] {cc->Dec_num_insert(); num_insert--; Equal();},
      [&] {cc->Dec_num_hit(); num_hit--; Equal();},
      [&] {cc->Dec_num_failure(); num_failure--; Equal();},
      [&] {cc->Dec_num_user(); num_user--; Equal();},
  };

 public:
  /**
   * Instantiates the CacheCounterTestObject with the given CacheCounter.
   * It will check that it remains consistent with the CacheCounter;
   * if you modify the CacheCounter, make sure you tell it.
   */
  explicit CacheCounterTestObject(CacheCounter *cc) : cc(cc) {}

  /**
   * Zeroes the cache counter, automatically checking that all the state remains consistent.
   */
  void Zero() {
    num_insert.store(0);
    num_hit.store(0);
    num_failure.store(0);
    num_user.store(0);
    cc->ZeroCounters();
    Equal();
  }

  /**
   * Runs random operations, automatically checking that all the state remains consistent.
   * @param generator random number generator
   * @param num_operations number of operations to run
   */
  void RandomOperation(std::default_random_engine generator, uint32_t num_operations) {
    MultiThreadedTestUtil::InvokeWorkloadWithDistribution(workloads, work_probs, &generator, num_operations);
  }
};

#ifdef NDEBUG
TEST(PerformanceCounterTests, CannotTest) {}
#else
// Test simple increment/decrement/zero/name setting operations on a performance counter
// NOLINTNEXTLINE
TEST(PerformanceCounterTests, SimpleCorrectnessTest) {
  std::default_random_engine generator;
  const uint32_t num_iterations = 1000;
  const uint32_t num_operations = 1000;

  CacheCounter cc;
  CacheCounterTestObject cc_test(&cc);

  // check we can change name, by default it is the class name
  EXPECT_EQ(cc.GetName(), "CacheCounter");
  cc.SetName("NewName");
  EXPECT_EQ(cc.GetName(), "NewName");
  cc.SetName("CacheCounter");
  EXPECT_EQ(cc.GetName(), "CacheCounter");

  cc_test.Zero();
  // test that our counter works for increment/decrement/get
  for (uint32_t i = 0; i < num_iterations; ++i) {
    cc_test.RandomOperation(generator, num_operations);
  }
}

// Test that we can serialize and deserialize our performance counters
// NOLINTNEXTLINE
TEST(PerformanceCounterTests, SerializationTest) {
  std::default_random_engine generator;
  const uint32_t num_iterations = 1000;
  const uint32_t num_operations = 100;

  CacheCounter cc;
  CacheCounterTestObject cc_test(&cc);

  terrier::common::json json_old, json_new;

  for (uint32_t i = 0; i < num_iterations; ++i) {
    cc_test.Zero();

    // perform some random operations
    cc_test.RandomOperation(generator, num_operations);
    // save the current state
    json_old = cc.ToJson();
    EXPECT_EQ(json_old, cc.ToJson());
    // perform one more random operation
    cc_test.RandomOperation(generator, 1);
    // assert the state is not the same
    json_new = cc.ToJson();
    EXPECT_EQ(json_new, cc.ToJson());
    EXPECT_NE(json_old, json_new);
    // restore cc from the old state
    cc.FromJson(json_old);
    // assert the state is the same now
    EXPECT_EQ(json_old, cc.ToJson());
  }
}

#endif  // NDEBUG

}  // namespace terrier
