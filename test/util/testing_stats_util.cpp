#include "util/testing_stats_util.h"
#include "storage/metric/stats_aggregator.h"
#include "storage/metric/test_metric.h"

namespace terrier {

int TestingStatsUtil::AggregateTestCounts() {
  stats::StatsAggregator aggregator(nullptr);
  auto result = aggregator.AggregateRawData();

  if (result.empty()) return 0;

  for (auto &raw_data : result) {
    if (raw_data->GetMetricType() == stats::MetricType::TEST) {
      return dynamic_cast<stats::TestMetricRawData *>(raw_data.get())->GetCount();
    }
  }
  return 0;
}

}  // namespace terrier
