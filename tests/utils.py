import subprocess
from collections import Counter

def run(binary):
    return subprocess.run(["../bin/"+binary, "--json"])

def compare_result(result, expected):
    if "type" in expected:
        if expected["type"] != result.type:
            raise Exception("Wrong result type")
    if "num_cases" in expected:
        if expected["num_cases"] != len(result.cases):
            print(expected)
            print(result)
            raise Exception("Wrong number of cases")

def compare(report, expected):
    testids = [f"{test.suite}.{test.test}" for test in report.tests]
    if Counter(testids) != Counter(expected.keys()):
        raise Exception("Test ids differ")

    for test in report.tests:
        id = f"{test.suite}.{test.test}"
        data = expected[id]
        if "num_results" in data:
            if data["num_results"] != len(test.results):
                raise Exception("Number of results is wrong")
        if "results" in data:
            res = data["results"]
            if isinstance(res, list):
                if len(res) != len(test.results):
                    raise Exception("Number of results is wrong")

                for r, e in zip(test.results, res):
                    compare_result(r, e)
            else:
                for r in test.results:
                    compare_result(r, res)
        if "points" in data:
            if data["points"] != test.points:
                raise Exception("Wrong points")
        if "max_points" in data:
            if data["max_points"] != test.max_points:
                raise Exception("Wrong max points")


