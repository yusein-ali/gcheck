#include <string>
#include <tuple>

#include <gcheck/gcheck.h>
#include <gcheck/method_test.h>

/**
 * @brief Example class with private state.
 *
 * The key point is that MethodTest does not bypass C++ access control.
 * You verify private state either:
 * 1) indirectly via public behavior (preferred), or
 * 2) via explicit friend probe/comparison helpers you define.
 */
class BankAccount {
public:
	explicit BankAccount(int initial_balance = 0) : balance_(initial_balance), locked_(false) {}

	void Deposit(int amount) {
		if (amount > 0 && !locked_) {
			balance_ += amount;
		}
	}

	bool Withdraw(int amount) {
		if (locked_ || amount < 0 || amount > balance_) {
			locked_ = true;
			return false;
		}
		balance_ -= amount;
		return true;
	}

	// Public observable API for behavior-level checks.
	int Balance() const {
		return balance_;
	}
	bool IsLocked() const {
		return locked_;
	}

	// Needed if you use SetObjectAfter(expected_object).
	friend bool operator==(const BankAccount &lhs, const BankAccount &rhs) {
		return lhs.balance_ == rhs.balance_ && lhs.locked_ == rhs.locked_;
	}

private:
	int balance_;
	bool locked_;

	// Optional: controlled test-only/private introspection.
	friend struct BankAccountProbe;
};

/**
 * @brief Explicit probe for private members.
 *
 * This keeps member access restricted while allowing targeted checks
 * in tests that require exact internal-state diagnostics.
 */
struct BankAccountProbe {
	static int balance(const BankAccount &a) {
		return a.balance_;
	}
	static bool locked(const BankAccount &a) {
		return a.locked_;
	}
};

// Method-like callables (MethodTest expects ReturnT(ObjectType*, Args...)).
void TDeposit(BankAccount *account, int amount) {
	account->Deposit(amount);
}
bool TWithdraw(BankAccount *account, int amount) {
	return account->Withdraw(amount);
}

/**
 * @brief Uses SetObjectAfter to validate full object state after mutation.
 */
class DepositChangesStateTest : public gcheck::MethodTest<void, BankAccount, int> {
public:
	DepositChangesStateTest() :
		gcheck::MethodTest<void, BankAccount, int>(
				gcheck::TestInfo("MethodPrivate", "DepositChangesState"), 1, TDeposit) {}

private:
	void SetInputsAndOutputs() override {
		current_ = BankAccount(10);
		expected_ = BankAccount(15);

		SetObject(current_);
		SetObjectAfter(expected_); // Uses operator== without exposing fields publicly.
		SetArguments(5);
	}

	BankAccount current_;
	BankAccount expected_;
};

/**
 * @brief Uses SetStateComparer with a friend probe for detailed diagnostics.
 */
class WithdrawLocksOnInvalidTest : public gcheck::MethodTest<bool, BankAccount, int> {
public:
	WithdrawLocksOnInvalidTest() :
		gcheck::MethodTest<bool, BankAccount, int>(
				gcheck::TestInfo("MethodPrivate", "WithdrawLocksOnInvalid"), 1, TWithdraw) {}

private:
	void SetInputsAndOutputs() override {
		account_ = BankAccount(10);

		SetObject(account_);
		SetArguments(20); // Invalid withdraw => should fail and lock account.
		SetReturn(false);

		SetStateComparer([](const BankAccount &current) -> StateDiff {
			const std::tuple<int, bool> expected_state{10, true};
			const std::tuple<int, bool> actual_state{BankAccountProbe::balance(current),
													 BankAccountProbe::locked(current)};
			const bool ok = expected_state == actual_state;
			return std::make_tuple(ok, gcheck::UserObject(expected_state), gcheck::UserObject(actual_state));
		});
	}

	BankAccount account_;
};

namespace {
DepositChangesStateTest deposit_changes_state_test;
WithdrawLocksOnInvalidTest withdraw_locks_on_invalid_test;
} // namespace
