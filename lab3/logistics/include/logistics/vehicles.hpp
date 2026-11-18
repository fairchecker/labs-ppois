#pragma once

#include <memory>
#include <string>
#include <vector>

/**
 * @file vehicles.hpp
 * @brief Fleet model: vehicles, licenses, maintenance and fuel records.
 */

namespace logistics {

/// @brief Operational state of a vehicle.
enum class VehicleStatus { Available, OnRoute, Maintenance, Retired };
/// @brief Minimal allowed driving experience, in years.
inline constexpr int kMinDrivingExperienceYears = 1;
/// @brief Fuel consumed by a truck per 100 km, in litres.
inline constexpr double kTruckFuelPer100Km = 24.0;
/// @brief Fuel consumed by a van per 100 km, in litres.
inline constexpr double kVanFuelPer100Km = 11.0;
/// @brief Fuel consumed by a refrigerated truck per 100 km, in litres.
inline constexpr double kRefrigeratedFuelPer100Km = 29.0;
/// @brief Percentage divisor used by utilization calculations.
inline constexpr double kPercentScale = 100.0;
/// @brief Maximum accepted driving license category count.
inline constexpr int kMaxLicenseValidityDays = 3650;

/**
 * @class DrivingLicense
 * @brief License with category and validity window.
 */
class DrivingLicense {
    public:
    /**
     * @brief Creates a license.
     * @throws InvalidLicenseException for an empty number or category, or an invalid window.
     */
    DrivingLicense(std::string number, std::string category, int issuedDay, int expiresDay);

    /// @brief Returns the license number.
    const std::string& Number() const { return number_; }

    /// @brief Returns the license category.
    const std::string& Category() const { return category_; }

    /// @brief Returns the issue day.
    int IssuedDay() const { return issuedDay_; }

    /// @brief Returns the expiry day.
    int ExpiresDay() const { return expiresDay_; }

    /**
     * @brief Checks whether the license is valid on a day.
     * @param day calendar day.
     */
    bool IsValidOn(int day) const;

    /**
     * @brief Checks whether the license covers a vehicle category.
     * @param category required category.
     */
    bool AllowsCategory(const std::string& category) const;

    /**
     * @brief Extends the license.
     * @param additionalDays extension in (0, kMaxLicenseValidityDays].
     * @throws InvalidLicenseException when the extension is out of range.
     */
    void Renew(int additionalDays);

    /**
     * @brief Days left before expiry.
     * @param day current calendar day.
     * @return remaining days, zero when already expired.
     */
    int DaysUntilExpiry(int day) const;

    private:
    std::string number_;
    std::string category_;
    int issuedDay_;
    int expiresDay_;
};

/**
 * @class MaintenanceRecord
 * @brief Service visit with cost and odometer reading.
 */
class MaintenanceRecord {
    public:
    /**
     * @brief Creates a maintenance record.
     * @throws InvalidVehicleException when an identifier is empty or a value is negative.
     */
    MaintenanceRecord(std::string id, std::string vehicleId, int day, std::string kind, double cost,
                      double odometerKm, std::string notes);

    /// @brief Returns the record identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the serviced vehicle identifier.
    const std::string& VehicleId() const { return vehicleId_; }

    /// @brief Returns the service day.
    int Day() const { return day_; }

    /// @brief Returns the service kind.
    const std::string& Kind() const { return kind_; }

    /// @brief Returns the service cost.
    double Cost() const { return cost_; }

    /// @brief Returns the odometer value at service time.
    double OdometerKm() const { return odometerKm_; }

    /// @brief Returns the free-form notes.
    const std::string& Notes() const { return notes_; }

    /**
     * @brief Checks whether the next service interval has elapsed.
     * @param currentOdometerKm current vehicle odometer.
     * @param intervalKm allowed interval between services.
     */
    bool IsOverdue(double currentOdometerKm, double intervalKm) const;

    /**
     * @brief Service cost per kilometre since the previous reading.
     * @param previousOdometerKm odometer before this service.
     */
    double CostPerKm(double previousOdometerKm) const;

    /// @brief Builds a one-line service description.
    std::string Describe() const;

    private:
    std::string id_;
    std::string vehicleId_;
    int day_;
    std::string kind_;
    double cost_;
    double odometerKm_;
    std::string notes_;
};

/**
 * @class FuelRecord
 * @brief Refuelling event with volume, price and odometer reading.
 */
class FuelRecord {
    public:
    /**
     * @brief Creates a fuel record.
     * @throws InvalidVehicleException when the vehicle is empty or a value is not positive.
     */
    FuelRecord(std::string vehicleId, double liters, double pricePerLiter, double odometerKm,
               int day);

