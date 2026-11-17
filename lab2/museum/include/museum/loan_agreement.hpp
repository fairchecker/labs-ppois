#pragma once

#include <string>

/**
 * @file loan_agreement.hpp
 * @brief Agreement to lend an exhibit to another institution.
 */

namespace museum {

/**
 * @class LoanAgreement
 * @brief Temporary transfer of an exhibit for a daily fee.
 */
class LoanAgreement {
    public:
    /**
     * @brief Creates a loan agreement.
     * @throws LoanException when identifiers are empty or dates and fee are invalid.
     */
    LoanAgreement(std::string exhibitId, std::string borrower, int startDay, int endDay,
                  double dailyFee);

    /**
     * @brief Checks whether the loan is active on a day.
     * @param day day number.
     * @return true inside the inclusive loan period.
     */
    bool IsActiveOn(int day) const;

    /**
     * @brief Computes the total loan fee.
     * @return daily fee multiplied by the inclusive duration.
     */
    double CalculateFee() const;

    /**
     * @brief Extends the loan period.
     * @param days strictly positive number of days.
     * @throws LoanException when days is not positive.
     */
    void Extend(int days);

    /// @brief Returns the exhibit identifier.
    const std::string& ExhibitId() const { return exhibitId_; }

    /// @brief Returns the borrower name.
    const std::string& Borrower() const { return borrower_; }

    /// @brief Returns the first loan day.
    int StartDay() const { return startDay_; }

    /// @brief Returns the last loan day.
    int EndDay() const { return endDay_; }

    /// @brief Returns the daily fee.
    double DailyFee() const { return dailyFee_; }

    private:
    /// Identifier of the lent exhibit.
    std::string exhibitId_;
    /// Name of the borrowing institution.
    std::string borrower_;
    /// First day of the loan.
    int startDay_;
    /// Last day of the loan.
    int endDay_;
    /// Fee charged per day.
    double dailyFee_;
};

}  // namespace museum
