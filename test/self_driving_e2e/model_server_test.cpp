#include <vector>

#include "gtest/gtest.h"
#include "loggers/messenger_logger.h"
#include "loggers/model_server_logger.h"
#include "main/db_main.h"
#include "self_driving/model_server/model_server_manager.h"
#include "self_driving/modeling/operating_unit_util.h"
#include "test_util/test_harness.h"

namespace noisepage::modelserver {

/**
 * @warning Running this test requires external dependency to be located at specific paths.
 */
class ModelServerTest : public TerrierTest {
 protected:
  static constexpr const char *BUILD_ABS_PATH = "BUILD_ABS_PATH";

  /** @return Unique pointer to built DBMain that has the relevant parameters configured. */
  static std::unique_ptr<DBMain> BuildDBMain() {
    const char* env = ::getenv(BUILD_ABS_PATH);
    std::string project_build_path = (env ? env : "");
    auto model_server_path = "../script/self_driving/model_server.py";

    auto db_main = noisepage::DBMain::Builder()
                       .SetUseSettingsManager(false)
                       .SetUseMessenger(true)
                       .SetUseCatalog(true)
                       .SetUseModelServer(true)
                       .SetUseNetwork(true)
                       .SetUseGC(true)
                       .SetUseExecution(true)
                       .SetUseStatsStorage(true)
                       .SetUseTrafficCop(true)
                       .SetModelServerPath(model_server_path)
                       .Build();

    return db_main;
  }

  /**
   * Wrapper to hide long chain of function name resolution
   * @param unit_type OpUnit
   * @return  string representation of the opunit
   */
  static std::string OpUnitToString(selfdriving::ExecutionOperatingUnitType unit_type) {
    return selfdriving::OperatingUnitUtil::ExecutionOperatingUnitTypeToString(unit_type);
  }
};

// NOLINTNEXTLINE
TEST_F(ModelServerTest, PipelineTest) {
  messenger::messenger_logger->set_level(spdlog::level::info);
  model_server_logger->set_level(spdlog::level::info);

  auto primary = BuildDBMain();
  primary->GetNetworkLayer()->GetServer()->RunServer();

  auto ms_manager = primary->GetModelServerManager();

  // Wait for the model server process to start
  while (!ms_manager->ModelServerStarted()) {
  }

  std::vector<std::vector<double>> features{
      {0, 0, 10000, 4, 1, 10000, 1, 0, 0},
      {0, 0, 10000, 4, 1, 10000, 1, 0, 0},
      {0, 0, 10000, 4, 1, 10000, 1, 0, 0},
      {0, 0, 10000, 4, 1, 10000, 1, 0, 0},
  };

  // Send a message
  std::string msg = "ModelServerTest";
  ms_manager->PrintMessage(msg);

  // Perform a training of the opunit models with {lr, rf} as training methods.
  std::vector<std::string> methods{"lr", "rf"};
  std::string save_path = "model_server_test.pickle";

  ModelServerFuture<std::string> future;
  const char* env = ::getenv(BUILD_ABS_PATH);
  std::string project_build_path = (env ? env : "");
  ms_manager->TrainModel(ModelType::Type::MiniRunner, methods, project_build_path + "/bin", save_path, nullptr,
                         common::ManagedPointer<ModelServerFuture<std::string>>(&future));
  auto res = future.Wait();
  ASSERT_EQ(res.second, true);  // Training succeeds

  // Perform inference on the trained opunit model for various opunits
  auto result = ms_manager->InferMiniRunnerModel(
      OpUnitToString(selfdriving::ExecutionOperatingUnitType::OP_INTEGER_PLUS_OR_MINUS), save_path, features);
  ASSERT_TRUE(result.second);
  ASSERT_EQ(result.first.size(), features.size());
  result = ms_manager->InferMiniRunnerModel(OpUnitToString(selfdriving::ExecutionOperatingUnitType::OP_REAL_COMPARE),
                                            save_path, features);
  ASSERT_TRUE(result.second);
  ASSERT_EQ(result.first.size(), features.size());
  result = ms_manager->InferMiniRunnerModel(
      OpUnitToString(selfdriving::ExecutionOperatingUnitType::OP_INTEGER_MULTIPLY), save_path, features);
  ASSERT_TRUE(result.second);
  ASSERT_EQ(result.first.size(), features.size());

  // Model at another path should not exist
  std::string non_exist_path("model_server_test_non_exist.pickle");
  result = ms_manager->InferMiniRunnerModel(
      OpUnitToString(selfdriving::ExecutionOperatingUnitType::OP_INTEGER_PLUS_OR_MINUS), non_exist_path, features);
  ASSERT_FALSE(result.second);

  // Inference with invalid opunit name will fail
  result = ms_manager->InferMiniRunnerModel("OP_SUPER_MAGICAL_DIVIDE", non_exist_path, features);
  ASSERT_FALSE(result.second);

  // Quit
  ms_manager->StopModelServer();
}

// NOLINTNEXTLINE
TEST_F(ModelServerTest, ForecastTest) {
  messenger::messenger_logger->set_level(spdlog::level::info);
  model_server_logger->set_level(spdlog::level::info);

  auto primary = BuildDBMain();
  primary->GetNetworkLayer()->GetServer()->RunServer();

  auto ms_manager = primary->GetModelServerManager();

  // Wait for the model server process to start
  while (!ms_manager->ModelServerStarted()) {
  }

  // Perform a training of the opunit models with {lr, rf} as training methods.
  std::vector<std::string> methods{"LSTM"};
  uint64_t interval = 500000;
  const char* env = ::getenv(BUILD_ABS_PATH);
  std::string project_build_path = (env ? env : "");
  std::string save_path = "model.pickle";
  std::string input_path = project_build_path + "../script/self_driving/query_trace.csv";

  ModelServerFuture<std::string> future;
  ms_manager->TrainForecastModel(methods, input_path, save_path, interval,
                                 common::ManagedPointer<ModelServerFuture<std::string>>(&future));
  auto res = future.Wait();
  ASSERT_EQ(res.second, true);  // Training succeeds

  // Perform inference on the trained opunit model for various opunits
  auto result = ms_manager->InferForecastModel(input_path, save_path, methods, NULL, interval);
  ASSERT_TRUE(result.second);

  // Quit
  ms_manager->StopModelServer();
}

}  // namespace noisepage::modelserver