    /// @brief Returns the vehicle identifier.
    const std::string& VehicleId() const { return vehicleId_; }

    /// @brief Returns the refuelled volume.
    double Liters() const { return liters_; }

    /// @brief Returns the price per litre.
    double PricePerLiter() const { return pricePerLiter_; }

    /// @brief Returns the odometer reading.
    double OdometerKm() const { return odometerKm_; }

    /// @brief Returns the refuelling day.
    int Day() const { return day_; }

    /// @brief Returns the total refuelling cost.
    double Cost() const;

    /**
     * @brief Fuel consumption since a previous reading.
     * @param previousOdometerKm odometer before this refuelling.
     * @throws InvalidVehicleException when the distance is not positive.
     */
    double ConsumptionPer100Km(double previousOdometerKm) const;

    private:
    std::string vehicleId_;
    double liters_;
    double pricePerLiter_;
    double odometerKm_;
    int day_;
};

/**
 * @class Vehicle
 * @brief Abstract transport unit with capacity, fuel and status.
 */
class Vehicle {
    public:
    /**
     * @brief Creates a vehicle in the available state.
     * @throws InvalidVehicleException for an empty identifier, plate or invalid measures.
     */
    Vehicle(std::string id, std::string plateNumber, double capacityKg, double capacityVolumeM3,
            double odometerKm, double fuelLiters);

    /// @brief Virtual destructor for polymorphic fleets.
    virtual ~Vehicle() = default;

    /// @brief Returns the vehicle identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the plate number.
    const std::string& PlateNumber() const { return plateNumber_; }

    /// @brief Returns the payload limit in kilograms.
    double CapacityKg() const { return capacityKg_; }

    /// @brief Returns the volume limit in cubic metres.
    double CapacityVolumeM3() const { return capacityVolumeM3_; }

    /// @brief Returns the odometer in kilometres.
    double OdometerKm() const { return odometerKm_; }

    /// @brief Returns the current fuel volume in litres.
    double FuelLiters() const { return fuelLiters_; }

    /// @brief Returns the operational status.
    VehicleStatus Status() const { return status_; }

    /**
     * @brief Checks whether the vehicle can carry a load.
     * @param weightKg cargo weight.
     * @param volumeM3 cargo volume.
     */
    bool CanCarry(double weightKg, double volumeM3) const;

    /**
     * @brief Validates a load against the vehicle limits.
     * @throws InvalidVehicleException when a measure is negative.
     * @throws VehicleOverloadException when weight or volume exceeds the capacity.
     */
    void ValidateLoad(double weightKg, double volumeM3) const;

    /**
     * @brief Ratio of a load to the payload limit.
     * @param weightKg cargo weight.
     * @return percentage in [0, 100] for a valid load.
     */
    double UtilizationPercent(double weightKg) const;

    /**
     * @brief Adds fuel to the tank.
     * @param liters volume to add.
     * @param tankCapacityLiters tank capacity.
     * @throws InvalidVehicleException when the volume is invalid or overflows the tank.
     */
    void Refuel(double liters, double tankCapacityLiters);

    /**
     * @brief Drives a distance and consumes fuel.
     * @param distanceKm distance in kilometres.
     * @throws InvalidVehicleException when the vehicle is unavailable or out of fuel.
     */
    void Drive(double distanceKm);

    /**
     * @brief Sends an available vehicle to maintenance.
     * @throws VehicleNotAvailableException when the vehicle is not available.
     */
    void SendToMaintenance();

    /**
     * @brief Returns a serviced vehicle to service.
     * @throws VehicleNotAvailableException when the vehicle is not in maintenance.
     */
    void ReturnToService();

    /**
     * @brief Marks an available vehicle as travelling.
     * @throws VehicleNotAvailableException when the vehicle is not available.
     */
    void MarkOnRoute();

    /**
     * @brief Marks a travelling vehicle as available.
     * @throws VehicleNotAvailableException when the vehicle is not on a route.
     */
    void CompleteRoute();

    /**
     * @brief Retires a vehicle from active service.
     * @throws VehicleNotAvailableException when the vehicle is on a route.
     */
    void Retire();

