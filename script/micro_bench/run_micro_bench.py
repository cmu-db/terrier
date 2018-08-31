#!/usr/bin/env python

"""
Run micro-benchmarks on a PR, for the purpose of comparing performance with
the master branch.

Usage:
From the directory in which this script resides
./run_micro_bench.py
"""

import json
import os
import pprint
import subprocess
import sys
import urllib
import xml.etree.ElementTree as ElementTree

class TestConfig(object):
    def __init__(self):
        # benchmark executables to run
        self.benchmark_list = ["data_table_benchmark",
                               "tuple_access_strategy_benchmark"]
        return

    def get_benchmark_list(self):
        return self.benchmark_list

    def is_tracked(self, suite_name, test_name):
        return

    def get_tolerance(self, suite_name, test_name):
        # % difference permitted
        return

    def get_reference(self, suite_name, test_name):
        return

class Artifact(object):
    """ A Jenkins build artifact, as visible from the web api """
    def __init__(self, build_url, artifact_dict):
        self.build_url = build_url
        self.artifact_dict = artifact_dict
        return

    def get_fileName(self):
        return self.artifact_dict['fileName']

    def get_data(self):
        """ Return the contents of the artifact  """
        url = "{}artifact/{}".format(self.build_url,
                                     self.artifact_dict['relativePath'])
        data = urllib.urlopen(url).read()
        return data

class Build(object):
    """ A Jenkins build, as visible from the web api """
    def __init__(self, build_dict):
        """ build_dict : dict as returned by the web api """

        self.build_dict = build_dict
        self.artifact_list = None
        return
    
    def get_artifact_by_fileName(self, fileName):
        """ Return Artifact object with specified fileName """
        for artifact in self.artifact_list:
            if artifact.get_fileName() == fileName:
                # return Artifact object
                return artifact
        return None

    def get_artifacts(self):
        """ Return a list of Artifact (objects) """
        build_url = self.get_build_url()

        # get the list of artifacts
        python_url = "{}/api/python".format(build_url)
        data = eval(urllib.urlopen(python_url).read())
        artifacts_lod = data['artifacts']
        """ returns a list of artifact dictionaries. These look like:
        
            {'displayPath': 'data_table_benchmark.json',
             'fileName': 'data_table_benchmark.json',
             'relativePath': 'script/micro_bench/data_table_benchmark.json'
            }
        """
        # turn them into Artifact objects
        self.artifact_list = []
        for item in artifacts_lod:
            self.artifact_list.append(Artifact(build_url, item))
        return self.artifact_list

    def get_build_url(self):
        """ return: the url for this build """
        return self.build_dict['url']

    def get_number(self):
        """ return the build number """
        return self.build_dict['number']

    def get_result(self):
        """ Return build result, SUCCESS, FAILURE or ABORTED"""
        return self.build_dict['result']

    def has_artifact_fileName(self, fileName):
        """ Does an artifact with the specified fileName, exist in
            this build
        """
        for artifact in self.artifact_list:
            if artifact.get_fileName() == fileName:
                return True
        return False

class ArtifactProcessor(object):
    """ Compute summary stats from Google Benchmark results.
        Provide access by (suite_name, test_name)
    """
    def __init__(self):
        # key = (suite_name, test_name)
        self.results = {}
        return

    def add_artifact_file(self, data):
        """
        data : raw json data from the artifact file
        """
        
        # create a GBResult
        gbr = GBResult(json.loads(data))
        # iterate over the GBBenchResult objects
        for bench_result in gbr.benchmarks:
            key = (bench_result.get_suite_name(),
                   bench_result.get_test_name())

            # add to a GBBenchResultProcessor
            gbr_p = self.results.get(key)
            if gbr_p is None:
                gbr_p = GBBenchResultProcessor()
                self.results[key] = gbr_p
                
            gbr_p.add_gbresult(bench_result)
        return

    def get_result(self, suite_name, test_name):
        """ Return a GBBenchResultProcessor, that can supply 
            summarized stats
        """
        key = (suite_name, test_name)
        if not self.results.has_key(key):        
            raise RuntimeError("key {} not present".format(key))        
        return self.results[key]
   
