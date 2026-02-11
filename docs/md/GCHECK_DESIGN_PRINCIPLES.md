# gcheck Design Principles and Module Intent

This document is the high-level overview of `gcheck`: what the framework is designed to do, how modules are separated, and how they are intended to be used in testing workflows.

## 1. What `gcheck` Is

`gcheck` is a grading-oriented C++ test framework.

It is not only an assertion library. It is built to:

1. Define test cases as framework-managed units.
2. Run tests with optional prerequisites/dependencies.
3. Track correctness and points.
4. Capture execution artifacts (stdout/stderr, timing, statuses).
5. Produce structured machine-readable results (JSON).

Primary use case: automated evaluation pipelines (course/autograder style), where each test contributes to score and reporting.

## 2. Core Architectural Principle

`gcheck` separates **test orchestration** from **data generation** and **presentation/serialization**.

1. Orchestration: test lifecycle, dependencies, scoring, run control.
2. Data generation: deterministic/random input production.
3. Presentation: human-readable strings + JSON output.
4. Runtime plumbing: stream redirection and optional process isolation.

This keeps grading logic independent from how inputs are generated and how outputs are formatted.

## 3. Module Responsibilities

## 3.1 `gcheck` Core (`gcheck.h`, `gcheck.cpp`, test base classes)

Intent:

1. Own test registry and discovery.
2. Execute tests in dependency-safe order.
3. Track report entries, pass/fail counters, scoring policy.
4. Aggregate final totals.

Key behaviors:

1. Tests self-register at construction time.
2. Prerequisites gate test execution.
3. Status and points are computed from collected report data.

## 3.2 Argument Generation (`argument.h`)

Intent:

1. Generate input values for test runs.
2. Provide both deterministic and random sources.
3. Support compositional generation for complex input shapes.

Used for:

1. Randomized function/method test runs.
2. Controlled edge-case sets (discrete choices).
3. Structured tuple/container input synthesis.

Design principle:

- Input generation is a first-class concern, but independent of scoring/reporting.

## 3.3 Formatting Helpers (`stringify.h`)

Intent:

1. Convert values into readable textual representations.
2. Provide two paths:
   - `toString(...)` for display
   - `toConstruct(...)` for constructor-like reconstruction text

Used for:

1. Human-facing report tables.
2. Diagnostic info in failing test cases.

Design principle:

- Reporting readability is customizable via ADL extension points (`to_string`, `to_construct`) without changing core test logic.

## 3.4 JSON Serialization (`json.h`)

Intent:

1. Convert test artifacts and values into structured JSON.
2. Power machine-consumable reports for CI/autograders.

Used for:

1. Final score/report export.
2. Downstream tooling and post-processing.

Design principle:

- JSON output is a stable integration boundary between `gcheck` and external systems.

## 3.5 Stream Redirectors (`redirectors.h`, `redirectors.cpp`)

Intent:

1. Inject stdin for deterministic IO testing.
2. Capture stdout/stderr emitted by tested code.

Used for:

1. IO behavior verification.
2. Distinguishing functional correctness from output-format correctness.

Design principle:

- IO control is infrastructure; tests should express expected behavior, not manually manage descriptor plumbing.

## 3.6 Safe-Run / Process Isolation (`multiprocessing.h`, `shared_allocator.h`, `shared_allocator.cpp`)

Intent:

1. Run test code in child process on supported platforms (Linux path).
2. Apply timeout/crash isolation.
3. Transfer test data back using shared-memory allocator path.

Used for:

1. Preventing one faulty test target from breaking the whole test runner.
2. Enforcing execution constraints (timeouts).

Design principle:

- Isolation and resilience are optional runtime capabilities, layered below test semantics.

## 4. Why This Separation Matters

1. You can change input strategy (`argument`) without changing score policy.
2. You can improve presentation (`stringify/json`) without changing test execution.
3. You can add isolation controls without changing test definitions.

The framework remains maintainable because each module has one primary reason to change.

## 5. Testing Workflow Model

Typical end-to-end flow:

1. Define tests and expected behavior.
2. Use `argument` generators (optional) for multi-run input variation.
3. Run test body and collect report entries.
4. Capture IO/timing/status via core/runtime modules.
5. Convert results to display text (`stringify`) and JSON (`json`).
6. Compute points and publish totals.

## 6. Practical Guidance

1. Put grading/scoring rules in core test configuration, not in formatting code.
2. Use discrete random sets for edge-case-heavy checks before broad numeric ranges.
3. Extend `to_string`/`to_construct`/`to_json` for custom types instead of hardcoding display logic in tests.
4. Use safe-run where crash/time isolation is needed.

## 7. Non-Goals

`gcheck` is not primarily designed to be:

1. A minimal assertion-only framework.
2. A replacement for general large-ecosystem unit test tooling.

Its design favors grading semantics, structured reporting, and controlled execution for evaluation environments.
