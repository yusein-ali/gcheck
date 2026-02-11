#pragma once

/**
 * @file core_types.h
 * @brief Fundamental gcheck data types with minimal internal dependencies.
 *
 * This header contains lightweight, reusable building blocks that can be
 * included without pulling in the full test framework:
 * - Generic test-result wrapper (@ref Result)
 * - Primitive assertion payloads (@ref TrueData, @ref FalseData)
 * - Common grading and lifecycle enums (@ref GradingMethod, @ref TestStatus)
 */

#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace gcheck {

/**
 * @brief Generic container for function execution output and optional input snapshot.
 *
 * @tparam T  Output value type.
 * @tparam S  First input element type (defaults to `std::string`).
 * @tparam Ss Remaining input element types.
 */
template<typename T, typename S = std::string, typename... Ss>
struct Result {
	/** @brief Tuple type used to store captured input values. */
	typedef std::tuple<S, Ss...> InputT;

	/** @brief Produced output value from tested call. */
	T output;
	/** @brief Captured error text associated with execution. */
	std::string error;

	/** @brief Default-construct output. */
	Result() : output() {}
	/** @brief Construct with a concrete output value. */
	Result(T out) : output(out) {}

	/** @brief Copy constructor. */
	Result(const Result &r) = default;

	/** @brief Assign directly from output value. */
	Result &operator=(const T &item) {
		output = item;
		return *this;
	}
	/** @brief Copy assignment. */
	Result &operator=(const Result &r) = default;

	/**
     * @brief Store the input tuple that produced this result.
     *
     * @param first First input argument.
     * @param args Remaining input arguments.
     */
	void SetInput(const S &first, const Ss &...args) {
		input = std::tuple(first, args...);
	}
	/** @brief Access stored input tuple when available. */
	const std::optional<InputT> &GetInput() const {
		return input;
	}

private:
	/** @brief Optional snapshot of input arguments. */
	std::optional<InputT> input;
};

/**
 * @brief Payload for "expect true" style checks.
 *
 * Stores:
 * - observed boolean value
 * - descriptor string
 * - final pass/fail result
 *
 */
struct TrueData {
	/** @brief String type for descriptor storage. */
	typedef std::basic_string<char, std::char_traits<char>, std::allocator<char>> string;
	/** @brief Observed boolean value under test. */
	bool value;
	/** @brief Human-readable assertion description. */
	string descriptor;
	/** @brief Final evaluation outcome for this check. */
	bool result;

	/** @brief Default constructor. */
	TrueData() {}

	/** @brief Copy constructor. */
	TrueData(const TrueData &d) = default;
	/** @brief Copy assignment operator. */
	TrueData &operator=(const TrueData &d) = default;
};


/**
 * @brief Payload for "expect false" style checks.
 *
 * Stores:
 * - observed boolean value
 * - descriptor string
 * - final pass/fail result
 *
 */
struct FalseData {
	/** @brief String type for descriptor storage. */
	typedef std::basic_string<char, std::char_traits<char>, std::allocator<char>> string;
	/** @brief Observed boolean value under test. */
	bool value;
	/** @brief Human-readable assertion description. */
	string descriptor;
	/** @brief Final evaluation outcome for this check. */
	bool result;

	/** @brief Default constructor. */
	FalseData() {}

	/** @brief Copy constructor. */
	FalseData(const FalseData &d) = default;
	/** @brief Copy assignment operator. */
	FalseData &operator=(const FalseData &d) = default;
};

/** @brief Scoring aggregation policy used when converting checks into points. */
enum GradingMethod {
	/** @brief Score proportional to passed checks. */
	Partial,
	/** @brief Full points only if all checks pass. */
	AllOrNothing,
	/** @brief Full points if passes are at least failures. */
	Most,
	/** @brief Full points only if passes strictly exceed failures. */
	StrictMost
};

/** @brief Execution lifecycle state for a test case. */
enum TestStatus : int {
	/** @brief Test has not started yet. */
	NotStarted,
	/** @brief Test execution is currently in progress. */
	Started,
	/** @brief Test was interrupted due to timeout. */
	TimedOut,
	/** @brief Test finished execution normally. */
	Finished
};

} // namespace gcheck