class GBResult(object):
    """ Holds the results from a single GoogleBench output file,
        which may have 1 or more benchmarks
    """
    def __init__(self, data):
        """ data - raw contents of the file """
        self.data = data
        # benchmark data converted to objects
        self.benchmarks = []
        
        self._init_benchmark_objects()
        return

    def _init_benchmark_objects(self):
        for bench in self.data['benchmarks']:
            self.benchmarks.append(GBBenchResult(bench))
            
        return

class GBBenchResult(object):
    """Holds a single test result """
    def __init__(self, result_dict):
        """result_dict: single test dict, from json Google Benchmark <file>.json
        """

        # sample input below
        """
        "name": "DataTableBenchmark/SimpleInsert/real_time",
        "iterations": 5,
        "real_time": 1.2099044392001815e+03,
        "cpu_time": 1.2098839266000000e+03,
        "time_unit": "ms",
        "items_per_second": 8.2651155545892473e+06
        """
        self.attrs = set()
        for k, v in result_dict.items():
            setattr(self, k, v)
            self.attrs.add(k)
        self._process_name()
        return

    def _process_name(self):
        """Split name into components"""
        parts = self.name.split("/")
        self.suite_name = parts[0]
        self.attrs.add("suite_name")

        self.test_name = parts[1]
        self.attrs.add("test_name")

        if len(parts) == 3:
            self.time_type = parts[2]
            self.attrs.add("time_type")
        return

    def get_suite_name(self):
        return self.suite_name

    def get_test_name(self):
        return self.test_name

    def get_time(self):
        time_attr = getattr(self, "time_type", "cpu_time")
        return getattr(self, time_attr)

    def get_time_unit(self):
        return self.time_unit

    def get_items_per_second(self):
        return self.items_per_second

    def __str__(self):
        st = ""
        for k in self.attrs:
            st = st + "{} : {}\n".format(k, getattr(self,k))
        return st

class GBBenchResultProcessor(object):
    """ Compute selected statistics from a list of GBBenchResult objects
        Computes:
        - mean time (real or cpu, autoselected)
        - mean items_per_second
        - std deviation
    """
    def __init__(self):
        self.test_suite = None
        self.test_name = None
        self.time_unit = None
        self.num_results = 0
        
        self.sum_time = 0.0
        self.sum_items_per_second = 0.0
        return

    def add_gbresult(self, gb_result):
        """ add a result, ensuring we have a valid input, consistent
            with results being accumulated
        """
        # check test suite name
        if self.test_suite:
            assert(self.test_suite == gb_result.get_suite_name())
        else:
            self.test_suite = gb_result.get_suite_name()

        # check test name
        if self.test_name:
            assert(self.test_name == gb_result.get_test_name())
        else:
            self.test_name = gb_result.get_test_name()

        # check units
        if self.time_unit:
            assert(self.time_unit == gb_result.get_time_unit())
        else:
            self.time_unit = gb_result.get_time_unit()

        # ok to use
        self.sum_time += gb_result.get_time()
        self.sum_items_per_second = gb_result.get_items_per_second()
        self.num_results += 1
        return

    def get_mean_time(self):
        return self.sum_time/self.num_results

    def get_mean_items_per_second(self):
        return self.sum_items_per_second/self.num_results

    def get_num_items(self):
        return self.num_results

    def get_suite_name(self):
        return self.test_suite

    def get_test_name(self):
        return self.test_name

    def get_time_unit(self):
        return self.time_unit

