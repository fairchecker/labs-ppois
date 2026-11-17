#pragma once

#include <string>

/**
 * @file budget.hpp
 * @brief Museum budget with allocation and spending limits.
 */

namespace museum {

/**
 * @class Budget
 * @brief Tracks allocated funds and prevents overspending.
 */
class Budget {
    public:
    /**
     * @brief Creates a budget.
     * @throws InvalidBudgetException when the amount is negative or the category is empty.
     */
    Budget(double totalAmount, std::string category);

    /**
     * @brief Adds funds to the budget.
     * @param amount strictly positive amount.
     * @throws InvalidBudgetException when the amount is not positive.
     */
    void Allocate(double amount);

    /**
     * @brief Spends money from the budget.
     * @param amount strictly positive amount within the remaining funds.
     * @throws InvalidBudgetException when the amount is not positive.
     * @throws BudgetExceededException when the amount exceeds the remaining funds.
     */
    void Spend(double amount);

    /// @brief Returns the unspent balance.
    double Remaining() const;

    /// @brief Checks whether spending exceeded allocation.
    bool IsOverBudget() const;

    /// @brief Returns the total allocated amount.
    double Total() const { return totalAmount_; }

    /// @brief Returns the spent amount.
    double Spent() const { return spentAmount_; }

    /// @brief Returns the budget category.
    const std::string& Category() const { return category_; }

    private:
    /// Total allocated amount.
    double totalAmount_;
    /// Amount already spent.
    double spentAmount_ = 0.0;
    /// Budget category, for example "restoration".
    std::string category_;
};

}  // namespace museum
