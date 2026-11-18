/**
 * @file test_warehouse.cpp
 * @brief Unit tests for warehouses, zones, racks, inventory and operations.
 */

#include <gtest/gtest.h>

#include "logistics/cargo.hpp"
#include "logistics/exceptions.hpp"
#include "logistics/warehouse.hpp"

namespace logistics {
namespace {

Address WarehouseAddress() {
    return Address("Depot", "Minsk", "220000", GeoPoint(53.9, 27.56));
}

Cargo MakeCargo(double weightKg, double volumeM3) {
    return Cargo("C1", "Pallets", weightKg, volumeM3, 500.0, "CL1");
}

}  // namespace

TEST(WarehouseTest, ValidatesRecord) {
    const Address address = WarehouseAddress();
    EXPECT_THROW(Warehouse("", "Hub", address, 100.0, 10.0), InvalidCargoException);
    EXPECT_THROW(Warehouse("W1", "", address, 100.0, 10.0), InvalidCargoException);
    EXPECT_THROW(Warehouse("W1", "Hub", address, 0.0, 10.0), InvalidCargoException);
    EXPECT_THROW(Warehouse("W1", "Hub", address, 100.0, 0.0), InvalidCargoException);
}

TEST(WarehouseTest, StoresAndReleasesCargo) {
    Warehouse warehouse("W1", "Hub", WarehouseAddress(), 1000.0, 10.0);
    EXPECT_TRUE(warehouse.CanStore(1000.0, 10.0));
    EXPECT_FALSE(warehouse.CanStore(1001.0, 1.0));
    EXPECT_FALSE(warehouse.CanStore(1.0, 11.0));
    EXPECT_FALSE(warehouse.CanStore(-1.0, 1.0));
    EXPECT_FALSE(warehouse.CanStore(1.0, -1.0));
    warehouse.Store(MakeCargo(100.0, 2.0));
    EXPECT_DOUBLE_EQ(warehouse.UsedKg(), 100.0);
    EXPECT_DOUBLE_EQ(warehouse.UsedM3(), 2.0);
    EXPECT_DOUBLE_EQ(warehouse.UtilizationPercent(), 20.0);
    EXPECT_FALSE(warehouse.IsFull());
    EXPECT_THROW(warehouse.Store(MakeCargo(1000.0, 1.0)), WarehouseCapacityException);
    warehouse.Release(100.0, 2.0);
    EXPECT_DOUBLE_EQ(warehouse.UsedKg(), 0.0);
    EXPECT_THROW(warehouse.Release(-1.0, 1.0), WarehouseCapacityException);
    EXPECT_THROW(warehouse.Release(1.0, 0.0), WarehouseCapacityException);
    EXPECT_EQ(warehouse.Describe(), "Hub (W1) at Minsk, stored 0.000000 kg");
    EXPECT_EQ(warehouse.Id(), "W1");
    EXPECT_EQ(warehouse.Name(), "Hub");
    EXPECT_EQ(warehouse.Location().City(), "Minsk");
    EXPECT_DOUBLE_EQ(warehouse.CapacityKg(), 1000.0);
    EXPECT_DOUBLE_EQ(warehouse.CapacityM3(), 10.0);
}

TEST(WarehouseTest, ReachesFullCapacity) {
    Warehouse warehouse("W1", "Hub", WarehouseAddress(), 100.0, 1.0);
    warehouse.Store(MakeCargo(100.0, 1.0));
    EXPECT_TRUE(warehouse.IsFull());
    EXPECT_DOUBLE_EQ(warehouse.UtilizationPercent(), 100.0);
    EXPECT_FALSE(warehouse.CanStore(1.0, 0.0));
}

TEST(StorageZoneTest, ManagesSlots) {
    EXPECT_THROW(StorageZone("", "Cold", 2.0, 10), InvalidCargoException);
    EXPECT_THROW(StorageZone("Z1", "", 2.0, 10), InvalidCargoException);
    EXPECT_THROW(StorageZone("Z1", "Cold", 2.0, 0), InvalidCargoException);
    StorageZone zone("Z1", "Cold", 4.0, 10);
    EXPECT_TRUE(zone.Fits(10));
    EXPECT_FALSE(zone.Fits(11));
    EXPECT_FALSE(zone.Fits(-1));
    EXPECT_THROW(zone.Occupy(0), WarehouseCapacityException);
    EXPECT_THROW(zone.Occupy(11), WarehouseCapacityException);
    zone.Occupy(5);
    EXPECT_DOUBLE_EQ(zone.UtilizationPercent(), 50.0);
    EXPECT_THROW(zone.Release(6), WarehouseCapacityException);
    EXPECT_THROW(zone.Release(0), WarehouseCapacityException);
    zone.Release(5);
    EXPECT_EQ(zone.OccupiedSlots(), 0);
    EXPECT_EQ(zone.TargetTemperatureC(), 4.0);
    EXPECT_EQ(zone.CapacitySlots(), 10);
    EXPECT_EQ(zone.Id(), "Z1");
    EXPECT_EQ(zone.Name(), "Cold");
}

TEST(StorageZoneTest, ChecksPerishableTemperature) {
    const StorageZone chill("Z1", "Chill", 4.0, 10);
    const StorageZone frozen("Z2", "Frozen", -18.0, 10);
    const PerishableCargo milk("C1", "Milk", 10.0, 1.0, 100.0, "CL1", 2.0, 6.0, 48);
    EXPECT_TRUE(chill.IsTemperatureSuitable(milk));
    EXPECT_FALSE(frozen.IsTemperatureSuitable(milk));
}

TEST(RackTest, OccupiesAndReleasesSlots) {
    EXPECT_THROW(Rack("", "Z1", 2), WarehouseCapacityException);
    EXPECT_THROW(Rack("R1", "", 2), WarehouseCapacityException);
    EXPECT_THROW(Rack("R1", "Z1", 0), WarehouseCapacityException);
    Rack rack("R1", "Z1", 2);
    EXPECT_FALSE(rack.IsFull());
    rack.PlaceSlot();
    rack.PlaceSlot();
    EXPECT_TRUE(rack.IsFull());
    EXPECT_THROW(rack.PlaceSlot(), WarehouseCapacityException);
    rack.RemoveSlot();
    rack.RemoveSlot();
    EXPECT_THROW(rack.RemoveSlot(), WarehouseCapacityException);
    EXPECT_EQ(rack.Id(), "R1");
    EXPECT_EQ(rack.ZoneId(), "Z1");
    EXPECT_EQ(rack.CapacitySlots(), 2);
    EXPECT_EQ(rack.OccupiedSlots(), 0);
}

TEST(InventoryRecordTest, ValidatesAndAges) {
    EXPECT_THROW(InventoryRecord("", "W1", "Z1", 0, 1), InvalidCargoException);
    EXPECT_THROW(InventoryRecord("C1", "", "Z1", 0, 1), InvalidCargoException);
    EXPECT_THROW(InventoryRecord("C1", "W1", "", 0, 1), InvalidCargoException);
    EXPECT_THROW(InventoryRecord("C1", "W1", "Z1", -1, 1), InvalidCargoException);
    EXPECT_THROW(InventoryRecord("C1", "W1", "Z1", 0, 0), InvalidCargoException);
    InventoryRecord record("C1", "W1", "Z1", 2, 10);
    EXPECT_EQ(record.AgeDays(5), 3);
    EXPECT_EQ(record.AgeDays(1), 0);
    EXPECT_FALSE(record.IsOverdue(4, 2));
    EXPECT_TRUE(record.IsOverdue(5, 2));
    EXPECT_THROW(record.IsOverdue(5, -1), InvalidCargoException);
    EXPECT_EQ(record.Describe(), "C1 in W1/Z1, quantity 10");
    EXPECT_EQ(record.CargoId(), "C1");
    EXPECT_EQ(record.WarehouseId(), "W1");
    EXPECT_EQ(record.ZoneId(), "Z1");
    EXPECT_EQ(record.StoredDay(), 2);
    EXPECT_EQ(record.Quantity(), 10);
}

TEST(WarehouseOperationTest, ClassifiesMovements) {
    EXPECT_THROW(WarehouseOperation("", "W1", WarehouseOperationKind::Inbound, "C1", 1, 0),
                 InvalidShipmentException);
    EXPECT_THROW(WarehouseOperation("OP1", "", WarehouseOperationKind::Inbound, "C1", 1, 0),
                 InvalidShipmentException);
    EXPECT_THROW(WarehouseOperation("OP1", "W1", WarehouseOperationKind::Inbound, "", 1, 0),
                 InvalidShipmentException);
    EXPECT_THROW(WarehouseOperation("OP1", "W1", WarehouseOperationKind::Inbound, "C1", 0, 0),
                 InvalidShipmentException);
    EXPECT_THROW(WarehouseOperation("OP1", "W1", WarehouseOperationKind::Inbound, "C1", 1, -1),
                 InvalidShipmentException);
    const WarehouseOperation inbound("OP1", "W1", WarehouseOperationKind::Inbound, "C1", 10, 1);
    const WarehouseOperation outbound("OP2", "W1", WarehouseOperationKind::Outbound, "C1", 5, 2);
    const WarehouseOperation transfer("OP3", "W1", WarehouseOperationKind::Transfer, "C1", 2, 3);
    EXPECT_TRUE(inbound.IsInbound());
    EXPECT_FALSE(inbound.IsOutbound());
    EXPECT_TRUE(outbound.IsOutbound());
    EXPECT_FALSE(outbound.IsInbound());
    EXPECT_FALSE(transfer.IsInbound());
    EXPECT_FALSE(transfer.IsOutbound());
    EXPECT_EQ(inbound.Describe(), "OP1 at W1, cargo C1, units 10");
    EXPECT_EQ(inbound.Id(), "OP1");
    EXPECT_EQ(inbound.WarehouseId(), "W1");
    EXPECT_EQ(inbound.Kind(), WarehouseOperationKind::Inbound);
    EXPECT_EQ(inbound.CargoId(), "C1");
    EXPECT_EQ(inbound.Units(), 10);
    EXPECT_EQ(inbound.Day(), 1);
}

}  // namespace logistics