class GBenchToJUnit(object):
    """Convert a Google Benchmark output file (json) into Junit output file format (xml)
    """
    def __init__(self, input_file, output_file):
        self.input_file = input_file
        self.output_file = output_file

        testsuite_dict = self.read_gb_results(self.input_file)
        self.write_output(self.output_file, testsuite_dict)
        return

    def read_gb_results(self, input_file):
        
        # suite level attributes:
        # errors
        
        # failures
        # name (of suite)?
        # skipped
        # tests (count)
        # timestamp
        # time (duration)

        # for each testcase
        # classname = suitname?
        # name = of test
        # time or perf measure?

        testcases = []
        test_suite = { "testcases" : testcases }

        # read the results file                                                 
        with open(input_file) as rf:
            gb_data = json.load(rf)

        # convert to internal, intermediate form                                
        bench_list = gb_data["benchmarks"]
        for bench in bench_list:
            # bench_name = bench["name"]
            one_test_dict = GBBenchResult(bench)

            testcases.append(one_test_dict)

        # pull out the suite_name from the first testcase                       
        assert(len(testcases) > 0)
        test_suite["name"] = testcases[0].suite_name

        self._annotate_test_suite(test_suite)
        # returns a dictionary
        return test_suite

    def _annotate_test_suite(self, suite):
        suite["errors"] = "0"
        suite["failures"] = "0"
        suite["skipped"] = "0"
        suite["tests"] = str(len(suite["testcases"]))
        return
    
    def write_output(self, output_file, testsuite_dict):
        tree = ElementTree.ElementTree()

        test_suite_el = ElementTree.Element("testsuite")
        tree._setroot(test_suite_el)

        # add attributes to root, testsuite element                             
        for el_name in ["errors",
                        "failures",
                        "skipped",
                        "tests",
                        "name"]:
            test_suite_el.set(el_name, testsuite_dict[el_name])

        # add tests                                                             
        for test in testsuite_dict["testcases"]:
            test_el = ElementTree.SubElement(test_suite_el,"testcase")
            test_el.set("classname", getattr(test, "suite_name"))
            test_el.set("name", getattr(test,"test_name"))
            # set time based on real_time or cpu_time
            test_el.set("time", str(getattr(test, "real_time")))
            # not in schema
            # test_el.set("items_per_second", str(getattr(test, "items_per_second")))
        tree.write(self.output_file, xml_declaration=True, encoding='utf8')
        return

class RunMicroBenchmarks(object):
    """ Run micro benchmarks. Output is to json files for post processing.
        Returns True if all benchmarks run, False otherwise
    """
    def __init__(self):
        # list of benchmarks to run
        self.benchmark_list = ["data_table_benchmark",
                               "tuple_access_strategy_benchmark"]

        # minimum run time for the benchmark
        self.min_time = 10
        return

    def run_all_benchmarks(self):
        """ Return 0 if all benchmarks succeed, otherwise return the error code
            code from the last benchmark to fail
        """
        ret_val = 0

        # iterate over all benchmarks and run them
        for benchmark_name in self.benchmark_list:
            bench_ret_val = self.run_single_benchmark(benchmark_name)
            if bench_ret_val:
                print "{} terminated with {}".format(benchmark_name, bench_ret_val)
                ret_val = bench_ret_val

        # return fail, if any of the benchmarks failed to run or complete
        return ret_val

    def run_single_benchmark(self, benchmark_name):
        """ Run benchmark, generate JSON results
        """
        benchmark_path = os.path.join("../../build/release", benchmark_name)
        output_file = "{}.json".format(benchmark_name)

        cmd = "{} --benchmark_min_time={} " + \
              " --benchmark_format=json" + \
              " --benchmark_out={}"
        cmd = cmd.format(benchmark_path,
                         self.min_time,
                         output_file)

        ret_val = subprocess.call([cmd],
                                  shell=True,
                                  stdout=sys.stdout,
                                  stderr=sys.stderr)

        # Do we need to check results from this benchmark?
        # Get result against which to compare
        # Get tolerance
        # check if within tolerance, and fail if not.
        # ?? Insert failure into xml? Yes.

        # convert json results file to xml
        xml_output_file = "{}.xml".format(benchmark_name)
        gb_to_ju = GBenchToJUnit(output_file, xml_output_file)

        # return the process exit code
        return ret_val

