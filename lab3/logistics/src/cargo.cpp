#include "logistics/cargo.hpp"

#include <algorithm>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kMinHazardClass = 1;
constexpr int kMaxHazardClass = 9;
constexpr int kExplosiveHazardClass = 1;
constexpr int kRadioactiveHazardClass = 7;
constexpr int kMinPackingGrade = 1;

bool IsValidHazardClass(int hazardClass) {
    return hazardClass >= kMinHazardClass && hazardClass <= kMaxHazardClass;
}

bool IsExplosiveOrRadioactive(int hazardClass) {
    return hazardClass == kExplosiveHazardClass || hazardClass == kRadioactiveHazardClass;
}

}  // namespace

Cargo::Cargo(std::string id, std::string description, double weightKg, double volumeM3,
             double declaredValue, std::string ownerId)
    : id_(std::move(id)),
      description_(std::move(description)),
      weightKg_(weightKg),
      volumeM3_(volumeM3),
      declaredValue_(declaredValue),
      ownerId_(std::move(ownerId)) {
    const bool measuresValid = weightKg_ > 0.0 && volumeM3_ > 0.0 && declaredValue_ >= 0.0;
    if (id_.empty() || description_.empty() || ownerId_.empty() || !measuresValid) {
        throw InvalidCargoException("Cargo requires an identifier, description, owner and measures");
    }
}

double Cargo::DensityKgPerM3() const { return weightKg_ / volumeM3_; }

bool Cargo::IsOverweight(double limitKg) const { return weightKg_ > limitKg; }

double Cargo::ValuePerKg() const { return declaredValue_ / weightKg_; }

std::string Cargo::Describe() const {
    return Category() + ": " + description_ + ", " + std::to_string(weightKg_) + " kg";
}

FragileCargo::FragileCargo(std::string id, std::string description, double weightKg, double volumeM3,
                           double declaredValue, std::string ownerId, double maxDropHeightCm,
                           int packingGrade)
    : Cargo(std::move(id), std::move(description), weightKg, volumeM3, declaredValue,
            std::move(ownerId)),
      maxDropHeightCm_(maxDropHeightCm),
      packingGrade_(packingGrade) {
    const bool gradeValid = packingGrade_ >= kMinPackingGrade && packingGrade_ <= kMaxPackingGrade;
    if (maxDropHeightCm_ <= 0.0 || !gradeValid) {
        throw InvalidCargoException("Fragile cargo requires a positive drop height and valid grade");
    }
}

bool FragileCargo::IsSecurelyPacked() const { return packingGrade_ >= kSecurePackingGrade; }

std::string FragileCargo::HandlingInstructions() const {
    const std::string protection = IsSecurelyPacked() ? "secure" : "reinforced";
    return "fragile: handle with care, " + protection + " packaging required";
}

PerishableCargo::PerishableCargo(std::string id, std::string description, double weightKg,
                                 double volumeM3, double declaredValue, std::string ownerId,
                                 double minTemperatureC, double maxTemperatureC, int shelfLifeHours)
    : Cargo(std::move(id), std::move(description), weightKg, volumeM3, declaredValue,
            std::move(ownerId)),
      minTemperatureC_(minTemperatureC),
      maxTemperatureC_(maxTemperatureC),
      shelfLifeHours_(shelfLifeHours) {
    if (minTemperatureC_ >= maxTemperatureC_ || shelfLifeHours_ <= 0) {
        throw InvalidCargoException("Perishable cargo requires a valid temperature range and shelf life");
    }
}

bool PerishableCargo::IsTemperatureCompatible(double temperatureC) const {
    return temperatureC >= minTemperatureC_ && temperatureC <= maxTemperatureC_;
}

bool PerishableCargo::IsExpired(int hoursInTransit) const {
    return hoursInTransit >= shelfLifeHours_;
}

int PerishableCargo::RemainingShelfLifeHours(int elapsedHours) const {
    const int remaining = shelfLifeHours_ - elapsedHours;
    return std::max(0, remaining);
}

