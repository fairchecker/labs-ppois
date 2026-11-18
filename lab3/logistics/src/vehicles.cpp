#include "logistics/vehicles.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kMinMaintenanceDay = 0;
constexpr int kMaxAxles = 12;
constexpr double kHundredKilometres = 100.0;

bool IsWorkingStatus(VehicleStatus status) {
    return status == VehicleStatus::Available || status == VehicleStatus::OnRoute;
}

}  // namespace

DrivingLicense::DrivingLicense(std::string number, std::string category, int issuedDay,
                               int expiresDay)
    : number_(std::move(number)),
      category_(std::move(category)),
      issuedDay_(issuedDay),
      expiresDay_(expiresDay) {
    if (number_.empty() || category_.empty() || issuedDay_ < 0 || expiresDay_ <= issuedDay_) {
        throw InvalidLicenseException("Driving license requires a number, category and valid window");
    }
}

bool DrivingLicense::IsValidOn(int day) const {
    return day >= issuedDay_ && day <= expiresDay_;
}

bool DrivingLicense::AllowsCategory(const std::string& category) const {
    return category_ == category;
}

void DrivingLicense::Renew(int additionalDays) {
    if (additionalDays <= 0 || additionalDays > kMaxLicenseValidityDays) {
        throw InvalidLicenseException("License renewal period is out of range");
    }
    expiresDay_ += additionalDays;
}

int DrivingLicense::DaysUntilExpiry(int day) const {
    return std::max(0, expiresDay_ - day);
}

MaintenanceRecord::MaintenanceRecord(std::string id, std::string vehicleId, int day,
                                     std::string kind, double cost, double odometerKm,
                                     std::string notes)
    : id_(std::move(id)),
      vehicleId_(std::move(vehicleId)),
      day_(day),
      kind_(std::move(kind)),
      cost_(cost),
      odometerKm_(odometerKm),
      notes_(std::move(notes)) {
    const bool valuesValid = day_ >= kMinMaintenanceDay && cost_ >= 0.0 && odometerKm_ >= 0.0;
    if (id_.empty() || vehicleId_.empty() || kind_.empty() || !valuesValid) {
        throw InvalidVehicleException("Maintenance record has an invalid identifier, kind or value");
    }
}

bool MaintenanceRecord::IsOverdue(double currentOdometerKm, double intervalKm) const {
    if (intervalKm <= 0.0) {
        throw InvalidVehicleException("Maintenance interval must be positive");
    }
    return currentOdometerKm - odometerKm_ >= intervalKm;
}

double MaintenanceRecord::CostPerKm(double previousOdometerKm) const {
    const double distanceKm = odometerKm_ - previousOdometerKm;
    if (distanceKm <= 0.0) {
        throw InvalidVehicleException("Service distance must be positive");
    }
    return cost_ / distanceKm;
}

std::string MaintenanceRecord::Describe() const {
    return kind_ + " on day " + std::to_string(day_) + ", cost " + std::to_string(cost_);
}

FuelRecord::FuelRecord(std::string vehicleId, double liters, double pricePerLiter,
                       double odometerKm, int day)
    : vehicleId_(std::move(vehicleId)),
      liters_(liters),
      pricePerLiter_(pricePerLiter),
      odometerKm_(odometerKm),
      day_(day) {
    const bool valuesValid = liters_ > 0.0 && pricePerLiter_ > 0.0 && odometerKm_ >= 0.0;
    if (vehicleId_.empty() || !valuesValid || day_ < 0) {
        throw InvalidVehicleException("Fuel record has an invalid vehicle or value");
    }
}

double FuelRecord::Cost() const { return liters_ * pricePerLiter_; }

double FuelRecord::ConsumptionPer100Km(double previousOdometerKm) const {
    const double distanceKm = odometerKm_ - previousOdometerKm;
    if (distanceKm <= 0.0) {
        throw InvalidVehicleException("Fuel distance must be positive");
    }
    return (liters_ / distanceKm) * kHundredKilometres;
}

Vehicle::Vehicle(std::string id, std::string plateNumber, double capacityKg,
                 double capacityVolumeM3, double odometerKm, double fuelLiters)
    : id_(std::move(id)),
      plateNumber_(std::move(plateNumber)),
      capacityKg_(capacityKg),
      capacityVolumeM3_(capacityVolumeM3),
      odometerKm_(odometerKm),
      fuelLiters_(fuelLiters),
      status_(VehicleStatus::Available) {
    const bool measuresValid = capacityKg_ > 0.0 && capacityVolumeM3_ > 0.0;
    if (id_.empty() || plateNumber_.empty() || !measuresValid || odometerKm_ < 0.0 ||
        fuelLiters_ < 0.0) {
        throw InvalidVehicleException("Vehicle requires an identifier, plate and valid measures");
    }
}

bool Vehicle::CanCarry(double weightKg, double volumeM3) const {
    if (weightKg < 0.0 || volumeM3 < 0.0) {
        return false;
    }
    return weightKg <= capacityKg_ && volumeM3 <= capacityVolumeM3_;
}

