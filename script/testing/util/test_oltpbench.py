#!/usr/bin/python3
import os
import sys
import subprocess
import json
from util import constants
from util.common import run_command
from util.test_server import TestServer
from xml.etree import ElementTree


class TestOLTPBench(TestServer):
    """ Class to run OLTP Bench tests """
    def __init__(self, args):
        TestServer.__init__(self, args)

        # oltpbench specific attributes
        self.benchmark = str(self.args.get("benchmark"))
        self.scalefactor = int(
            self.args.get("scalefactor", constants.OLTP_DEFAULT_SCALEFACTOR))
        self.terminals = int(
            self.args.get("terminals", constants.OLTP_DEFAULT_TERMINALS))
        self.time = int(
            self.args.get("client_time", constants.OLTP_DEFAULT_TIME))
        self.weights = str(self.args.get("weights"))
        self.transaction_isolation = str(
            self.args.get("transaction_isolation",
                          constants.OLTP_DEFAULT_TRANSACTION_ISOLATION))

        # oltpbench xml file paths
        xml_file = "{}_config.xml".format(self.benchmark)
        self.xml_config = os.path.join(constants.OLTP_DIR_CONFIG, xml_file)
        self.xml_template = os.path.join(constants.OLTP_DIR_CONFIG,
                                         "sample_{}".format(xml_file))

        # oltpbench test results
        self.test_output_file = self.args.get("test_output_file")
        if not self.test_output_file:
            self.test_output_file = "outputfile_{WEIGHTS}_{SCALEFACTOR}".format(
                WEIGHTS=self.weights.replace(",", "_"),
                SCALEFACTOR=str(self.scalefactor))
            # TODO: ask Andy to remove the relative path from the oltpbench for execution and result logging
            # self.test_output_file = os.path.join(
            #     constants.OLTP_DIR_TEST_RESULT, self.result_path)

        # oltpbench json format test results 
        self.test_output_json_file = self.args.get("test_output_json_file")
        if not self.test_output_json_file:
            self.test_output_json_file = "resultjsonfile_{WEIGHTS}_{SCALEFACTOR}".format(
                WEIGHTS=self.weights.replace(",", "_"),
                SCALEFACTOR=str(self.scalefactor)) 

        # oltpbench json result file paths
        self.json_result=os.path.join(constants.OLTP_GIT_LOCAL_PATH, self.test_output_json_file)

        # oltpbench test command
        self.test_command = "{BIN} -b {BENCHMARK} -c {XML} {FLAGS} -o {RESULTS} -json-histograms {JSONRESULTS}".format(
            BIN=constants.OLTP_DEFAULT_BIN,
            BENCHMARK=self.benchmark,
            XML=self.xml_config,
            FLAGS=constants.OLTP_DEFAULT_COMMAND_FLAGS,
            RESULTS=self.test_output_file,
            JSONRESULTS=self.test_output_json_file)
        self.test_command_cwd = constants.OLTP_GIT_LOCAL_PATH
        self.test_error_msg = constants.OLTP_TEST_ERROR_MSG

    def run_pre_test(self):
        # install the OLTP
        self.install_oltp()
        self.config_xml_file()
        self.create_result_dir()

    def run_post_test(self):
        # validate the OLTP result
        self.validate_json_result()
    
    def create_result_dir(self):
        if not os.path.exists(constants.OLTP_DIR_TEST_RESULT):
            os.mkdir(constants.OLTP_DIR_TEST_RESULT)

    def install_oltp(self):
        self.clean_oltp()
        self.download_oltp()
        self.build_oltp()

    def clean_oltp(self):
        rc, stdout, stderr = run_command(constants.OLTP_GIT_CLEAN_COMMAND,
                                         "Error: unable to clean OLTP repo")
        if rc != constants.ErrorCode.SUCCESS:
            print(stderr)
            sys.exit(rc)

    def download_oltp(self):
        rc, stdout, stderr = run_command(
            constants.OLTP_GIT_COMMAND,
            "Error: unable to git clone OLTP source code")
        if rc != constants.ErrorCode.SUCCESS:
            print(stderr)
            sys.exit(rc)

    def build_oltp(self):
        for command in constants.OTLP_ANT_COMMANDS:
            error_msg = "Error: unable to run \"{}\"".format(command)
            rc, stdout, stderr = run_command(command, error_msg)
            if rc != constants.ErrorCode.SUCCESS:
                print(stderr)
                sys.exit(rc)

    def config_xml_file(self):
        xml = ElementTree.parse(self.xml_template)
        root = xml.getroot()
        root.find("dbtype").text = constants.OLTP_DEFAULT_DBTYPE
        root.find("driver").text = constants.OLTP_DEFAULT_DRIVER
        root.find(
            "DBUrl"
        ).text = "jdbc:postgresql://{}:{}/terrier?preferQueryMode=simple".format(
            self.db_host, self.db_port)
        root.find("username").text = constants.OLTP_DEFAULT_USERNAME
        root.find("password").text = constants.OLTP_DEFAULT_PASSWORD
        root.find("isolation").text = str(self.transaction_isolation)
        root.find("scalefactor").text = str(self.scalefactor)
        root.find("terminals").text = str(self.terminals)
        for work in root.find("works").findall("work"):
            work.find("time").text = str(self.time)
            work.find("rate").text = str(constants.OLTP_DEFAULT_RATE)
            work.find("weights").text = str(self.weights)

        xml.write(self.xml_config)

    def validate_json_result(self):
        # read the results file
        with open(self.json_result) as jr:
            jr_data=json.load(jr)
        jr_unexpected=jr_data["unexpected"]
        jr_hitogram=jr_unexpected["HISTOGRAM"]
        for key in jr_hitogram.keys():
            if(jr_hitogram[key] != 0):
                print(jr_hitogram)
                sys.exit(constants.ErrorCode.ERROR) 