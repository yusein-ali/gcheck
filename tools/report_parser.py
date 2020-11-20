import json
from enum import Enum
from typing import Union

class Dictifiable:
    @staticmethod
    def _value(v):
        if isinstance(v, Dictifiable):
            return v.to_dict()
        elif isinstance(v, Enum):
            return v.value
        elif isinstance(v, list):
            return [Dictifiable._value(val) for val in v]
        return v

    def to_dict(self):
        d = {}
        for k, v in vars(self).items():
            d[k] = Dictifiable._value(v)
        return d

    def __iter__(self):
        for k, v in self.to_dict().items():
            yield (k, v)

class Detail(Dictifiable):
    def __init__(self, report):
        self.test = report["test"]
        self.suite = report["suite"]
        self.fullfilled = report["isfullfilled"]

class Prerequisite(Dictifiable):
    def __init__(self, report):
        self.fullfilled = report["isfullfilled"]
        self.details = [Detail(detail) for detail in report["details"]]

class Type(Enum):
    FC = 1
    TC = 2
    EE = 3
    EF = 4
    ET = 5

class ForkStatus(Enum):
    OK = 1
    TIMEDOUT = 2
    ERROR = 3

class Status(Enum):
    NotStarted = 1
    Started = 2
    Finished = 3
    TimedOut = 4

class UserObject(Dictifiable):
    def __init__(self, report):
        self.json = report["json"]
        self.string = report["string"]
        self.construct = report.get("construct", None)

class FunctionEntry(Dictifiable):
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
        self.status = ForkStatus[report["status"]]


class CaseEntry(Dictifiable):
    def __init__(self, report):
        self.result = report["result"]

        def UO_or_None(var):
            return UserObject(report[var]) if var in report else None

        self.output = UO_or_None("output")
        self.output_expected = UO_or_None("output_expected")
        self.input = UO_or_None("input")
        self.arguments = UO_or_None("arguments")

class Result(Dictifiable):
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

class Test(Dictifiable):
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

class Report(Dictifiable):
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

    def scale_points(self, max_points):
        multiplier = max_points/self.max_points
        self.points = multiplier*self.points
        self.max_points = multiplier*self.max_points
        for test in self.tests:
            test.points = multiplier*test.points
            test.max_points = multiplier*test.max_points
