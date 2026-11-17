#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file painting.hpp
 * @brief Painting artifact with artist, technique and dimensions.
 */

namespace museum {

/**
 * @class Painting
 * @brief Two-dimensional artwork measured in centimetres.
 */
class Painting : public Artifact {
    public:
    /**
     * @brief Creates a painting.
     * @throws InvalidExhibitException when the artist is empty or dimensions are not positive.
     */
    Painting(std::string id, std::string title, std::string epoch, int conditionScore,
             double appraisedValue, double weightKg, std::string origin, std::string artist,
             std::string technique, double widthCm, double heightCm);

    /// @brief Returns the painting category.
    std::string Category() const override;

    /**
     * @brief Computes the painted area.
     * @return width multiplied by height in square centimetres.
     */
    double Area() const;

    /**
     * @brief Checks whether the painting is large.
     * @return true when the area exceeds kLargePaintingAreaCm2.
     */
    bool IsLarge() const;

    /// @brief Returns the artist name.
    const std::string& Artist() const { return artist_; }

    /// @brief Returns the painting technique.
    const std::string& Technique() const { return technique_; }

    /// @brief Returns the width in centimetres.
    double WidthCm() const { return widthCm_; }

    /// @brief Returns the height in centimetres.
    double HeightCm() const { return heightCm_; }

    private:
    /// Artist name.
    std::string artist_;
    /// Painting technique, for example "oil on canvas".
    std::string technique_;
    /// Width in centimetres.
    double widthCm_;
    /// Height in centimetres.
    double heightCm_;
};

}  // namespace museum