class Jenkins(object):
    def __init__(self, base_url="http://jenkins.db.cs.cmu.edu:8080"):
        self.base_url = base_url
        return

    def get_builds(self, project, branch):
        """ 
        Get the list of builds for the specified project/branch

        Parameters:
        project : string
            Name of project, e.g. Peloton
        branch : string
            Branch desired

        Returns a list of Build objects
        """
        url = "{}/job/{}/job/{}".format(self.base_url, project, branch)
        python_url = "{}/api/python".format(url)
        data = eval(urllib.urlopen(python_url).read())

        """ Return a list of build dictionaries. These appear to be by 
            descending build number

            Each build dict looks like:
            {'_class': 'org.jenkinsci.plugins.workflow.job.WorkflowRun',
             'number': 8,
             'url': 'http://jenkins.db.cs.cmu.edu:8080/job/pa_terrier/job/micro_bench/8/'},
        """

        # retrieve data for each build and turn into a Build object
        ret_list = []
        for item in data['builds']:
            build_url = "{}/api/python".format(item['url'])
            data = eval(urllib.urlopen(build_url).read())            
            ret_list.append(Build(data))
        return ret_list

    # deprecate
    def get_artifact_list(self, build_dict):
        """ Return the list of artifacts from a single build """
        
        python_url = "{}/api/python".format(build_dict['url'])
        data = eval(urllib.urlopen(python_url).read())
        artifacts_lod = data['artifacts']

        """ Return a list of artifact dictionaries. These look like:
        
            {'displayPath': 'data_table_benchmark.json',
             'fileName': 'data_table_benchmark.json',
             'relativePath': 'script/micro_bench/data_table_benchmark.json'
            }
        """
        return artifacts_lod

    # deprecate    
    def has_artifact_name(self, build_dict, artifact_name):
        artifact_list = self.get_artifact_list(build_dict)
        for artifact in artifact_list:
            if artifact['fileName'] == artifact_name:
                return True
        return False

    # deprecate
    def get_artifact(self, artifact_url):
        data = urllib.urlopen(artifact_url).read()
        return data

    def get_n_builds_with_artifacts(self, artifact_name, n):
        # define a build object
        # define an artifact object
        # return a list of builds, which have attached artifacts, satisfying the
        # request for n builds with specified artifact name
        return

    # -----

    # deprecate
    def get_build_artifacts(self, build_list):
        for build in build_list:
            build_url = build['url']
            artifact_lod = self.get_artifact_list(build)
            for artifact_dict in artifact_lod:
                url = "{}artifact/{}".format(build_url, artifact_dict['relativePath'])
                print url
                data = urllib.urlopen(url).read()
                print data
        return

    def debug_print(self, data):
        pp = pprint.PrettyPrinter()
        pp.pprint(data)
        return

if __name__ == "__main__":
    run_bench = RunMicroBenchmarks()
    ret = run_bench.run_all_benchmarks()

    ap = ArtifactProcessor()

    h = Jenkins()
    builds = h.get_builds("pa_terrier", "micro_bench")
    # filter down to successful builds
    builds = [build for build in builds if build.get_result() == "SUCCESS"]
    
    for build in builds:
        print "build {}, status {}".format(build.get_number(),
                                           build.get_result())
        artifacts = build.get_artifacts()
        for artifact in artifacts:
            artifact_filename = artifact.get_fileName()
            print "artifact: {}".format(artifact_filename)
            
            ap.add_artifact_file(artifact.get_data())

    for key in ap.results.keys():
        suite_name, test_name = key
        v = ap.get_result(suite_name, test_name)
        print key
        print v.get_suite_name()
        print v.get_test_name()
        print v.get_mean_time()
        print v.get_num_items()
    sys.exit(ret)