double Vehicle::UtilizationPercent(double weightKg) const {
    if (weightKg < 0.0) {
        throw InvalidVehicleException("Cargo weight cannot be negative");
    }
    const double ratio = weightKg / capacityKg_;
    return std::min(kPercentScale, ratio * kPercentScale);
}

void Vehicle::ValidateLoad(double weightKg, double volumeM3) const {
    if (weightKg < 0.0 || volumeM3 < 0.0) {
        throw InvalidVehicleException("Cargo measures cannot be negative");
    }
    if (!CanCarry(weightKg, volumeM3)) {
        throw VehicleOverloadException("Cargo exceeds the vehicle capacity");
    }
}

void Vehicle::Refuel(double liters, double tankCapacityLiters) {
    if (liters <= 0.0 || tankCapacityLiters <= 0.0) {
        throw InvalidVehicleException("Refuelling volumes must be positive");
    }
    if (fuelLiters_ + liters > tankCapacityLiters) {
        throw InvalidVehicleException("Refuelling exceeds tank capacity");
    }
    fuelLiters_ += liters;
}

void Vehicle::Drive(double distanceKm) {
    if (distanceKm <= 0.0) {
        throw InvalidVehicleException("Distance must be positive");
    }
    if (!IsWorkingStatus(status_)) {
        throw VehicleNotAvailableException("Vehicle cannot drive in its current state");
    }
    const double requiredFuel = (distanceKm / kHundredKilometres) * FuelPer100Km();
    if (requiredFuel > fuelLiters_) {
        throw InvalidVehicleException("Vehicle has insufficient fuel");
    }
    fuelLiters_ -= requiredFuel;
    odometerKm_ += distanceKm;
}

void Vehicle::SendToMaintenance() {
    if (status_ != VehicleStatus::Available) {
        throw VehicleNotAvailableException("Only an available vehicle can enter maintenance");
    }
    status_ = VehicleStatus::Maintenance;
}

void Vehicle::ReturnToService() {
    if (status_ != VehicleStatus::Maintenance) {
        throw VehicleNotAvailableException("Only a vehicle in maintenance can return to service");
    }
    status_ = VehicleStatus::Available;
}

void Vehicle::MarkOnRoute() {
    if (status_ != VehicleStatus::Available) {
        throw VehicleNotAvailableException("Only an available vehicle can start a route");
    }
    status_ = VehicleStatus::OnRoute;
}

void Vehicle::CompleteRoute() {
    if (status_ != VehicleStatus::OnRoute) {
        throw VehicleNotAvailableException("Only a vehicle on a route can complete it");
    }
    status_ = VehicleStatus::Available;
}

void Vehicle::Retire() {
    if (status_ == VehicleStatus::OnRoute) {
        throw VehicleNotAvailableException("A vehicle on a route cannot be retired");
    }
    status_ = VehicleStatus::Retired;
}

double Vehicle::RangeKm() const { return (fuelLiters_ / FuelPer100Km()) * kHundredKilometres; }

Truck::Truck(std::string id, std::string plateNumber, double capacityKg, double capacityVolumeM3,
             double odometerKm, double fuelLiters, int axleCount, double maxTrailerWeightKg)
    : Vehicle(std::move(id), std::move(plateNumber), capacityKg, capacityVolumeM3, odometerKm,
              fuelLiters),
      axleCount_(axleCount),
      maxTrailerWeightKg_(maxTrailerWeightKg) {
    if (axleCount_ < 2 || axleCount_ > kMaxAxles || maxTrailerWeightKg_ < 0.0) {
        throw InvalidVehicleException("Truck requires a valid axle count and trailer limit");
    }
}

bool Truck::CanPull(double trailerWeightKg) const {
    return trailerWeightKg > 0.0 && trailerWeightKg <= maxTrailerWeightKg_;
}

Van::Van(std::string id, std::string plateNumber, double capacityKg, double capacityVolumeM3,
         double odometerKm, double fuelLiters, bool refrigerated, int maxParcelCount)
    : Vehicle(std::move(id), std::move(plateNumber), capacityKg, capacityVolumeM3, odometerKm,
              fuelLiters),
      refrigerated_(refrigerated),
      maxParcelCount_(maxParcelCount) {
    if (maxParcelCount_ <= 0) {
        throw InvalidVehicleException("Van requires a positive parcel limit");
    }
}

bool Van::FitsParcels(int parcelCount) const {
    return parcelCount >= 0 && parcelCount <= maxParcelCount_;
}

