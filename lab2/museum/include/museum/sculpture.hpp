#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file sculpture.hpp
 * @brief Three-dimensional artifact with pedestal and outdoor metadata.
 */

namespace museum {

/**
 * @class Sculpture
 * @brief Artifact whose display may require a pedestal.
 */
class Sculpture : public Artifact {
    public:
    /**
     * @brief Creates a sculpture.
     * @throws InvalidExhibitException when height is not positive.
     */
    Sculpture(std::string id, std::string title, std::string epoch, int conditionScore,
              double appraisedValue, double weightKg, std::string origin, std::string material,
              double heightCm, bool hasPedestal, bool outdoorSuitable);

    /// @brief Returns the sculpture category.
    std::string Category() const override;

    /**
     * @brief Estimates the display volume.
     * @return volume in cubic metres based on a square footprint.
     */
    double VolumeEstimate() const;

    /**
     * @brief Checks whether the sculpture may stand outdoors.
     * @return configured outdoor suitability.
     */
    bool IsOutdoorSuitable() const { return outdoorSuitable_; }

    /// @brief Returns the height in centimetres.
    double HeightCm() const { return heightCm_; }

    /// @brief Checks whether the sculpture requires a pedestal.
    bool HasPedestal() const { return hasPedestal_; }

    private:
    /// Height in centimetres.
    double heightCm_;
    /// Whether the sculpture is displayed on a pedestal.
    bool hasPedestal_;
    /// Whether the sculpture tolerates outdoor conditions.
    bool outdoorSuitable_;
};

}  // namespace museum
