#pragma once

#include <string>
#include <vector>

#include "logistics/cargo.hpp"
#include "logistics/geo.hpp"

/**
 * @file warehouse.hpp
 * @brief Warehouses, storage zones, racks and inventory records.
 */

namespace logistics {

/// @brief Maximum accepted hour of the day, shared by scheduling code.
inline constexpr double kWarehousePercentScale = 100.0;

/// @brief Kind of a warehouse operation.
enum class WarehouseOperationKind { Inbound, Outbound, Transfer };

/**
 * @class Warehouse
 * @brief Storage facility with weight and volume capacity.
 */
class Warehouse {
    public:
    /**
     * @brief Creates a warehouse.
     * @throws InvalidCargoException for empty identifiers or invalid capacities.
     */
    Warehouse(std::string id, std::string name, Address address, double capacityKg,
              double capacityM3);

    /// @brief Returns the warehouse identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the warehouse name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the warehouse address.
    const Address& Location() const { return address_; }

    /// @brief Returns the weight capacity.
    double CapacityKg() const { return capacityKg_; }

    /// @brief Returns the used weight capacity.
    double UsedKg() const { return usedKg_; }

    /// @brief Returns the volume capacity.
    double CapacityM3() const { return capacityM3_; }

    /// @brief Returns the used volume capacity.
    double UsedM3() const { return usedM3_; }

    /**
     * @brief Checks whether a load fits the remaining capacity.
     * @param weightKg load weight.
     * @param volumeM3 load volume.
     */
    bool CanStore(double weightKg, double volumeM3) const;

    /**
     * @brief Stores cargo in the warehouse.
     * @throws WarehouseCapacityException when the load does not fit.
     */
    void Store(const Cargo& cargo);

    /**
     * @brief Releases capacity from the warehouse.
     * @throws WarehouseCapacityException when more is released than stored.
     */
    void Release(double weightKg, double volumeM3);

    /// @brief Returns the higher of weight and volume utilization.
    double UtilizationPercent() const;

    /// @brief Checks whether any capacity is exhausted.
    bool IsFull() const;

    /// @brief Builds a one-line warehouse summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string name_;
    Address address_;
    double capacityKg_;
    double usedKg_;
    double capacityM3_;
    double usedM3_;
};

/**
 * @class StorageZone
 * @brief Temperature-controlled area with a slot count.
 */
class StorageZone {
    public:
    /**
     * @brief Creates a storage zone.
     * @throws InvalidCargoException for empty identifiers or invalid capacity.
     */
    StorageZone(std::string id, std::string name, double targetTemperatureC, int capacitySlots);

    /// @brief Returns the zone identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the zone name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the target temperature.
    double TargetTemperatureC() const { return targetTemperatureC_; }

    /// @brief Returns the slot capacity.
    int CapacitySlots() const { return capacitySlots_; }

    /// @brief Returns the occupied slots.
    int OccupiedSlots() const { return occupiedSlots_; }

    /**
     * @brief Checks whether a number of slots fits.
     * @param slots requested slots.
     */
    bool Fits(int slots) const;

    /**
     * @brief Occupies slots.
     * @throws WarehouseCapacityException when there are not enough free slots.
     */
    void Occupy(int slots);

    /**
     * @brief Releases occupied slots.
     * @throws WarehouseCapacityException when more slots are released than occupied.
     */
    void Release(int slots);

    /**
     * @brief Checks whether the zone temperature suits perishable cargo.
     * @param cargo perishable cargo.
     */
    bool IsTemperatureSuitable(const PerishableCargo& cargo) const;

    /// @brief Returns slot utilization as a percentage.
    double UtilizationPercent() const;

    private:
    std::string id_;
    std::string name_;
    double targetTemperatureC_;
    int capacitySlots_;
    int occupiedSlots_;
};

/**
 * @class Rack
 * @brief Storage rack inside a zone.
 */
class Rack {
    public:
    /**
     * @brief Creates a rack.
     * @throws WarehouseCapacityException for an empty identifier or invalid capacity.
     */
    Rack(std::string id, std::string zoneId, int capacitySlots);

    /// @brief Returns the rack identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the zone identifier.
    const std::string& ZoneId() const { return zoneId_; }

    /// @brief Returns the slot capacity.
    int CapacitySlots() const { return capacitySlots_; }

    /// @brief Returns the occupied slots.
    int OccupiedSlots() const { return occupiedSlots_; }

    /**
     * @brief Occupies one slot.
     * @throws WarehouseCapacityException when the rack is full.
     */
    void PlaceSlot();

    /**
     * @brief Releases one slot.
     * @throws WarehouseCapacityException when the rack is empty.
     */
    void RemoveSlot();

    /// @brief Checks whether the rack is full.
    bool IsFull() const;

    private:
    std::string id_;
    std::string zoneId_;
    int capacitySlots_;
    int occupiedSlots_;
};

/**
 * @class InventoryRecord
 * @brief Cargo stored in a warehouse zone since a given day.
 */
class InventoryRecord {
    public:
    /**
     * @brief Creates an inventory record.
     * @throws InvalidCargoException for empty identifiers or invalid quantity/day.
     */
    InventoryRecord(std::string cargoId, std::string warehouseId, std::string zoneId, int storedDay,
                    int quantity);

    /// @brief Returns the cargo identifier.
    const std::string& CargoId() const { return cargoId_; }

    /// @brief Returns the warehouse identifier.
    const std::string& WarehouseId() const { return warehouseId_; }

    /// @brief Returns the zone identifier.
    const std::string& ZoneId() const { return zoneId_; }

    /// @brief Returns the storage day.
    int StoredDay() const { return storedDay_; }

    /// @brief Returns the stored quantity.
    int Quantity() const { return quantity_; }

    /**
     * @brief Days spent in storage.
     * @param currentDay current calendar day.
     */
    int AgeDays(int currentDay) const;

    /**
     * @brief Checks whether storage exceeds a limit.
     * @param currentDay current calendar day.
     * @param maxDays maximum storage duration.
     */
    bool IsOverdue(int currentDay, int maxDays) const;

    /// @brief Builds a one-line inventory description.
    std::string Describe() const;

    private:
    std::string cargoId_;
    std::string warehouseId_;
    std::string zoneId_;
    int storedDay_;
    int quantity_;
};

/**
 * @class WarehouseOperation
 * @brief Inbound, outbound or transfer movement of cargo.
 */
class WarehouseOperation {
    public:
    /**
     * @brief Creates a warehouse operation.
     * @throws InvalidShipmentException for empty identifiers or invalid units/day.
     */
    WarehouseOperation(std::string id, std::string warehouseId, WarehouseOperationKind kind,
                       std::string cargoId, int units, int day);

    /// @brief Returns the operation identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the warehouse identifier.
    const std::string& WarehouseId() const { return warehouseId_; }

    /// @brief Returns the operation kind.
    WarehouseOperationKind Kind() const { return kind_; }

    /// @brief Returns the cargo identifier.
    const std::string& CargoId() const { return cargoId_; }

    /// @brief Returns the moved unit count.
    int Units() const { return units_; }

    /// @brief Returns the operation day.
    int Day() const { return day_; }

    /// @brief Checks whether cargo enters the warehouse.
    bool IsInbound() const;

    /// @brief Checks whether cargo leaves the warehouse.
    bool IsOutbound() const;

    /// @brief Builds a one-line operation description.
    std::string Describe() const;

    private:
    std::string id_;
    std::string warehouseId_;
    WarehouseOperationKind kind_;
    std::string cargoId_;
    int units_;
    int day_;
};

}  // namespace logistics