RefrigeratedTruck::RefrigeratedTruck(std::string id, std::string plateNumber, double capacityKg,
                                     double capacityVolumeM3, double odometerKm, double fuelLiters,
                                     int axleCount, double maxTrailerWeightKg,
                                     double minTemperatureC, double maxTemperatureC)
    : Truck(std::move(id), std::move(plateNumber), capacityKg, capacityVolumeM3, odometerKm,
            fuelLiters, axleCount, maxTrailerWeightKg),
      minTemperatureC_(minTemperatureC),
      maxTemperatureC_(maxTemperatureC) {
    if (minTemperatureC_ >= maxTemperatureC_) {
        throw InvalidVehicleException("Refrigerated truck requires a valid temperature range");
    }
}

bool RefrigeratedTruck::KeepsTemperature(double requiredTemperatureC) const {
    return requiredTemperatureC >= minTemperatureC_ && requiredTemperatureC <= maxTemperatureC_;
}

Trailer::Trailer(std::string id, std::string plateNumber, double capacityKg, double volumeM3,
                 bool refrigerated)
    : id_(std::move(id)),
      plateNumber_(std::move(plateNumber)),
      capacityKg_(capacityKg),
      volumeM3_(volumeM3),
      refrigerated_(refrigerated) {
    if (id_.empty() || plateNumber_.empty() || capacityKg_ <= 0.0 || volumeM3_ <= 0.0) {
        throw InvalidVehicleException("Trailer requires an identifier, plate and valid measures");
    }
}

void Trailer::AttachTo(const std::string& vehicleId) {
    if (vehicleId.empty()) {
        throw InvalidVehicleException("Trailer attachment requires a vehicle identifier");
    }
    if (IsAttached()) {
        throw InvalidVehicleException("Trailer is already attached");
    }
    attachedVehicleId_ = vehicleId;
}

void Trailer::Detach() {
    if (!IsAttached()) {
        throw InvalidVehicleException("Trailer is not attached");
    }
    attachedVehicleId_.clear();
}

bool Trailer::CanCarry(double weightKg, double volumeM3) const {
    if (weightKg < 0.0 || volumeM3 < 0.0) {
        return false;
    }
    return weightKg <= capacityKg_ && volumeM3 <= volumeM3_;
}

void VehicleFleet::AddVehicle(std::unique_ptr<Vehicle> vehicle) {
    if (!vehicle) {
        throw InvalidVehicleException("Cannot register an empty vehicle");
    }
    if (FindVehicle(vehicle->Id()) != nullptr) {
        throw InvalidVehicleException("Vehicle identifier is already registered");
    }
    vehicles_.push_back(std::move(vehicle));
}

Vehicle* VehicleFleet::FindVehicle(const std::string& vehicleId) {
    for (const std::unique_ptr<Vehicle>& vehicle : vehicles_) {
        if (vehicle->Id() == vehicleId) {
            return vehicle.get();
        }
    }
    return nullptr;
}

const Vehicle* VehicleFleet::FindVehicle(const std::string& vehicleId) const {
    for (const std::unique_ptr<Vehicle>& vehicle : vehicles_) {
        if (vehicle->Id() == vehicleId) {
            return vehicle.get();
        }
    }
    return nullptr;
}

std::vector<Vehicle*> VehicleFleet::AvailableVehicles() {
    std::vector<Vehicle*> available;
    for (const std::unique_ptr<Vehicle>& vehicle : vehicles_) {
        if (vehicle->IsAvailable()) {
            available.push_back(vehicle.get());
        }
    }
    return available;
}

bool VehicleFleet::RemoveVehicle(const std::string& vehicleId) {
    const auto found = std::find_if(
        vehicles_.begin(), vehicles_.end(),
        [&vehicleId](const std::unique_ptr<Vehicle>& vehicle) { return vehicle->Id() == vehicleId; });
    if (found == vehicles_.end()) {
        return false;
    }
    vehicles_.erase(found);
    return true;
}

double VehicleFleet::TotalCapacityKg() const {
    double total = 0.0;
    for (const std::unique_ptr<Vehicle>& vehicle : vehicles_) {
        total += vehicle->CapacityKg();
    }
    return total;
}

double VehicleFleet::AvailabilityPercent() const {
    if (vehicles_.empty()) {
        return 0.0;
    }
    std::size_t available = 0;
    for (const std::unique_ptr<Vehicle>& vehicle : vehicles_) {
        if (vehicle->IsAvailable()) {
            ++available;
        }
    }
    const double ratio = static_cast<double>(available) / static_cast<double>(vehicles_.size());
    return ratio * kPercentScale;
}

void VehicleFleet::ScheduleMaintenance(const MaintenanceRecord& record) {
    Vehicle* vehicle = FindVehicle(record.VehicleId());
    if (vehicle == nullptr) {
        throw InvalidVehicleException("Maintenance record refers to an unknown vehicle");
    }
    vehicle->SendToMaintenance();
    maintenanceRecords_.push_back(record);
}

void VehicleFleet::RecordFuel(const FuelRecord& record) {
    if (FindVehicle(record.VehicleId()) == nullptr) {
        throw InvalidVehicleException("Fuel record refers to an unknown vehicle");
    }
    fuelRecords_.push_back(record);
}

}  // namespace logistics