    /// @brief Checks whether the vehicle is available.
    bool IsAvailable() const { return status_ == VehicleStatus::Available; }

    /// @brief Returns the remaining range in kilometres.
    double RangeKm() const;

    /// @brief Returns the vehicle type name.
    virtual std::string TypeName() const = 0;

    /// @brief Returns fuel consumption per 100 km.
    virtual double FuelPer100Km() const = 0;

    protected:
    std::string id_;
    std::string plateNumber_;
    double capacityKg_;
    double capacityVolumeM3_;
    double odometerKm_;
    double fuelLiters_;
    VehicleStatus status_;
};

/**
 * @class Truck
 * @brief Heavy vehicle that can pull a trailer.
 */
class Truck : public Vehicle {
    public:
    /**
     * @brief Creates a truck.
     * @throws InvalidVehicleException for invalid axle or trailer limits.
     */
    Truck(std::string id, std::string plateNumber, double capacityKg, double capacityVolumeM3,
          double odometerKm, double fuelLiters, int axleCount, double maxTrailerWeightKg);

    /// @brief Returns the axle count.
    int AxleCount() const { return axleCount_; }

    /// @brief Returns the maximum trailer weight.
    double MaxTrailerWeightKg() const { return maxTrailerWeightKg_; }

    /**
     * @brief Checks whether the truck can pull a trailer.
     * @param trailerWeightKg loaded trailer weight.
     */
    bool CanPull(double trailerWeightKg) const;

    /// @brief Returns the vehicle type name.
    std::string TypeName() const override { return "truck"; }

    /// @brief Returns fuel consumption per 100 km.
    double FuelPer100Km() const override { return kTruckFuelPer100Km; }

    private:
    int axleCount_;
    double maxTrailerWeightKg_;
};

/**
 * @class Van
 * @brief Light vehicle for parcel deliveries.
 */
class Van : public Vehicle {
    public:
    /**
     * @brief Creates a van.
     * @throws InvalidVehicleException when the parcel limit is not positive.
     */
    Van(std::string id, std::string plateNumber, double capacityKg, double capacityVolumeM3,
        double odometerKm, double fuelLiters, bool refrigerated, int maxParcelCount);

    /// @brief Checks whether the van has refrigeration.
    bool IsRefrigerated() const { return refrigerated_; }

    /// @brief Returns the maximum parcel count.
    int MaxParcelCount() const { return maxParcelCount_; }

    /**
     * @brief Checks whether a parcel count fits the van.
     * @param parcelCount parcels to load.
     */
    bool FitsParcels(int parcelCount) const;

    /// @brief Returns the vehicle type name.
    std::string TypeName() const override { return "van"; }

    /// @brief Returns fuel consumption per 100 km.
    double FuelPer100Km() const override { return kVanFuelPer100Km; }

    private:
    bool refrigerated_;
    int maxParcelCount_;
};

/**
 * @class RefrigeratedTruck
 * @brief Truck with a controlled temperature compartment.
 */
class RefrigeratedTruck : public Truck {
    public:
    /**
     * @brief Creates a refrigerated truck.
     * @throws InvalidVehicleException for an inverted temperature range.
     */
    RefrigeratedTruck(std::string id, std::string plateNumber, double capacityKg,
                      double capacityVolumeM3, double odometerKm, double fuelLiters, int axleCount,
                      double maxTrailerWeightKg, double minTemperatureC, double maxTemperatureC);

    /// @brief Returns the minimum compartment temperature.
    double MinTemperatureC() const { return minTemperatureC_; }

    /// @brief Returns the maximum compartment temperature.
    double MaxTemperatureC() const { return maxTemperatureC_; }

    /**
     * @brief Checks whether the compartment can hold a required temperature.
     * @param requiredTemperatureC target temperature.
     */
    bool KeepsTemperature(double requiredTemperatureC) const;

    /// @brief Returns the vehicle type name.
    std::string TypeName() const override { return "refrigerated truck"; }

    /// @brief Returns fuel consumption per 100 km.
    double FuelPer100Km() const override { return kRefrigeratedFuelPer100Km; }

    private:
    double minTemperatureC_;
    double maxTemperatureC_;
};

/**
 * @class Trailer
 * @brief Detachable cargo container pulled by a truck.
 */
class Trailer {
    public:
    /**
     * @brief Creates an unattached trailer.
     * @throws InvalidVehicleException when an identifier is empty or a measure is invalid.
     */
    Trailer(std::string id, std::string plateNumber, double capacityKg, double volumeM3,
            bool refrigerated);

