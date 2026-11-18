#include "logistics/warehouse.hpp"

#include <algorithm>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {

Warehouse::Warehouse(std::string id, std::string name, Address address, double capacityKg,
                     double capacityM3)
    : id_(std::move(id)),
      name_(std::move(name)),
      address_(std::move(address)),
      capacityKg_(capacityKg),
      usedKg_(0.0),
      capacityM3_(capacityM3),
      usedM3_(0.0) {
    if (id_.empty() || name_.empty() || capacityKg_ <= 0.0 || capacityM3_ <= 0.0) {
        throw InvalidCargoException("Warehouse requires an identifier, name and positive capacities");
    }
}

bool Warehouse::CanStore(double weightKg, double volumeM3) const {
    if (weightKg < 0.0 || volumeM3 < 0.0) {
        return false;
    }
    return usedKg_ + weightKg <= capacityKg_ && usedM3_ + volumeM3 <= capacityM3_;
}

void Warehouse::Store(const Cargo& cargo) {
    if (!CanStore(cargo.WeightKg(), cargo.VolumeM3())) {
        throw WarehouseCapacityException("Warehouse cannot store the cargo");
    }
    usedKg_ += cargo.WeightKg();
    usedM3_ += cargo.VolumeM3();
}

void Warehouse::Release(double weightKg, double volumeM3) {
    if (weightKg < 0.0 || volumeM3 < 0.0 || weightKg > usedKg_ || volumeM3 > usedM3_) {
        throw WarehouseCapacityException("Released capacity exceeds the stored capacity");
    }
    usedKg_ -= weightKg;
    usedM3_ -= volumeM3;
}

double Warehouse::UtilizationPercent() const {
    const double weightRatio = usedKg_ / capacityKg_;
    const double volumeRatio = usedM3_ / capacityM3_;
    return std::max(weightRatio, volumeRatio) * kWarehousePercentScale;
}

bool Warehouse::IsFull() const {
    return usedKg_ >= capacityKg_ || usedM3_ >= capacityM3_;
}

std::string Warehouse::Describe() const {
    return name_ + " (" + id_ + ") at " + address_.City() + ", stored " + std::to_string(usedKg_) +
           " kg";
}

StorageZone::StorageZone(std::string id, std::string name, double targetTemperatureC,
                         int capacitySlots)
    : id_(std::move(id)),
      name_(std::move(name)),
      targetTemperatureC_(targetTemperatureC),
      capacitySlots_(capacitySlots),
      occupiedSlots_(0) {
    if (id_.empty() || name_.empty() || capacitySlots_ <= 0) {
        throw InvalidCargoException("Storage zone requires an identifier, name and positive capacity");
    }
}

bool StorageZone::Fits(int slots) const {
    return slots >= 0 && occupiedSlots_ + slots <= capacitySlots_;
}

void StorageZone::Occupy(int slots) {
    if (slots <= 0 || !Fits(slots)) {
        throw WarehouseCapacityException("Storage zone does not have enough free slots");
    }
    occupiedSlots_ += slots;
}

void StorageZone::Release(int slots) {
    if (slots <= 0 || slots > occupiedSlots_) {
        throw WarehouseCapacityException("Released slots exceed the occupied slots");
    }
    occupiedSlots_ -= slots;
}

bool StorageZone::IsTemperatureSuitable(const PerishableCargo& cargo) const {
    return cargo.IsTemperatureCompatible(targetTemperatureC_);
}

double StorageZone::UtilizationPercent() const {
    const double ratio = static_cast<double>(occupiedSlots_) / static_cast<double>(capacitySlots_);
    return ratio * kWarehousePercentScale;
}

Rack::Rack(std::string id, std::string zoneId, int capacitySlots)
    : id_(std::move(id)),
      zoneId_(std::move(zoneId)),
      capacitySlots_(capacitySlots),
      occupiedSlots_(0) {
    if (id_.empty() || zoneId_.empty() || capacitySlots_ <= 0) {
        throw WarehouseCapacityException("Rack requires an identifier, zone and positive capacity");
    }
}

void Rack::PlaceSlot() {
    if (IsFull()) {
        throw WarehouseCapacityException("Rack is full");
    }
    ++occupiedSlots_;
}

void Rack::RemoveSlot() {
    if (occupiedSlots_ == 0) {
        throw WarehouseCapacityException("Rack is empty");
    }
    --occupiedSlots_;
}

bool Rack::IsFull() const { return occupiedSlots_ >= capacitySlots_; }

InventoryRecord::InventoryRecord(std::string cargoId, std::string warehouseId, std::string zoneId,
                                 int storedDay, int quantity)
    : cargoId_(std::move(cargoId)),
      warehouseId_(std::move(warehouseId)),
      zoneId_(std::move(zoneId)),
      storedDay_(storedDay),
      quantity_(quantity) {
    if (cargoId_.empty() || warehouseId_.empty() || zoneId_.empty() || storedDay_ < 0 ||
        quantity_ <= 0) {
        throw InvalidCargoException("Inventory record has an invalid identifier, day or quantity");
    }
}

int InventoryRecord::AgeDays(int currentDay) const { return std::max(0, currentDay - storedDay_); }

bool InventoryRecord::IsOverdue(int currentDay, int maxDays) const {
    if (maxDays < 0) {
        throw InvalidCargoException("Maximum storage duration cannot be negative");
    }
    return AgeDays(currentDay) > maxDays;
}

std::string InventoryRecord::Describe() const {
    return cargoId_ + " in " + warehouseId_ + "/" + zoneId_ + ", quantity " +
           std::to_string(quantity_);
}

WarehouseOperation::WarehouseOperation(std::string id, std::string warehouseId,
                                       WarehouseOperationKind kind, std::string cargoId, int units,
                                       int day)
    : id_(std::move(id)),
      warehouseId_(std::move(warehouseId)),
      kind_(kind),
      cargoId_(std::move(cargoId)),
      units_(units),
      day_(day) {
    if (id_.empty() || warehouseId_.empty() || cargoId_.empty() || units_ <= 0 || day_ < 0) {
        throw InvalidShipmentException("Warehouse operation has an invalid identifier or value");
    }
}

bool WarehouseOperation::IsInbound() const { return kind_ == WarehouseOperationKind::Inbound; }

bool WarehouseOperation::IsOutbound() const { return kind_ == WarehouseOperationKind::Outbound; }

std::string WarehouseOperation::Describe() const {
    return id_ + " at " + warehouseId_ + ", cargo " + cargoId_ + ", units " + std::to_string(units_);
}

}  // namespace logistics
