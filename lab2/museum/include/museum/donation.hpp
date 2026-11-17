#pragma once

#include <string>

#include "museum/budget.hpp"

/**
 * @file donation.hpp
 * @brief Donation record that can be applied to the museum budget.
 */

namespace museum {

/**
 * @class Donation
 * @brief Charitable contribution with donor anonymity support.
 */
class Donation {
    public:
    /**
     * @brief Creates a donation.
     * @throws InvalidBudgetException when names are empty or the amount is not positive.
     */
    Donation(std::string donorName, double amount, std::string purpose, bool anonymous);

    /**
     * @brief Checks whether the donation is large.
     * @return true at or above kLargeDonation.
     */
    bool IsLarge() const;

    /**
     * @brief Returns the donor name respecting anonymity.
     * @return "Anonymous" for anonymous donations, otherwise the donor name.
     */
    std::string DisplayName() const;

    /**
     * @brief Adds the donation to a budget.
     * @param budget budget that receives the funds.
     */
    void ApplyToBudget(Budget& budget) const;

    /// @brief Returns the donation amount.
    double Amount() const { return amount_; }

    /// @brief Returns the stated purpose.
    const std::string& Purpose() const { return purpose_; }

    /// @brief Checks whether the donation is anonymous.
    bool IsAnonymous() const { return anonymous_; }

    private:
    /// Name of the donor.
    std::string donorName_;
    /// Donated amount.
    double amount_;
    /// Stated purpose of the donation.
    std::string purpose_;
    /// Whether the donor requested anonymity.
    bool anonymous_;
};

}  // namespace museum