    /// @brief Returns the trailer identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the plate number.
    const std::string& PlateNumber() const { return plateNumber_; }

    /// @brief Returns the payload limit.
    double CapacityKg() const { return capacityKg_; }

    /// @brief Returns the volume limit.
    double VolumeM3() const { return volumeM3_; }

    /// @brief Checks whether the trailer is refrigerated.
    bool IsRefrigerated() const { return refrigerated_; }

    /// @brief Returns the attached vehicle identifier, empty when detached.
    const std::string& AttachedVehicleId() const { return attachedVehicleId_; }

    /**
     * @brief Attaches the trailer to a vehicle.
     * @throws InvalidVehicleException when the vehicle identifier is empty or already attached.
     */
    void AttachTo(const std::string& vehicleId);

    /// @brief Detaches the trailer from its vehicle.
    void Detach();

    /// @brief Checks whether the trailer is attached.
    bool IsAttached() const { return !attachedVehicleId_.empty(); }

    /**
     * @brief Checks whether the trailer can carry a load.
     * @param weightKg cargo weight.
     * @param volumeM3 cargo volume.
     */
    bool CanCarry(double weightKg, double volumeM3) const;

    private:
    std::string id_;
    std::string plateNumber_;
    double capacityKg_;
    double volumeM3_;
    bool refrigerated_;
    std::string attachedVehicleId_;
};

/**
 * @class VehicleFleet
 * @brief Owns vehicles and their service history.
 */
class VehicleFleet {
    public:
    /// @brief Creates an empty fleet.
    VehicleFleet() = default;

    /// @brief Fleets own vehicles and cannot be copied.
    VehicleFleet(const VehicleFleet&) = delete;
    /// @brief Fleets own vehicles and cannot be copy-assigned.
    VehicleFleet& operator=(const VehicleFleet&) = delete;
    /// @brief Moving a fleet transfers vehicle ownership.
    VehicleFleet(VehicleFleet&&) noexcept = default;
    /// @brief Moving a fleet transfers vehicle ownership.
    VehicleFleet& operator=(VehicleFleet&&) noexcept = default;

    /// @brief Returns the registered vehicles.
    const std::vector<std::unique_ptr<Vehicle>>& Vehicles() const { return vehicles_; }

    /// @brief Returns the maintenance history.
    const std::vector<MaintenanceRecord>& MaintenanceRecords() const { return maintenanceRecords_; }

    /// @brief Returns the fuel history.
    const std::vector<FuelRecord>& FuelRecords() const { return fuelRecords_; }

    /**
     * @brief Registers a vehicle.
     * @throws InvalidVehicleException when the pointer is null or the identifier is duplicated.
     */
    void AddVehicle(std::unique_ptr<Vehicle> vehicle);

    /**
     * @brief Finds a vehicle by identifier.
     * @return pointer to the vehicle, or nullptr.
     */
    Vehicle* FindVehicle(const std::string& vehicleId);

    /**
     * @brief Finds a vehicle by identifier in a const fleet.
     * @return pointer to the vehicle, or nullptr.
     */
    const Vehicle* FindVehicle(const std::string& vehicleId) const;

    /// @brief Returns pointers to all available vehicles.
    std::vector<Vehicle*> AvailableVehicles();

    /**
     * @brief Removes a vehicle from the fleet.
     * @return true when the vehicle was present.
     */
    bool RemoveVehicle(const std::string& vehicleId);

    /// @brief Returns the total payload capacity.
    double TotalCapacityKg() const;

    /// @brief Returns the percentage of available vehicles.
    double AvailabilityPercent() const;

    /**
     * @brief Records maintenance and sends the vehicle to the workshop.
     * @throws InvalidVehicleException when the vehicle does not exist or is unavailable.
     */
    void ScheduleMaintenance(const MaintenanceRecord& record);

    /**
     * @brief Records a refuelling event.
     * @throws InvalidVehicleException when the vehicle does not exist.
     */
    void RecordFuel(const FuelRecord& record);

    /// @brief Returns the number of registered vehicles.
    std::size_t VehicleCount() const { return vehicles_.size(); }

    private:
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
    std::vector<MaintenanceRecord> maintenanceRecords_;
    std::vector<FuelRecord> fuelRecords_;
};

}  // namespace logistics
