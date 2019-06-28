#include "metrics/metrics_store.h"
#include <bitset>
#include <memory>
#include <vector>
#include "metrics/logging_metric.h"
#include "metrics/metric_defs.h"
#include "metrics/transaction_metric.h"

namespace terrier::metrics {

MetricsStore::MetricsStore(const std::bitset<NUM_COMPONENTS> &enabled_metrics) : enabled_metrics_{enabled_metrics} {
  metrics_[static_cast<uint8_t>(MetricsComponent::TRANSACTION)] = std::make_unique<TransactionMetric>();
  metrics_[static_cast<uint8_t>(MetricsComponent::LOGGING)] = std::make_unique<LoggingMetric>();
}

std::array<std::unique_ptr<AbstractRawData>, NUM_COMPONENTS> MetricsStore::GetDataToAggregate() {
  std::array<std::unique_ptr<AbstractRawData>, NUM_COMPONENTS> result;

  for (uint8_t component = 0; component < NUM_COMPONENTS; component++) {
    if (enabled_metrics_.test(component)) {
      result[component] = metrics_[component]->Swap();
    }
  }

  return result;
}
}  // namespace terrier::metrics
