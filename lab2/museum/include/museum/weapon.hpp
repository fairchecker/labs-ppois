#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file weapon.hpp
 * @brief Historical weapon with license metadata.
 */

namespace museum {

/**
 * @class Weapon
 * @brief Arms and armor exhibit.
 */
class Weapon : public Artifact {
    public:
    /**
     * @brief Creates a weapon exhibit.
     * @throws InvalidExhibitException when the type is empty or the year is implausible.
     */
    Weapon(std::string id, std::string title, std::string epoch, int conditionScore,
           double appraisedValue, double weightKg, std::string origin, std::string weaponType,
           int yearMade, bool licenseRequired);

    /// @brief Returns the weapon category.
    std::string Category() const override;

    /**
     * @brief Checks whether the weapon is antique.
     * @param currentYear current calendar year.
     * @return true when the weapon is at least kAntiqueYears old.
     */
    bool IsAntique(int currentYear) const;

    /**
     * @brief Checks whether exhibiting the weapon requires a license.
     * @return configured license requirement.
     */
    bool RequiresLicense() const { return licenseRequired_; }

    /// @brief Returns the weapon type.
    const std::string& WeaponType() const { return weaponType_; }

    /// @brief Returns the manufacturing year.
    int YearMade() const { return yearMade_; }

    private:
    /// Weapon type, for example "sabre".
    std::string weaponType_;
    /// Year the weapon was made.
    int yearMade_;
    /// Whether a license is required for display.
    bool licenseRequired_;
};

}  // namespace museum
