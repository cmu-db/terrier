#include "model_server/model_server_manager.h"

#include <csignal.h>
#include <sys/wait.h>

#include <filesystem>
#include <thread>  // NOLINT

#include "common/json.h"
#include "loggers/model_logger.h"
#include "messenger/connection_destination.h"
#include "messenger/messenger.h"

namespace noisepage::model {

/**
 * This initializes a connection to the model by openning up a zmq connection
 * @param messenger
 * @return A ConnectionId that should be used only to the calling thread
 */
common::ManagedPointer<messenger::ConnectionRouter> ListenAndMakeConnection(
    const common::ManagedPointer<messenger::Messenger> &messenger, const std::string &ipc_path,
    messenger::CallbackFn model_server_logic) {
  // Create an IPC connection that the Python process will talk to.
  auto destination = messenger::ConnectionDestination::MakeIPC(MODEL_TARGET_NAME, ipc_path);

  // Listen for the connection
  messenger->ListenForConnection(destination, MODEL_CONN_ID_NAME, std::move(model_server_logic));
  while (true) {
    try {
      return messenger->GetConnectionRouter(MODEL_CONN_ID_NAME);
    } catch (std::exception &e) {
      ::sleep(1);
    }
  }
}

}  // namespace noisepage::model

namespace noisepage::model {

void ModelServerManager::ModelServerHandler(common::ManagedPointer<messenger::Messenger> messenger,
                                            std::string_view sender_id, std::string_view message, uint64_t recv_cb_id) {
  // TODO(ricky): Currently just echo the results. No continuation
  MESSENGER_LOG_TRACE("[PID={},SEND_ID={},RECV_ID={}] Messenger RECV: {}", ::getpid(), message, sender_id, recv_cb_id);
  (void)messenger;
}

ModelServerManager::ModelServerManager(const std::string &model_bin,
                                       const common::ManagedPointer<messenger::Messenger> &messenger)
    : messenger_(messenger),
      router_(ListenAndMakeConnection(messenger, MODEL_IPC_PATH, ModelServerHandler)),
      thd_(std::thread([this, model_bin] {
        while (!shut_down_) {
          this->StartModelServer(model_bin);
        }
      })) {}

void ModelServerManager::StartModelServer(const std::string &model_path) {
  py_pid_ = ::fork();
  if (py_pid_ < 0) {
    MODEL_LOG_ERROR("Failed to fork to spawn model process");
    return;
  }

  // Fork success
  if (py_pid_ > 0) {
    // Parent Process Routine
    MODEL_LOG_INFO("Model Server Process running at : {}", py_pid_);

    // Wait for the child to exit
    int status;
    pid_t wait_pid;

    // Wait for the child
    wait_pid = waitpid(py_pid_, &status, 0);

    if (wait_pid < 0) {
      MODEL_LOG_ERROR("Failed to wait for the child process...");
      return;
    }

    // TODO(ricky): what other cases the model server manager should give up?
    if (WIFEXITED(status) && WEXITSTATUS(status) == MODEL_ERROR_BINARY) {
      MODEL_LOG_ERROR("Stop model server");
      shut_down_ = true;
    }
  } else {
    // Run the script in in a child
    std::string ipc_path = IPCPath();
    char *args[] = {model_path.data(), ipc_path.data(), nullptr};
    MODEL_LOG_INFO("Inovking binary at :{}", model_path);
    if (execvp(args[0], args) < 0) {
      MODEL_LOG_ERROR("Failed to execute model binary: {}, {}", strerror(errno), errno);
      /* Shutting down */
      ::_exit(MODEL_ERROR_BINARY);
    }
  }
}

void ModelServerManager::PrintMessage(const std::string &msg) {
  nlohmann::json j;
  j["cmd"] = "PRINT";
  j["data"] = msg;
  messenger_->SendMessage(router_, MODEL_TARGET_NAME, j.dump(), messenger::CallbackFns::Noop, 0);
}

void ModelServerManager::StopModelServer() {
  if (!shut_down_) {
    shut_down_ = true;
    nlohmann::json j;
    j["cmd"] = "QUIT";
    j["data"] = "";
    messenger_->SendMessage(router_, MODEL_TARGET_NAME, j.dump(), messenger::CallbackFns::Noop, 0);
  }
  if (thd_.joinable()) thd_.join();
}

void ModelServerManager::TrainWith(const std::string &model_name, const std::string &seq_files_dir) {
  nlohmann::json j;
  j["cmd"] = "TRAIN";
  j["data"]["model_name"] = model_name;
  j["data"]["seq_files"] = seq_files_dir;
  messenger_->SendMessage(router_, MODEL_TARGET_NAME, j.dump(), messenger::CallbackFns::Noop, 0);
}

void ModelServerManager::DoInference(const std::string &data_file, const std::string &model_map_path) {
  nlohmann::json j;
  j["cmd"] = "INFER";
  j["data"]["data_file"] = data_file;
  j["data"]["model_map_path"] = model_map_path;
  messenger_->SendMessage(router_, MODEL_TARGET_NAME, j.dump(), messenger::CallbackFns::Noop, 0);
}

}  // namespace noisepage::model
