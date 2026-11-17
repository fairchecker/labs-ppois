#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file fossil.hpp
 * @brief Paleontological exhibit with species and geological era.
 */

namespace museum {

/**
 * @class Fossil
 * @brief Natural history specimen with an estimated age.
 */
class Fossil : public Artifact {
    public:
    /**
     * @brief Creates a fossil.
     * @throws InvalidExhibitException when species or era is empty or age is negative.
     */
    Fossil(std::string id, std::string title, std::string epoch, int conditionScore,
           double appraisedValue, double weightKg, std::string origin, std::string species,
           std::string era, double ageThousandYears);

    /// @brief Returns the fossil category.
    std::string Category() const override;

    /**
     * @brief Checks whether the specimen predates recorded history.
     * @return true when older than ten thousand years.
     */
    bool IsPrehistoric() const;

    /**
     * @brief Describes the geological era and age.
     * @return human-readable era description.
     */
    std::string EraDescription() const;

    /// @brief Returns the species name.
    const std::string& Species() const { return species_; }

    /// @brief Returns the geological era.
    const std::string& Era() const { return era_; }

    /// @brief Returns the age in thousands of years.
    double AgeThousandYears() const { return ageThousandYears_; }

    private:
    /// Species name.
    std::string species_;
    /// Geological era.
    std::string era_;
    /// Estimated age in thousands of years.
    double ageThousandYears_;
};

}  // namespace museum
