#!/usr/bin/env python3

import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
sys.path.insert(1, os.path.join(sys.path[0], '../../tools'))

from utils import run, compare
from report_parser import Report, Type

process = run("function_test")
report = Report("report.json")

expect = {
    "basic.VoidAndEmpty": {
        "points": 1,
        "max_points": 1,
        "results": {
            "type": Type.FC,
            "num_cases": 3,
        },
    },
    "basic.IntAndEmpty": {
        "points": 3,
        "max_points": 3,
        "results": {
            "type": Type.FC,
            "num_cases": 2,
        },
    },
    "basic.VoidAndIntInt": {
        "points": 4,
        "max_points": 4,
        "results": {
            "type": Type.FC,
            "num_cases": 4,
        },
    },
    "basic.IntAndIntInt": {
        "points": 5,
        "max_points": 5,
        "results": {
            "type": Type.FC,
            "num_cases": 3,
        },
    },
    "values.IntAndEmpty2": {
        "points": 4,
        "max_points": 4,
        "results": [{
            "type": Type.FC,
            "num_cases": 3,
        }],
    },
    "values.VoidAndIntInt2": {
        "points": 4,
        "max_points": 4,
        "results": [{
            "type": Type.FC,
            "num_cases": 3,
        }],
    },
    "values.IntAndIntInt2": {
        "points": 4,
        "max_points": 4,
        "results": [{
            "type": Type.FC,
            "num_cases": 3,
        }],
    },
    "values.VoidAndIntInt2_fail": {
        "points": 0,
        "max_points": 4,
        "results": [{
            "type": Type.FC,
            "num_cases": 3,
        }],
    },
    "values.IntAndIntInt2_fail": {
        "points": 0,
        "max_points": 4,
        "results": [{
            "type": Type.FC,
            "num_cases": 3,
        }],
    },
}

compare(report, expect)
