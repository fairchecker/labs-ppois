#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file textile.hpp
 * @brief Textile artifact with fabric and dimensions.
 */

namespace museum {

/**
 * @class Textile
 * @brief Fabric item such as a tapestry or historical garment.
 */
class Textile : public Artifact {
    public:
    /**
     * @brief Creates a textile exhibit.
     * @throws InvalidExhibitException when fabric is empty or dimensions are not positive.
     */
    Textile(std::string id, std::string title, std::string epoch, int conditionScore,
            double appraisedValue, double weightKg, std::string origin, std::string fabric,
            double lengthCm, double widthCm);

    /// @brief Returns the textile category.
    std::string Category() const override;

    /**
     * @brief Computes the fabric area.
     * @return length multiplied by width in square centimetres.
     */
    double Area() const;

    /**
     * @brief Checks whether the fabric needs strict climate control.
     * @return true for silk, wool and leather.
     */
    bool NeedsClimateControl() const;

    /// @brief Returns the fabric name.
    const std::string& Fabric() const { return fabric_; }

    /// @brief Returns the length in centimetres.
    double LengthCm() const { return lengthCm_; }

    /// @brief Returns the width in centimetres.
    double WidthCm() const { return widthCm_; }

    private:
    /// Fabric name.
    std::string fabric_;
    /// Length in centimetres.
    double lengthCm_;
    /// Width in centimetres.
    double widthCm_;
};

}  // namespace museum
