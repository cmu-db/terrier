#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/managed_pointer.h"
#include "self_driving/modeling/operating_unit.h"

namespace noisepage::messenger {
class Messenger;
}  // namespace noisepage::messenger

namespace noisepage::modelserver {
static constexpr const char *MODEL_CONN_ID_NAME = "model-server-conn";
static constexpr const char *MODEL_TARGET_NAME = "model";
static constexpr const char *MODEL_IPC_PATH = "/tmp/model-server-ipc";
static constexpr const char *MODEL_TCP_HOST = "127.0.0.1";
static constexpr const int MODEL_TCP_PORT = 15645;
static constexpr const int INVALID_PID = 0;

/*
 * Use 128 as convention to indicate failure in a subprocess:
 * https://www.gnu.org/software/libc/manual/html_node/Exit-Status.html
 */
static constexpr const unsigned char MODEL_SERVER_ERROR_BINARY = 128;

/**
 * ModelServerFuture is a wrapper over a condition_variable, and the condition.
 *
 * It is used to build synchronous API over asynchronous function calls:
 * ```c++
 *      ModelServerFuture future;
 *      AsyncCall(future);
 *
 *      auto result = future.Wait();
 *      bool success = result.first;
 *      auto data = result.second;
 * ```
 *
 * @tparam Result the future's result
 */
template <class Result>
class ModelServerFuture {
 public:
  /**
   * Initialize the future with a given mutex
   * @param mtx
   */
  ModelServerFuture() = default;

  /**
   * Suspends the current thread and wait for the result to be ready
   * @return Result, and success/fail
   */
  std::pair<Result, bool> Wait() {
    {
      std::unique_lock<std::mutex> lock(mtx_);

      // Wait for it to be done
      cvar_.wait(lock, [&] { return done_.load(); });
    }

    return {result_, success_};
  }

  /**
   * Indicate a future is done
   * @param result
   */
  void Done(Result result) {
    {
      std::unique_lock<std::mutex> lock(mtx_);
      result_ = result;
      done_ = true;
      success_ = true;
    }
    cvar_.notify_all();
  }

  /**
   * Indicate the future could not complete due to failure of the Async call
   */
  void Fail(const std::string &reason) {
    {
      std::unique_lock<std::mutex> lock(mtx_);
      done_ = true;
      success_ = false;
      fail_msg_ = reason;
    }
    cvar_.notify_all();
  }

  /**
   * Return why the operation has failed
   */
  const std::string &FailMessage() const { return fail_msg_; }

 private:
  /** Result for the future **/
  Result result_;

  /** Condition variable **/
  std::condition_variable cvar_;

  /** If async operation done */
  std::atomic<bool> done_ = false;

  /** If async operation succeeds */
  std::atomic<bool> success_ = false;

  /** Reason for failure **/
  std::string fail_msg_;

  /** Mutex associated with the condition variable **/
  std::mutex mtx_;
};

/**
 * Interface for ModelServerManager related operations
 */
class ModelServerManager {
  enum class Callback : uint64_t { NOOP = 0, CONNECTED, DEFAULT };

 public:
  /**
   * Construct a ModelServerManager with the given executable script to the python ModelServer
   * @param model_bin python script path
   * @param messenger Messenger pointer
   */
  ModelServerManager(const std::string &model_bin, const common::ManagedPointer<messenger::Messenger> &messenger);

  /**
   * Stop the python ModelServer when exits
   */
  ~ModelServerManager() { StopModelServer(); }

  /**
   * Stop the python-ModelServer daemon by sending a message to the Python model server
   */
  void StopModelServer();

  /**
   * Check if the model server has started.
   * The user of this function should poll this until the pid changes.
   *
   * A true return value means the model server script is running and ready to receive message.
   *
   * @return true if model server has started
   */
  bool ModelServerStarted() const { return connected_; }

  /**
   * Get the python model-server's PID
   * @return  pid
   */
  pid_t GetModelPid() const { return py_pid_; }

  /*******************************************************
   * ModelServer <-> ModelServerManager logic routines
   *******************************************************/

  /**
   * Ask the model server to print a message
   * This function does not expect a callback
   *
   * @warning The message should not contain '-' as per current protocol
   * @param msg Message to print
   */
  void PrintMessage(const std::string &msg);

  /**
   * Train a model with the given seq files directory
   *
   *  This function will be invoked asynchronously.
   *  The caller should poll from ModelServerManager::Trained() to check if the training has been done.
   *
   *  The caller should use the save_path as a handle to the trained model for inference later on.
   *
   * @param methods list of candidates methods that will be used for training
   * @param save_path path to where the trained model map will be stored at
   * @param seq_files_dir Seq files's enclosing directory (seq files generated by mini_runner)
   * @param future A future object which the caller waits for
   */
  void TrainWith(const std::vector<std::string> &methods, const std::string &seq_files_dir,
                 const std::string &save_path, ModelServerFuture<std::string> *future);

  /**
   * Perform inference on the given data file.
   *
   * This function should expect some return (Not figured out yet)
   * @param opunit Model for which to invoke
   * @param model_path Path to a model that has been trained. (In pickle format)
   * @param features Feature vectors
   * @return vector of results
   */
  std::vector<std::vector<double>> DoInference(const std::string &opunit, const std::string &model_path,
                                               const std::vector<std::vector<double>> &features);

 private:
  /**
   * This should be run as a thread routine.
   * 1. Make connection with the messenger
   * 2. Prepare arguments and forks to initialize a python daemon
   * 3. Record the pid
   */
  void StartModelServer(const std::string &model_path);

  static void ModelServerHandler(common::ManagedPointer<messenger::Messenger> messenger, std::string_view sender_id,
                                 std::string_view message, uint64_t recv_cb_id);

  std::string IPCPath() const { return MODEL_IPC_PATH; }
  std::string TCPPath() const { return fmt::format("{}:{}", MODEL_TCP_HOST, MODEL_TCP_PORT); }

  /** Messenger handler **/
  common::ManagedPointer<messenger::Messenger> messenger_;

  /** Connection router **/
  common::ManagedPointer<messenger::ConnectionRouter> router_;

  /** Thread the ModelServerManager runs in **/
  std::thread thd_;

  /** Python model pid **/
  pid_t py_pid_ = INVALID_PID;

  /** Bool shutting down **/
  std::atomic<bool> shut_down_ = false;

  /** If ModelServer is connected **/
  std::atomic<bool> connected_ = false;

  /** Mutex for sync */
  std::mutex mtx_;
};

}  // namespace noisepage::modelserver
