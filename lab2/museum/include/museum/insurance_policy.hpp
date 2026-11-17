#pragma once

#include <string>

/**
 * @file insurance_policy.hpp
 * @brief Insurance policy for an exhibit.
 */

namespace museum {

/**
 * @class InsurancePolicy
 * @brief Coverage contract with claim handling.
 */
class InsurancePolicy {
    public:
    /**
     * @brief Creates an insurance policy.
     * @throws InsuranceException when the record is malformed.
     */
    InsurancePolicy(std::string exhibitId, std::string provider, double premium,
                    double coverage);

    /**
     * @brief Checks whether the coverage is below the exhibit value.
     * @param exhibitValue current appraised value.
     * @return true when coverage is lower than the value.
     */
    bool IsUnderinsured(double exhibitValue) const;

    /**
     * @brief Files a claim against the policy.
     * @param damage strictly positive damage amount within the coverage.
     * @return the reimbursed amount.
     * @throws InsuranceException when the claim is invalid or exceeds the coverage.
     */
    double Claim(double damage);

    /**
     * @brief Renews the policy with extra coverage.
     * @param extraCoverage strictly positive additional coverage.
     * @throws InsuranceException when the extra coverage is not positive.
     */
    void Renew(double extraCoverage);

    /// @brief Returns the exhibit identifier.
    const std::string& ExhibitId() const { return exhibitId_; }

    /// @brief Returns the provider name.
    const std::string& Provider() const { return provider_; }

    /// @brief Returns the premium.
    double Premium() const { return premium_; }

    /// @brief Returns the current coverage.
    double Coverage() const { return coverage_; }

    private:
    /// Identifier of the insured exhibit.
    std::string exhibitId_;
    /// Insurance provider name.
    std::string provider_;
    /// Policy premium.
    double premium_;
    /// Remaining coverage amount.
    double coverage_;
};

}  // namespace museum
