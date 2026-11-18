#pragma once

#include <string>
#include <vector>

/**
 * @file cargo.hpp
 * @brief Cargo hierarchy, packaging and manifest handling.
 */

namespace logistics {

/// @brief Packaging materials supported by the warehouse.
enum class PackagingType { Box, Crate, Pallet, Bag };
/// @brief Upper bound of the fragility grade scale.
inline constexpr int kMaxPackingGrade = 5;
/// @brief Minimum grade that counts as secure fragile packaging.
inline constexpr int kSecurePackingGrade = 3;
/// @brief Longest side above which cargo is treated as oversized, in centimetres.
inline constexpr double kOversizeSideThresholdCm = 250.0;
/// @brief Default weight limit of a manifest, in kilograms.
inline constexpr double kDefaultManifestWeightKg = 100000.0;

/**
 * @class Cargo
 * @brief Base shipment unit with weight, volume and declared value.
 */
class Cargo {
    public:
    /**
     * @brief Creates a cargo record.
     * @throws InvalidCargoException when an identifier is empty or a measure is invalid.
     */
    Cargo(std::string id, std::string description, double weightKg, double volumeM3,
          double declaredValue, std::string ownerId);

    /// @brief Virtual destructor for polymorphic cargo collections.
    virtual ~Cargo() = default;

    /// @brief Returns the cargo identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the cargo description.
    const std::string& Description() const { return description_; }

    /// @brief Returns the weight in kilograms.
    double WeightKg() const { return weightKg_; }

    /// @brief Returns the volume in cubic metres.
    double VolumeM3() const { return volumeM3_; }

    /// @brief Returns the declared value in currency units.
    double DeclaredValue() const { return declaredValue_; }

    /// @brief Returns the owning client identifier.
    const std::string& OwnerId() const { return ownerId_; }

    /**
     * @brief Volumetric density.
     * @return kilograms per cubic metre.
     */
    double DensityKgPerM3() const;

    /**
     * @brief Checks whether the cargo is heavier than a limit.
     * @param limitKg positive weight limit.
     */
    bool IsOverweight(double limitKg) const;

    /**
     * @brief Declared value per kilogram.
     * @return value in currency units per kilogram.
     */
    double ValuePerKg() const;

    /**
     * @brief Builds a one-line cargo summary.
     * @return "category: description, W kg".
     */
    std::string Describe() const;

    /// @brief Returns the cargo category implemented by the hierarchy.
    virtual std::string Category() const { return "general"; }

    protected:
    std::string id_;
    std::string description_;
    double weightKg_;
    double volumeM3_;
    double declaredValue_;
    std::string ownerId_;
};

/**
 * @class FragileCargo
 * @brief Cargo that requires shock protection and careful handling.
 */
class FragileCargo : public Cargo {
    public:
    /**
     * @brief Creates fragile cargo.
     * @throws InvalidCargoException when the drop height or packing grade is invalid.
     */
    FragileCargo(std::string id, std::string description, double weightKg, double volumeM3,
                 double declaredValue, std::string ownerId, double maxDropHeightCm, int packingGrade);

    /// @brief Returns the allowed drop height in centimetres.
    double MaxDropHeightCm() const { return maxDropHeightCm_; }

    /// @brief Returns the packing grade in [1, kMaxPackingGrade].
    int PackingGrade() const { return packingGrade_; }

    /// @brief Checks whether the packaging meets kSecurePackingGrade.
    bool IsSecurelyPacked() const;

    /// @brief Returns handling instructions for the loading crew.
    std::string HandlingInstructions() const;

    /// @brief Returns the cargo category.
    std::string Category() const override { return "fragile"; }

    private:
    double maxDropHeightCm_;
    int packingGrade_;
};

/**
 * @class PerishableCargo
 * @brief Cargo with a temperature range and limited shelf life.
 */
class PerishableCargo : public Cargo {
    public:
    /**
     * @brief Creates perishable cargo.
     * @throws InvalidCargoException for an inverted temperature range or non-positive shelf life.
     */
    PerishableCargo(std::string id, std::string description, double weightKg, double volumeM3,
                    double declaredValue, std::string ownerId, double minTemperatureC,
                    double maxTemperatureC, int shelfLifeHours);

    /// @brief Returns the minimum allowed temperature.
    double MinTemperatureC() const { return minTemperatureC_; }

    /// @brief Returns the maximum allowed temperature.
    double MaxTemperatureC() const { return maxTemperatureC_; }

    /// @brief Returns the shelf life in hours.
    int ShelfLifeHours() const { return shelfLifeHours_; }

    /**
     * @brief Checks whether a vehicle temperature keeps the cargo fresh.
     * @param temperatureC transport temperature.
     */
    bool IsTemperatureCompatible(double temperatureC) const;

    /**
     * @brief Checks whether the shelf life has been exhausted.
     * @param hoursInTransit elapsed hours.
     */
    bool IsExpired(int hoursInTransit) const;

    /**
     * @brief Remaining shelf life after elapsed hours.
     * @param elapsedHours hours already spent in transit.
     */
    int RemainingShelfLifeHours(int elapsedHours) const;

    /// @brief Returns the cargo category.
    std::string Category() const override { return "perishable"; }

