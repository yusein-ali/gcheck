#pragma once

/**
 * @file user_object.h
 * @brief Generic value wrapper that stores both JSON and human-readable string forms.
 *
 * This type is used extensively in report payloads to preserve test values in a
 * serialization-friendly and display-friendly form without coupling report code
 * to concrete user value types.
 *
 * Conversion behavior for incoming values:
 * 1. JSON form is produced via @ref gcheck::JSON constructors.
 * 2. String form is produced via `toString(...)` helpers from `stringify.h`.
 * 3. Optional construct-expression form is produced when
 *    `GCHECK_CONSTRUCT_DATA` is enabled.
 */

#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "argument.h"
#include "json.h"
#include "sfinae.h"
#include "stringify.h"

namespace gcheck {
/**
 * @brief Wrapper for values recorded in test reports.
 *
 * `UserObject` stores one logical value in multiple representations:
 * - JSON representation for machine-readable exports.
 * - String representation for human-readable output tables.
 * - Optional construct-expression representation (`GCHECK_CONSTRUCT_DATA`).
 *
 * Typical usage scenario:
 * 1. Test/report code assigns arbitrary values (`int`, `std::string`, tuple, etc.).
 * 2. `UserObject` captures both JSON and display string forms.
 * 3. Formatter/JSON code retrieves `string()` / `json()` as needed.
 *
 * Example:
 * @code{.cpp}
 * gcheck::UserObject o(42);
 * std::string shown = o.string(); // e.g. "42"
 * gcheck::JSON j = o.json();       // JSON number/string/object based on value
 * @endcode
 *
 */
class UserObject {
public:
	/** @brief Default constructor (empty/default JSON and string state). */
	UserObject() {}
	/** @brief Copy constructor. */
	UserObject(const UserObject &v) = default;
	/** @brief Copy assignment. */
	UserObject &operator=(const UserObject &v) = default;

	/**
	 * @brief Deleted constructor to avoid ambiguous optional-wrapped conversions.
	 *
	 * Prevents accidental wrapping of `optional<UserObject>` in places where
	 * raw values or `UserObject` are expected.
	 *
	 * @param item Optional user-object wrapper (deleted overload).
	 */
	UserObject(const std::optional<UserObject> &item) = delete;
	/**
	 * @brief Construct from an arbitrary value type.
	 *
	 * Stores:
	 * - JSON form in @ref as_json_
	 * - display string in @ref as_string_
	 * - optional construct-form when enabled
	 *
	 * @tparam T Input value type.
	 * @param item Input value.
	 */
	template<typename T>
	UserObject(const T &item) {
		as_json_ = item;
		as_string_ = toString(item);
#ifdef GCHECK_CONSTRUCT_DATA
		construct_ = toConstruct(item);
#endif
	}
	/**
	 * @brief Variadic convenience constructor that packs arguments into a tuple.
	 *
	 * Typical usage:
	 * - Capture multi-argument call state as one object.
	 *
	 * @tparam Args Argument value types.
	 * @param items Values to pack.
	 */
	template<typename... Args>
	UserObject(const Args &...items) : UserObject(std::tuple<Args...>(items...)) {}

	/**
	 * @brief Get JSON representation.
	 *
	 * @return Stored JSON value.
	 */
	JSON json() const {
		return as_json_;
	}
	/**
	 * @brief Get human-readable string representation.
	 *
	 * @return Stored string form.
	 */
	std::string string() const {
		return (std::string) as_string_;
	}
#ifdef GCHECK_CONSTRUCT_DATA
	/**
	 * @brief Get construct-expression representation when enabled.
	 *
	 * @return Expression-like string that can recreate the value (best effort).
	 */
	std::string construct() const {
		return (std::string) construct_;
	}
#endif

	/**
	 * @brief Assign from arbitrary value by rebuilding wrapper state.
	 *
	 * Equivalent to `*this = UserObject(v)`.
	 *
	 * @tparam T Input value type.
	 * @param v Value to store.
	 * @return Reference to `*this`.
	 */
	template<typename T>
	UserObject &operator=(const T &v) {
		return *this = UserObject(v);
	}

private:
	/** @brief Human-readable string representation of wrapped value. */
	std::string as_string_;
	/** @brief JSON representation of wrapped value. */
	JSON as_json_;
#ifdef GCHECK_CONSTRUCT_DATA
	/** @brief Optional construct-expression text, e.g. `std::vector<int>({0, 1, 2})`. */
	std::string construct_;
#endif
};

} // namespace gcheck
