# gcheck JSON Fields Reference

This document describes JSON fields produced by gcheck, based on the serializer implementation in `src/json.cpp`.

## Top-level `TestData` object

Serialized by `_JSON(const TestData&)`.

Fields:
- `results`: array of report items (`TestReport` JSON objects).
- `grading_method`: grading mode value from `GradingMethod`.
- `prerequisite`: prerequisite status object.
- `format`: output format string.
- `points`: awarded points.
- `max_points`: maximum points.
- `stdout`: captured stdout text.
- `stderr`: captured stderr text.
- `correct`: number of passing checks/cases.
- `incorrect`: number of failing checks/cases.
- `status`: test lifecycle status string.

## `results[]` item (`TestReport`)

Each report object contains:
- `type`: statically enumerated report kind code.
- payload fields (depend on `type`).
- `info`: free-form info string.

### Statically enumerated `type` codes

The following values are hardcoded in `src/json.cpp`:
- `EE`: equals-style assertion payload (`EqualsData`).
- `ET`: true-check payload (`TrueData`).
- `EF`: false-check payload (`FalseData`).
- `TC`: case-test payload (`CaseData`).
- `FC`: function/method/io run payload (`FunctionData`).

### Payload fields by `type`

#### `type = "EE"`
- `output_expected`
- `output`
- `result`
- `descriptor`

#### `type = "ET"`
- `value`
- `result`
- `descriptor`

#### `type = "EF"`
- `value`
- `result`
- `descriptor`

#### `type = "TC"`
- `cases`: array of `CaseEntry` objects.

#### `type = "FC"`
- `cases`: array of `FunctionEntry` objects.

## `CaseEntry` object (inside `TC.cases`)

Serialized by `_JSON(const CaseEntry&)`.

Possible fields:
- `input` (optional)
- `output` (optional)
- `output_expected` (optional)
- `arguments` (optional)
- `result` (always present)

Note: optional fields are omitted when no value exists.

## `FunctionEntry` object (inside `FC.cases`)

Serialized by `_JSON(const FunctionEntry&)`.

Possible fields:
- `input` (optional)
- `output` (optional)
- `output_expected` (optional)
- `error` (optional)
- `error_expected` (optional)
- `arguments` (optional)
- `arguments_after` (optional)
- `arguments_after_expected` (optional)
- `return_value` (optional)
- `return_value_expected` (optional)
- `object` (optional)
- `object_after` (optional)
- `object_after_expected` (optional)
- `run_time` (always present, integer nanoseconds)
- `timeout` (always present, floating-point seconds)
- `status` (always present, fork status string)
- `result` (always present, boolean)

Note: `max_run_time` exists in C++ (`FunctionEntry`) but is currently not serialized to JSON.

## `prerequisite` object

Serialized by `_JSON(const Prerequisite&)`.

Fields:
- `isfullfilled`: overall prerequisite fulfillment flag.
- `details`: array of objects containing:
  - `suite`
  - `test`
  - `isfullfilled`

Note: the key spelling is `isfullfilled` in code (double `l`) and appears exactly like that in output.

## Statically enumerated status values

### `FunctionEntry.status` (`ForkStatus`)
Hardcoded string outputs:
- `OK`
- `TIMEDOUT`
- `ERROR`

### top-level `status` (`TestStatus`)
Hardcoded string outputs:
- `NotStarted`
- `Started`
- `TimedOut`
- `Finished`
- `ERROR` (fallback/default branch)

## UserObject JSON shape

When a field stores a `UserObject`, it serializes as:
- `json`: structured JSON rendering (if available)
- `string`: string rendering
- `construct`: constructor-like representation (only when `GCHECK_CONSTRUCT_DATA` is enabled at compile time)

## Minimal example skeleton

```json
{
  "results": [
    {
      "type": "FC",
      "cases": [
        {
          "arguments": { "json": "...", "string": "..." },
          "run_time": 12345,
          "timeout": 1.0,
          "status": "OK",
          "result": true
        }
      ],
      "info": ""
    }
  ],
  "grading_method": "...",
  "prerequisite": {
    "isfullfilled": true,
    "details": []
  },
  "format": "vertical",
  "points": 1,
  "max_points": 1,
  "stdout": "",
  "stderr": "",
  "correct": 1,
  "incorrect": 0,
  "status": "Finished"
}
```