HazardousCargo::HazardousCargo(std::string id, std::string description, double weightKg,
                               double volumeM3, double declaredValue, std::string ownerId,
                               int hazardClass, std::string unNumber, bool requiresEscort)
    : Cargo(std::move(id), std::move(description), weightKg, volumeM3, declaredValue,
            std::move(ownerId)),
      hazardClass_(hazardClass),
      unNumber_(std::move(unNumber)),
      requiresEscort_(requiresEscort) {
    if (!IsValidHazardClass(hazardClass_) || unNumber_.empty()) {
        throw InvalidCargoException("Hazardous cargo requires a valid UN class and number");
    }
}

bool HazardousCargo::IsCompatibleWith(const HazardousCargo& other) const {
    if (hazardClass_ == other.hazardClass_) {
        return true;
    }
    return !IsExplosiveOrRadioactive(hazardClass_) && !IsExplosiveOrRadioactive(other.hazardClass_);
}

OversizedCargo::OversizedCargo(std::string id, std::string description, double weightKg,
                               double volumeM3, double declaredValue, std::string ownerId,
                               double lengthCm, double widthCm, double heightCm)
    : Cargo(std::move(id), std::move(description), weightKg, volumeM3, declaredValue,
            std::move(ownerId)),
      lengthCm_(lengthCm),
      widthCm_(widthCm),
      heightCm_(heightCm) {
    if (lengthCm_ <= 0.0 || widthCm_ <= 0.0 || heightCm_ <= 0.0) {
        throw InvalidCargoException("Oversized cargo requires positive dimensions");
    }
}

double OversizedCargo::LongestSideCm() const {
    return std::max({lengthCm_, widthCm_, heightCm_});
}

bool OversizedCargo::NeedsPermit() const { return LongestSideCm() > kOversizeSideThresholdCm; }

Packaging::Packaging(PackagingType type, double costPerUnit, int units)
    : type_(type), costPerUnit_(costPerUnit), units_(units) {
    if (costPerUnit_ < 0.0 || units_ < 0) {
        throw InvalidCargoException("Packaging requires non-negative cost and units");
    }
}

double Packaging::TotalCost() const { return costPerUnit_ * units_; }

void Packaging::AddUnits(int units) {
    if (units <= 0) {
        throw InvalidCargoException("Packaging units to add must be positive");
    }
    units_ += units;
}

bool Packaging::IsReusable() const {
    return type_ == PackagingType::Crate || type_ == PackagingType::Pallet;
}

std::string Packaging::TypeName() const {
    switch (type_) {
        case PackagingType::Box:
            return "box";
        case PackagingType::Crate:
            return "crate";
        case PackagingType::Pallet:
            return "pallet";
        case PackagingType::Bag:
            return "bag";
    }
    return "unknown";
}

CargoManifest::CargoManifest(double maxWeightKg) : maxWeightKg_(maxWeightKg) {
    if (maxWeightKg_ <= 0.0) {
        throw InvalidCargoException("Manifest weight limit must be positive");
    }
}

void CargoManifest::AddCargo(const Cargo& cargo) {
    const auto found = std::find(cargoIds_.begin(), cargoIds_.end(), cargo.Id());
    if (found != cargoIds_.end()) {
        throw InvalidCargoException("Cargo already exists in the manifest");
    }
    if (totalWeightKg_ + cargo.WeightKg() > maxWeightKg_) {
        throw CargoOverweightException("Manifest weight limit would be exceeded");
    }
    cargoIds_.push_back(cargo.Id());
    totalWeightKg_ += cargo.WeightKg();
    totalValue_ += cargo.DeclaredValue();
}

bool CargoManifest::RemoveCargo(const Cargo& cargo) {
    const auto found = std::find(cargoIds_.begin(), cargoIds_.end(), cargo.Id());
    if (found == cargoIds_.end()) {
        return false;
    }
    cargoIds_.erase(found);
    totalWeightKg_ -= cargo.WeightKg();
    totalValue_ -= cargo.DeclaredValue();
    return true;
}

double CargoManifest::AverageValuePerKg() const {
    if (totalWeightKg_ <= 0.0) {
        return 0.0;
    }
    return totalValue_ / totalWeightKg_;
}

}  // namespace logistics
