#pragma once

/**
 * @file report_types.h
 * @brief Report payload types used by gcheck test execution and output.
 *
 * This header defines the canonical in-memory schema for test evidence:
 * - Single-check equality payloads (@ref EqualsData)
 * - Case-style payloads with input/output snapshots (@ref CaseEntry)
 * - Function-run payloads with timings, I/O and object state (@ref FunctionEntry)
 * - Aggregates of entries (@ref CaseData, @ref FunctionData)
 * - A tagged union wrapper for one report item (@ref TestReport)
 */

#include <chrono>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "core_types.h"
#include "multiprocessing.h"
#include "user_object.h"

namespace gcheck {

/**
 * @brief Payload for "expected == actual" assertion style checks.
 */
struct EqualsData {
	/** @brief User-object alias used for values. */
	typedef UserObject UO;
	/** @brief String type for descriptors. */
	typedef std::string string;

	/** @brief Expected value snapshot. */
	UO output_expected;
	/** @brief Actual value snapshot. */
	UO output;
	/** @brief Human-readable assertion label/details. */
	string descriptor;
	/** @brief `true` if assertion passed, otherwise `false`. */
	bool result;

	/** @brief Default constructor. */
	EqualsData() {}
	/** @brief Copy constructor. */
	EqualsData(const EqualsData &) = default;
	/** @brief Copy assignment. */
	EqualsData &operator=(const EqualsData &) = default;
};

/**
 * @brief One case entry for case-based tests.
 */
struct CaseEntry {
	/** @brief User-object alias used for stored values. */
	typedef UserObject UO;

	/** @brief Optional argument bundle used to invoke the tested call. */
	std::optional<UO> arguments;
	/** @brief Optional input representation (e.g. stdin or function input). */
	std::optional<UO> input;
	/** @brief Optional observed output value. */
	std::optional<UO> output;
	/** @brief Optional expected output value. */
	std::optional<UO> output_expected;
	/** @brief `true` when this case passes. */
	bool result;

	/** @brief Default constructor. */
	CaseEntry() {}
	/** @brief Copy constructor. */
	CaseEntry(const CaseEntry &) = default;
	/** @brief Copy assignment. */
	CaseEntry &operator=(const CaseEntry &) = default;
};
/** @brief Vector of @ref CaseEntry entries. */
using CaseData = std::vector<CaseEntry>;

/**
 * @brief One function-run entry with rich runtime and comparison data.
 */
struct FunctionEntry {
	/** @brief User-object alias used for stored values. */
	typedef UserObject UO;

	/** @brief Optional textual/structured input fed to the tested call. */
	std::optional<UO> input;
	/** @brief Optional observed stdout/output data. */
	std::optional<UO> output;
	/** @brief Optional expected stdout/output data. */
	std::optional<UO> output_expected;
	/** @brief Optional observed stderr/error data. */
	std::optional<UO> error;
	/** @brief Optional expected stderr/error data. */
	std::optional<UO> error_expected;
	/** @brief Optional argument snapshot before invocation. */
	std::optional<UO> arguments;
	/** @brief Optional argument snapshot after invocation. */
	std::optional<UO> arguments_after;
	/** @brief Optional expected argument snapshot after invocation. */
	std::optional<UO> arguments_after_expected;
	/** @brief Optional observed return value. */
	std::optional<UO> return_value;
	/** @brief Optional expected return value. */
	std::optional<UO> return_value_expected;
	/** @brief Optional object state before method invocation. */
	std::optional<UO> object;
	/** @brief Optional object state after method invocation. */
	std::optional<UO> object_after;
	/** @brief Optional expected object state after invocation. */
	std::optional<UO> object_after_expected;
	/** @brief Optional maximum allowed run time for the invocation. */
	std::optional<std::chrono::nanoseconds> max_run_time;
	/** @brief Measured run time for the invocation. */
	std::chrono::nanoseconds run_time;
	/** @brief Hard timeout used by process execution wrappers. */
	std::chrono::duration<double> timeout;
	/** @brief Process status (`OK`, `TIMEDOUT`, `ERROR`). */
	ForkStatus status = OK;
	/** @brief Overall pass/fail result for this run entry. */
	bool result;

	/** @brief Default constructor. */
	FunctionEntry() {}
	/** @brief Copy constructor. */
	FunctionEntry(const FunctionEntry &) = default;
	/** @brief Copy assignment. */
	FunctionEntry &operator=(const FunctionEntry &) = default;
};
/** @brief Vector of @ref FunctionEntry entries. */
using FunctionData = std::vector<FunctionEntry>;

/**
 * @brief Tagged wrapper for one report record.
 */
struct TestReport {
	/** @brief Stringstream type for info notes. */
	typedef std::stringstream stringstream;

	/** @brief Free-form informational message stream for this report item. */
	stringstream info_stream;
	/** @brief Variant payload containing one report representation. */
	std::variant<EqualsData, TrueData, FalseData, CaseData, FunctionData> data;

	/**
	 * @brief Copy constructor.
	 *
	 * Copies payload and textual contents of @ref info_stream.
	 */
	TestReport(const TestReport &r) : data(r.data) {
		info_stream << r.info_stream.str();
	}

	/**
	 * @brief Construct report from a concrete payload type.
	 */
	template<typename T>
	TestReport(const T &d) : data(d) {}

	/**
	 * @brief Factory helper for creating a report with a payload.
	 */
	template<typename T>
	static TestReport Make(T item = T()) {
		return TestReport(item);
	}

	/**
	 * @brief Access mutable payload of type `T`.
	 */
	template<typename T>
	T &Get() {
		return std::get<T>(data);
	}

	/**
	 * @brief Access const payload of type `T`.
	 */
	template<typename T>
	const T &Get() const {
		return std::get<T>(data);
	}
};

} // namespace gcheck
