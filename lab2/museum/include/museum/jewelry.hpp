#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file jewelry.hpp
 * @brief Jewelry artifact with gemstone and carat data.
 */

namespace museum {

/**
 * @class Jewelry
 * @brief Precious item appraised from gemstone weight.
 */
class Jewelry : public Artifact {
    public:
    /**
     * @brief Creates a jewelry item.
     * @throws InvalidExhibitException when gemstone is empty or carat weight is negative.
     */
    Jewelry(std::string id, std::string title, std::string epoch, int conditionScore,
            double appraisedValue, double weightKg, std::string origin, std::string gemstone,
            double carat, bool authenticated);

    /// @brief Returns the jewelry category.
    std::string Category() const override;

    /**
     * @brief Appraises the item including the gemstone premium.
     * @return base value plus a premium proportional to carat weight.
     */
    double Appraise() const;

    /**
     * @brief Checks whether the appraised value is high.
     * @return true at or above kHighValueThreshold.
     */
    bool IsHighValue() const;

    /// @brief Returns the gemstone name.
    const std::string& Gemstone() const { return gemstone_; }

    /// @brief Returns the carat weight.
    double Carat() const { return carat_; }

    /// @brief Checks whether the item has an authenticity certificate.
    bool IsAuthenticated() const { return authenticated_; }

    private:
    /// Gemstone name.
    std::string gemstone_;
    /// Gemstone weight in carats.
    double carat_;
    /// Whether the item has an authenticity certificate.
    bool authenticated_;
};

}  // namespace museum