    private:
    double minTemperatureC_;
    double maxTemperatureC_;
    int shelfLifeHours_;
};

/**
 * @class HazardousCargo
 * @brief Dangerous goods with a UN hazard class.
 */
class HazardousCargo : public Cargo {
    public:
    /**
     * @brief Creates hazardous cargo.
     * @throws InvalidCargoException for an out-of-range class or an empty UN number.
     */
    HazardousCargo(std::string id, std::string description, double weightKg, double volumeM3,
                   double declaredValue, std::string ownerId, int hazardClass, std::string unNumber,
                   bool requiresEscort);

    /// @brief Returns the UN hazard class in [1, 9].
    int HazardClass() const { return hazardClass_; }

    /// @brief Returns the United Nations substance number.
    const std::string& UnNumber() const { return unNumber_; }

    /// @brief Checks whether an escort vehicle is mandatory.
    bool RequiresEscort() const { return requiresEscort_; }

    /**
     * @brief Checks whether two hazardous cargos may travel together.
     * @param other cargo loaded into the same vehicle.
     */
    bool IsCompatibleWith(const HazardousCargo& other) const;

    /// @brief Returns the cargo category.
    std::string Category() const override { return "hazardous"; }

    private:
    int hazardClass_;
    std::string unNumber_;
    bool requiresEscort_;
};

/**
 * @class OversizedCargo
 * @brief Cargo whose dimensions require route permits.
 */
class OversizedCargo : public Cargo {
    public:
    /**
     * @brief Creates oversized cargo.
     * @throws InvalidCargoException when a dimension is not positive.
     */
    OversizedCargo(std::string id, std::string description, double weightKg, double volumeM3,
                   double declaredValue, std::string ownerId, double lengthCm, double widthCm,
                   double heightCm);

    /// @brief Returns the length in centimetres.
    double LengthCm() const { return lengthCm_; }

    /// @brief Returns the width in centimetres.
    double WidthCm() const { return widthCm_; }

    /// @brief Returns the height in centimetres.
    double HeightCm() const { return heightCm_; }

    /// @brief Returns the longest side in centimetres.
    double LongestSideCm() const;

    /// @brief Checks whether a special transport permit is required.
    bool NeedsPermit() const;

    /// @brief Returns the cargo category.
    std::string Category() const override { return "oversized"; }

    private:
    double lengthCm_;
    double widthCm_;
    double heightCm_;
};

/**
 * @class Packaging
 * @brief Reusable or disposable packaging batch.
 */
class Packaging {
    public:
    /**
     * @brief Creates a packaging batch.
     * @throws InvalidCargoException for a negative unit cost or count.
     */
    Packaging(PackagingType type, double costPerUnit, int units);

    /// @brief Returns the packaging type.
    PackagingType Type() const { return type_; }

    /// @brief Returns the cost of one unit.
    double CostPerUnit() const { return costPerUnit_; }

    /// @brief Returns the number of units.
    int Units() const { return units_; }

    /// @brief Returns the total packaging cost.
    double TotalCost() const;

    /**
     * @brief Adds units to the batch.
     * @param units positive number of units.
     * @throws InvalidCargoException when units is not positive.
     */
    void AddUnits(int units);

    /// @brief Checks whether the material can be returned for reuse.
    bool IsReusable() const;

    /// @brief Returns the packaging type name.
    std::string TypeName() const;

    private:
    PackagingType type_;
    double costPerUnit_;
    int units_;
};

/**
 * @class CargoManifest
 * @brief Accumulates cargo identifiers and their aggregate metrics.
 */
class CargoManifest {
    public:
    /**
     * @brief Creates an empty manifest.
     * @param maxWeightKg positive weight limit.
     * @throws InvalidCargoException when the limit is not positive.
     */
    explicit CargoManifest(double maxWeightKg = kDefaultManifestWeightKg);

    /// @brief Returns the manifest weight limit.
    double MaxWeightKg() const { return maxWeightKg_; }

    /// @brief Returns the cargo identifiers in insertion order.
    const std::vector<std::string>& CargoIds() const { return cargoIds_; }

    /// @brief Returns the total weight of all added cargo.
    double TotalWeightKg() const { return totalWeightKg_; }

    /// @brief Returns the total declared value.
    double TotalValue() const { return totalValue_; }

    /**
     * @brief Adds cargo to the manifest.
     * @throws InvalidCargoException when the same cargo is added twice.
     */
    void AddCargo(const Cargo& cargo);

    /**
     * @brief Removes cargo from the manifest by identifier.
     * @param cargo cargo to remove.
     * @return true when the cargo was present.
     */
    bool RemoveCargo(const Cargo& cargo);

    /// @brief Checks whether the manifest is empty.
    bool IsEmpty() const { return cargoIds_.empty(); }

    /// @brief Average declared value per kilogram, zero for an empty manifest.
    double AverageValuePerKg() const;

    /// @brief Returns the number of cargo units.
    std::size_t Size() const { return cargoIds_.size(); }

    private:
    std::vector<std::string> cargoIds_;
    double totalWeightKg_ = 0.0;
    double totalValue_ = 0.0;
    double maxWeightKg_ = kDefaultManifestWeightKg;
};

}  // namespace logistics
