#pragma once

#include <string>

/**
 * @file grant.hpp
 * @brief Research grant with application deadline.
 */

namespace museum {

class Budget;

/**
 * @class Grant
 * @brief Funding opportunity that can be awarded once.
 */
class Grant {
    public:
    /**
     * @brief Creates a grant.
     * @throws InvalidBudgetException when the record is malformed.
     */
    Grant(std::string title, double amount, int deadlineDay);

    /**
     * @brief Applies for the grant.
     * @return true when the application is accepted, false when already awarded.
     */
    bool Apply();

    /**
     * @brief Checks whether the deadline has passed.
     * @param currentDay current day.
     * @return true when currentDay is after the deadline.
     */
    bool IsExpired(int currentDay) const;

    /**
     * @brief Awards the grant.
     * @throws InvalidBudgetException when the grant was already awarded or has no value.
     */
    void Award();

    /**
     * @brief Deposits an awarded grant into a budget.
     * @param budget budget that receives the funds.
     * @throws InvalidBudgetException when the grant was not awarded.
     */
    void ApplyToBudget(Budget& budget) const;

    /// @brief Returns the grant title.
    const std::string& Title() const { return title_; }

    /// @brief Returns the grant amount.
    double Amount() const { return amount_; }

    /// @brief Returns the deadline day.
    int DeadlineDay() const { return deadlineDay_; }

    /// @brief Checks whether the grant was awarded.
    bool IsAwarded() const { return awarded_; }

    private:
    /// Grant title.
    std::string title_;
    /// Grant amount.
    double amount_;
    /// Application deadline day.
    int deadlineDay_;
    /// Whether the grant has been awarded.
    bool awarded_ = false;
};

}  // namespace museum
