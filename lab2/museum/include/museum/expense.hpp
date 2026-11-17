#pragma once

#include <string>

/**
 * @file expense.hpp
 * @brief Expense request with an approval workflow.
 */

namespace museum {

/**
 * @class Expense
 * @brief Money request that must be approved or rejected.
 */
class Expense {
    public:
    /**
     * @brief Creates a pending expense.
     * @throws InvalidBudgetException when the record is malformed.
     */
    Expense(std::string id, std::string category, double amount, int day);

    /**
     * @brief Approves a pending expense.
     * @throws InvalidBudgetException when the expense is not pending.
     */
    void Approve();

    /**
     * @brief Rejects a pending expense.
     * @throws InvalidBudgetException when the expense is not pending.
     */
    void Reject();

    /// @brief Checks whether the expense is pending.
    bool IsPending() const { return status_ == "pending"; }

    /// @brief Returns the expense identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the expense category.
    const std::string& Category() const { return category_; }

    /// @brief Returns the amount.
    double Amount() const { return amount_; }

    /// @brief Returns the request day.
    int Day() const { return day_; }

    /// @brief Returns the current status.
    const std::string& Status() const { return status_; }

    private:
    /// Expense identifier.
    std::string id_;
    /// Expense category.
    std::string category_;
    /// Requested amount.
    double amount_;
    /// Request day.
    int day_;
    /// Approval status: pending, approved or rejected.
    std::string status_;
};

}  // namespace museum
