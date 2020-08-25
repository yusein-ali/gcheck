import json
from enum import Enum
from typing import Union

class Prerequisite:
    def __init__(self, report):
        self.fullfilled = report["isfullfilled"]
        self.details = report["details"]

class Type(Enum):
    FC = 1
    TC = 2
    EE = 3
    EF = 4
    ET = 5

class Status(Enum):
    NotStarted = 1
    Started = 2
    Finished = 3

class UserObject:
    def __init__(self, report):
        self.json = report["json"]
        self.string = report["string"]
        self.construct = report["construct"]

class FunctionEntry:
    def __init__(self, report):
        self.result = report["result"]

        def or_None(var) -> Union[str, None]:
            return report[var] if var in report else None
        def UO_or_None(var) -> Union[UserObject, None]:
            return UserObject(report[var]) if var in report else None

        self.input = UO_or_None("input")
        self.output = UO_or_None("output")
        self.output_expected = UO_or_None("output_expected")
        self.error = UO_or_None("error")
        self.error_expected = UO_or_None("error_expected")
        self.arguments = UO_or_None("arguments")
        self.arguments_after = UO_or_None("arguments_after")
        self.arguments_after_expected = UO_or_None("arguments_after_expected")
        self.return_value = UO_or_None("return_value")
        self.return_value_expected = UO_or_None("return_value_expected")
        self.object = UO_or_None("object")
        self.object_after = UO_or_None("object_after")
        self.object_after_expected = UO_or_None("object_after_expected")
        self.max_run_time = or_None("max_run_time")
        self.run_time = or_None("run_time")
        self.timeout = or_None("timeout")
        self.timed_out = or_None("timed_out")


class CaseEntry:
    def __init__(self, report):
        self.result = report["result"]

        def UO_or_None(var):
            return UserObject(report[var]) if var in report else None

        self.output = UO_or_None("output")
        self.output_expected = UO_or_None("output_expected")
        self.input = UO_or_None("input")
        self.arguments = UO_or_None("arguments")

class Result:
    def __init__(self, report):
        self.type = Type[report["type"]]
        self.info = report["info"]
        if self.type == Type.FC:
            self.cases = [FunctionEntry(r) for r in report["cases"]]
        elif self.type ==  Type.TC:
            self.cases = [CaseEntry(r) for r in report["cases"]]
        elif self.type in [Type.EE, Type.EF, Type.ET]:
            self.result = report["result"]
            self.descriptor = report["descriptor"]
            if self.type in [Type.EF, Type.ET]:
                self.value = report["value"]
            elif self.type in [Type.EE]:
                self.output_expected = report["output_expected"]
                self.output = report["output"]

class Test:
    def __init__(self, suite, test, report):
        self.suite = suite
        self.test = test
        self.points = report["points"]
        self.max_points = report["max_points"]
        self.grading_method = report["grading_method"]
        self.format = report["format"]
        self.stdout = report["stdout"]
        self.stderr = report["stderr"]
        self.correct = report["correct"]
        self.incorrect = report["incorrect"]
        self.status = Status[report["status"]]
        self.results = [Result(r) for r in report["results"]]
        self.prerequisite = Prerequisite(report["prerequisite"])

    def get_name(self):
        return self.suite + " : " + self.test

class Report:
    points = 0
    max_points = 0
    def __init__(self, filename):
        with open(filename, 'r') as f:
            self.data = json.load(f)
            self.points = self.data["points"]
            self.max_points = self.data["max_points"]
            self.tests = [Test(suite_name, test_name, test_data) for suite_name, suite_data in self.data["test_results"].items() for test_name, test_data in suite_data.items()]

    def get_json(self):
        return json.dumps(self.data)