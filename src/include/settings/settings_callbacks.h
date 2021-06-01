#pragma once

#include <memory>

#include "common/action_context.h"
#include "common/managed_pointer.h"

namespace noisepage {
class DBMain;
}

namespace noisepage::settings {

/**
 * Utility class for defining callbacks for settings in settings_defs.h.
 */
class Callbacks {
 public:
  Callbacks() = delete;

  /**
   * Default callback for settings in the Settings Manager. Does nothing but set the state to SUCCESS.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void NoOp(void *old_value, void *new_value, DBMain *db_main,
                   common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Changes the buffer segment pool size limit.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void BufferSegmentPoolSizeLimit(void *old_value, void *new_value, DBMain *db_main,
                                         common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Changes the buffer segment pool reuse limit.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void BufferSegmentPoolReuseLimit(void *old_value, void *new_value, DBMain *db_main,
                                          common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Changes the block store size limit.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void BlockStoreSizeLimit(void *old_value, void *new_value, DBMain *db_main,
                                  common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Changes the block store reuse limit.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void BlockStoreReuseLimit(void *old_value, void *new_value, DBMain *db_main,
                                   common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Changes the number of buffers the log manager uses.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void WalNumBuffers(void *old_value, void *new_value, DBMain *db_main,
                            common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Changes the number of buffers the log manager uses.
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void WalSerializationInterval(void *old_value, void *new_value, DBMain *db_main,
                                       common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for Logging component
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsLogging(void *old_value, void *new_value, DBMain *db_main,
                             common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for TransactionManager component
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsTransaction(void *old_value, void *new_value, DBMain *db_main,
                                 common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for GarbageCollector component
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsGC(void *old_value, void *new_value, DBMain *db_main,
                        common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for Execution component
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsExecution(void *old_value, void *new_value, DBMain *db_main,
                               common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for ExecutionEngine pipeline
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsPipeline(void *old_value, void *new_value, DBMain *db_main,
                              common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Update the sampling interval for logging
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsLoggingSampleRate(void *old_value, void *new_value, DBMain *db_main,
                                       common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Update the sampling interval for ExecutionEngine pipelines
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsPipelineSampleRate(void *old_value, void *new_value, DBMain *db_main,
                                        common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for bind command
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsBindCommand(void *old_value, void *new_value, DBMain *db_main,
                                 common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for execute command
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsExecuteCommand(void *old_value, void *new_value, DBMain *db_main,
                                    common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable metrics collection for Query Trace component
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsQueryTrace(void *old_value, void *new_value, DBMain *db_main,
                                common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Update the metrics output type being used by a metric component
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void MetricsQueryTraceOutput(void *old_value, void *new_value, DBMain *db_main,
                                      common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Set the forecast sample limit
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void ForecastSampleLimit(void *old_value, void *new_value, DBMain *db_main,
                                  common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Set the number of task manager threads
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void TaskPoolSize(void *old_value, void *new_value, DBMain *db_main,
                           common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Enable or disable planning in Pilot thread
   * @param old_value old settings value
   * @param new_value new settings value
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void PilotEnablePlanning(void *old_value, void *new_value, DBMain *db_main,
                                  common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Train the forecast model
   * @param old_value old settings value (not relevant)
   * @param new_value new settings value (not relevant)
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void TrainForecastModel(void *old_value, void *new_value, DBMain *db_main,
                                 common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Train the interference model
   * @param old_value old settings value (not relevant)
   * @param new_value new settings value (not relevant)
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void TrainInterferenceModel(void *old_value, void *new_value, DBMain *db_main,
                                     common::ManagedPointer<common::ActionContext> action_context);

  /**
   * Train the OU model
   * @param old_value old settings value (not relevant)
   * @param new_value new settings value (not relevant)
   * @param db_main pointer to db_main
   * @param action_context pointer to the action context for this settings change
   */
  static void TrainOUModel(void *old_value, void *new_value, DBMain *db_main,
                           common::ManagedPointer<common::ActionContext> action_context);
};
}  // namespace noisepage::settings
