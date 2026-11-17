#pragma once

#include <string>

#include "museum/exhibit.hpp"

/**
 * @file artifact.hpp
 * @brief Physical artifact with material, weight and storage location.
 */

namespace museum {

/**
 * @class Artifact
 * @brief Exhibit that has a material composition and physical location.
 */
class Artifact : public Exhibit {
    public:
    /**
     * @brief Creates an artifact.
     * @throws InvalidExhibitException when material or origin is empty or weight is negative.
     */
    Artifact(std::string id, std::string title, std::string epoch, int conditionScore,
             double appraisedValue, std::string material, double weightKg, std::string origin);

    /// @brief Returns the artifact category.
    std::string Category() const override;

    /**
     * @brief Moves the artifact to another room.
     * @param roomId non-empty room identifier.
     * @throws InvalidExhibitException when the room identifier is empty.
     */
    void MoveTo(const std::string& roomId);

    /**
     * @brief Checks whether the artifact needs special handling.
     * @return true for glass, ceramic and very light items.
     */
    bool IsFragile() const;

    /// @brief Returns the material.
    const std::string& Material() const { return material_; }

    /// @brief Returns the weight in kilograms.
    double WeightKg() const { return weightKg_; }

    /// @brief Returns the current room identifier.
    const std::string& LocationRoomId() const { return locationRoomId_; }

    protected:
    /// Material the artifact is made of.
    std::string material_;
    /// Weight in kilograms.
    double weightKg_;
    /// Place of origin.
    std::string origin_;
    /// Identifier of the room where the artifact is stored.
    std::string locationRoomId_;
};

}  // namespace museum
