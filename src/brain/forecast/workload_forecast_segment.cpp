#include "brain/forecast/workload_forecast_segment.h"

#include <fstream>
#include <utility>

#include "execution/exec_defs.h"
#include "parser/expression/constant_value_expression.h"

namespace noisepage::brain {

WorkloadForecastSegment::WorkloadForecastSegment(std::unordered_map<execution::query_id_t, uint64_t> id_to_num_exec)
    : id_to_num_exec_(id_to_num_exec) {}

}  // namespace noisepage::brain