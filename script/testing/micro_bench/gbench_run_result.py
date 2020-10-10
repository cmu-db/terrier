import json
import os
from datetime import datetime

from micro_bench.gbench_test_result import GBenchTestResult
from util.constants import LOG

class GBenchRunResult(object):
    """ The results from running a Google benchmark file (i.e. bwtree_benchmark)
        This may contain multiple individual benchmark tests. This class will
        parse the results and create a map of benchmark test results where the
        key is (suite_name, test_name) and the value is a GBenchTestResult.
    """
    def __init__(self, json_results):
        self.results = json_results
        self.benchmarks = self._populate_benchmark_results(self.results)
        self.benchmark_name = self._get_benchmark_name()


    def _populate_benchmark_results(self, results_dict):
        """ populates the benchmark map with GBenchTestResult objects and adds
            a timestamp from the higher level JSON into the individual test
            result
        """
        benchmarks = {}
        for bench in results_dict.get('benchmarks',[]):
            bench['timestamp'] = self._get_datetime()
            test_result = GBenchTestResult(bench)
            key = (test_result.suite_name, test_result.test_name)
            benchmarks[key] = test_result
        return benchmarks

    def _get_datetime(self):
        """ Return formatted datetime from the top-level JSON """
        date_format = "%Y-%m-%dT%H:%M:%S"
        default_date = datetime.now().strftime(date_format)+'+00:00'
        date_str = self.results.get('context',{}).get('date', default_date)
        try:
            return datetime.strptime(date_str[:-6], date_format)
        except:
            # google benchmark switched the date format so old files may have the old format
            old_date_format = "%Y-%m-%d %H:%M:%S"
            return datetime.strptime(date_str, old_date_format)

    def _get_benchmark_name(self):
        """ get benchmark name from the file path """
        return os.path.basename(self.results.get('context').get('executable'))

    @classmethod
    def from_benchmark_file(cls, result_file_name):
        LOG.debug("Loading local benchmark result file {}".format(result_file_name))
        with open(result_file_name) as result_file:
            try:
                raw_results = result_file.read()
                results = json.loads(raw_results)
                return cls(results)
            except Exception as err:
                LOG.error("Invalid data read from benchmark result file {}".format(result_file_name))
                LOG.error(err)
                LOG.error(raw_results)
                raise 

