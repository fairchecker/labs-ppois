/**
 * @file test_vehicles.cpp
 * @brief Unit tests for vehicles, licenses, service records and the fleet.
 */

#include <gtest/gtest.h>

#include <memory>

#include "logistics/exceptions.hpp"
#include "logistics/vehicles.hpp"

namespace logistics {
namespace {

DrivingLicense MakeLicense() { return DrivingLicense("LIC-1", "C", 0, 100); }

MaintenanceRecord MakeMaintenance() {
    return MaintenanceRecord("M1", "T1", 10, "oil", 250.0, 10000.0, "routine");
}

FuelRecord MakeFuel() { return FuelRecord("T1", 50.0, 2.0, 1500.0, 12); }

std::unique_ptr<Truck> MakeTruck() {
    return std::make_unique<Truck>("T1", "AB1234", 1000.0, 10.0, 100.0, 50.0, 3, 5000.0);
}

}  // namespace

TEST(DrivingLicenseTest, ValidatesWindow) {
    EXPECT_THROW(DrivingLicense("", "C", 0, 100), InvalidLicenseException);
    EXPECT_THROW(DrivingLicense("L", "", 0, 100), InvalidLicenseException);
    EXPECT_THROW(DrivingLicense("L", "C", -1, 100), InvalidLicenseException);
    EXPECT_THROW(DrivingLicense("L", "C", 100, 100), InvalidLicenseException);
}

TEST(DrivingLicenseTest, ChecksValidityAndRenewal) {
    DrivingLicense license = MakeLicense();
    EXPECT_TRUE(license.IsValidOn(0));
    EXPECT_TRUE(license.IsValidOn(100));
    EXPECT_FALSE(license.IsValidOn(101));
    EXPECT_TRUE(license.AllowsCategory("C"));
    EXPECT_FALSE(license.AllowsCategory("B"));
    EXPECT_EQ(license.DaysUntilExpiry(90), 10);
    EXPECT_EQ(license.DaysUntilExpiry(120), 0);
    license.Renew(365);
    EXPECT_EQ(license.ExpiresDay(), 465);
    EXPECT_THROW(license.Renew(0), InvalidLicenseException);
    EXPECT_THROW(license.Renew(4000), InvalidLicenseException);
    EXPECT_EQ(license.Number(), "LIC-1");
    EXPECT_EQ(license.Category(), "C");
    EXPECT_EQ(license.IssuedDay(), 0);
}

TEST(MaintenanceRecordTest, ValidatesRecord) {
    EXPECT_THROW(MaintenanceRecord("", "T1", 1, "oil", 10.0, 100.0, ""), InvalidVehicleException);
    EXPECT_THROW(MaintenanceRecord("M1", "", 1, "oil", 10.0, 100.0, ""), InvalidVehicleException);
    EXPECT_THROW(MaintenanceRecord("M1", "T1", -1, "oil", 10.0, 100.0, ""), InvalidVehicleException);
    EXPECT_THROW(MaintenanceRecord("M1", "T1", 1, "", 10.0, 100.0, ""), InvalidVehicleException);
    EXPECT_THROW(MaintenanceRecord("M1", "T1", 1, "oil", -1.0, 100.0, ""), InvalidVehicleException);
    EXPECT_THROW(MaintenanceRecord("M1", "T1", 1, "oil", 10.0, -1.0, ""), InvalidVehicleException);
}

TEST(MaintenanceRecordTest, ComputesServiceMetrics) {
    const MaintenanceRecord record = MakeMaintenance();
    EXPECT_FALSE(record.IsOverdue(24999.0, 15000.0));
    EXPECT_TRUE(record.IsOverdue(25000.0, 15000.0));
    EXPECT_THROW(record.IsOverdue(25000.0, 0.0), InvalidVehicleException);
    EXPECT_DOUBLE_EQ(record.CostPerKm(9000.0), 0.25);
    EXPECT_THROW(record.CostPerKm(10000.0), InvalidVehicleException);
    EXPECT_EQ(record.Describe(), "oil on day 10, cost 250.000000");
    EXPECT_EQ(record.Id(), "M1");
    EXPECT_EQ(record.VehicleId(), "T1");
    EXPECT_EQ(record.Kind(), "oil");
    EXPECT_EQ(record.Notes(), "routine");
}

TEST(FuelRecordTest, ValidatesAndComputes) {
    EXPECT_THROW(FuelRecord("", 10.0, 2.0, 100.0, 1), InvalidVehicleException);
    EXPECT_THROW(FuelRecord("T1", 0.0, 2.0, 100.0, 1), InvalidVehicleException);
    EXPECT_THROW(FuelRecord("T1", 10.0, 0.0, 100.0, 1), InvalidVehicleException);
    EXPECT_THROW(FuelRecord("T1", 10.0, 2.0, -1.0, 1), InvalidVehicleException);
    EXPECT_THROW(FuelRecord("T1", 10.0, 2.0, 100.0, -1), InvalidVehicleException);
    const FuelRecord record = MakeFuel();
    EXPECT_DOUBLE_EQ(record.Cost(), 100.0);
    EXPECT_DOUBLE_EQ(record.ConsumptionPer100Km(1000.0), 10.0);
    EXPECT_THROW(record.ConsumptionPer100Km(1500.0), InvalidVehicleException);
    EXPECT_EQ(record.VehicleId(), "T1");
    EXPECT_EQ(record.Liters(), 50.0);
    EXPECT_EQ(record.PricePerLiter(), 2.0);
    EXPECT_EQ(record.OdometerKm(), 1500.0);
    EXPECT_EQ(record.Day(), 12);
}

TEST(VehicleTest, ValidatesBaseRecord) {
    EXPECT_THROW(Truck("", "P", 100.0, 1.0, 0.0, 0.0, 2, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "", 100.0, 1.0, 0.0, 0.0, 2, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "P", 0.0, 1.0, 0.0, 0.0, 2, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "P", 100.0, 0.0, 0.0, 0.0, 2, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "P", 100.0, 1.0, -1.0, 0.0, 2, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "P", 100.0, 1.0, 0.0, -1.0, 2, 100.0), InvalidVehicleException);
}

TEST(VehicleTest, ChecksLoadAndUtilization) {
    const std::unique_ptr<Truck> truck = MakeTruck();
    EXPECT_TRUE(truck->CanCarry(1000.0, 10.0));
    EXPECT_FALSE(truck->CanCarry(1001.0, 10.0));
    EXPECT_FALSE(truck->CanCarry(1000.0, 11.0));
    EXPECT_FALSE(truck->CanCarry(-1.0, 1.0));
    EXPECT_FALSE(truck->CanCarry(1.0, -1.0));
    EXPECT_DOUBLE_EQ(truck->UtilizationPercent(500.0), 50.0);
    EXPECT_DOUBLE_EQ(truck->UtilizationPercent(2000.0), 100.0);
    EXPECT_THROW(truck->UtilizationPercent(-1.0), InvalidVehicleException);
    truck->ValidateLoad(1000.0, 10.0);
    EXPECT_THROW(truck->ValidateLoad(1001.0, 10.0), VehicleOverloadException);
    EXPECT_THROW(truck->ValidateLoad(10.0, 11.0), VehicleOverloadException);
    EXPECT_THROW(truck->ValidateLoad(-1.0, 1.0), InvalidVehicleException);
    EXPECT_EQ(truck->Id(), "T1");
    EXPECT_EQ(truck->PlateNumber(), "AB1234");
    EXPECT_DOUBLE_EQ(truck->CapacityKg(), 1000.0);
    EXPECT_DOUBLE_EQ(truck->CapacityVolumeM3(), 10.0);
    EXPECT_DOUBLE_EQ(truck->OdometerKm(), 100.0);
    EXPECT_DOUBLE_EQ(truck->FuelLiters(), 50.0);
}

TEST(VehicleTest, RefuelsWithinTankCapacity) {
    const std::unique_ptr<Truck> truck = MakeTruck();
    truck->Refuel(10.0, 100.0);
    EXPECT_DOUBLE_EQ(truck->FuelLiters(), 60.0);
    EXPECT_THROW(truck->Refuel(0.0, 100.0), InvalidVehicleException);
    EXPECT_THROW(truck->Refuel(10.0, 0.0), InvalidVehicleException);
    EXPECT_THROW(truck->Refuel(50.0, 100.0), InvalidVehicleException);
}

TEST(VehicleTest, DrivesAndConsumesFuel) {
    const std::unique_ptr<Truck> truck = MakeTruck();
    EXPECT_DOUBLE_EQ(truck->RangeKm(), 50.0 / 24.0 * 100.0);
    truck->Drive(100.0);
    EXPECT_DOUBLE_EQ(truck->OdometerKm(), 200.0);
    EXPECT_DOUBLE_EQ(truck->FuelLiters(), 26.0);
    EXPECT_THROW(truck->Drive(0.0), InvalidVehicleException);
    EXPECT_THROW(truck->Drive(200.0), InvalidVehicleException);
}

TEST(VehicleTest, EnforcesStateMachine) {
    const std::unique_ptr<Truck> truck = MakeTruck();
    EXPECT_TRUE(truck->IsAvailable());
    truck->SendToMaintenance();
    EXPECT_FALSE(truck->IsAvailable());
    EXPECT_THROW(truck->SendToMaintenance(), VehicleNotAvailableException);
    EXPECT_THROW(truck->Drive(10.0), VehicleNotAvailableException);
    truck->ReturnToService();
    EXPECT_TRUE(truck->IsAvailable());
    EXPECT_THROW(truck->ReturnToService(), VehicleNotAvailableException);
    truck->MarkOnRoute();
    EXPECT_THROW(truck->MarkOnRoute(), VehicleNotAvailableException);
    EXPECT_THROW(truck->Retire(), VehicleNotAvailableException);
    truck->CompleteRoute();
    EXPECT_THROW(truck->CompleteRoute(), VehicleNotAvailableException);
    truck->Retire();
    EXPECT_FALSE(truck->IsAvailable());
    EXPECT_THROW(truck->MarkOnRoute(), VehicleNotAvailableException);
}

TEST(TruckTest, ValidatesAxlesAndPullsTrailers) {
    EXPECT_THROW(Truck("T", "P", 100.0, 1.0, 0.0, 0.0, 1, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "P", 100.0, 1.0, 0.0, 0.0, 13, 100.0), InvalidVehicleException);
    EXPECT_THROW(Truck("T", "P", 100.0, 1.0, 0.0, 0.0, 2, -1.0), InvalidVehicleException);
    const std::unique_ptr<Truck> truck = MakeTruck();
    EXPECT_TRUE(truck->CanPull(5000.0));
    EXPECT_FALSE(truck->CanPull(5001.0));
    EXPECT_FALSE(truck->CanPull(0.0));
    EXPECT_EQ(truck->AxleCount(), 3);
    EXPECT_DOUBLE_EQ(truck->MaxTrailerWeightKg(), 5000.0);
    EXPECT_EQ(truck->TypeName(), "truck");
    EXPECT_DOUBLE_EQ(truck->FuelPer100Km(), kTruckFuelPer100Km);
}

TEST(VanTest, ChecksParcelCapacity) {
    EXPECT_THROW(Van("V", "P", 100.0, 1.0, 0.0, 0.0, false, 0), InvalidVehicleException);
    const Van van("V1", "AC1", 100.0, 1.0, 0.0, 0.0, true, 10);
    EXPECT_TRUE(van.FitsParcels(0));
    EXPECT_TRUE(van.FitsParcels(10));
    EXPECT_FALSE(van.FitsParcels(11));
    EXPECT_FALSE(van.FitsParcels(-1));
    EXPECT_TRUE(van.IsRefrigerated());
    EXPECT_EQ(van.MaxParcelCount(), 10);
    EXPECT_EQ(van.TypeName(), "van");
    EXPECT_DOUBLE_EQ(van.FuelPer100Km(), kVanFuelPer100Km);
}

TEST(RefrigeratedTruckTest, ChecksTemperatureRange) {
    EXPECT_THROW(RefrigeratedTruck("R", "P", 100.0, 1.0, 0.0, 0.0, 2, 100.0, 5.0, 5.0),
                 InvalidVehicleException);
    const RefrigeratedTruck truck("R1", "AC2", 100.0, 1.0, 0.0, 0.0, 2, 100.0, -20.0, 5.0);
    EXPECT_TRUE(truck.KeepsTemperature(-20.0));
    EXPECT_TRUE(truck.KeepsTemperature(5.0));
    EXPECT_FALSE(truck.KeepsTemperature(-21.0));
    EXPECT_FALSE(truck.KeepsTemperature(6.0));
    EXPECT_EQ(truck.TypeName(), "refrigerated truck");
    EXPECT_DOUBLE_EQ(truck.FuelPer100Km(), kRefrigeratedFuelPer100Km);
    EXPECT_DOUBLE_EQ(truck.MinTemperatureC(), -20.0);
    EXPECT_DOUBLE_EQ(truck.MaxTemperatureC(), 5.0);
}

TEST(TrailerTest, ValidatesAndAttaches) {
    EXPECT_THROW(Trailer("", "P", 100.0, 1.0, false), InvalidVehicleException);
    EXPECT_THROW(Trailer("TR", "", 100.0, 1.0, false), InvalidVehicleException);
    EXPECT_THROW(Trailer("TR", "P", 0.0, 1.0, false), InvalidVehicleException);
    EXPECT_THROW(Trailer("TR", "P", 100.0, 0.0, false), InvalidVehicleException);
    Trailer trailer("TR1", "TRL1", 5000.0, 20.0, true);
    EXPECT_FALSE(trailer.IsAttached());
    EXPECT_THROW(trailer.Detach(), InvalidVehicleException);
    EXPECT_THROW(trailer.AttachTo(""), InvalidVehicleException);
    trailer.AttachTo("T1");
    EXPECT_TRUE(trailer.IsAttached());
    EXPECT_EQ(trailer.AttachedVehicleId(), "T1");
    EXPECT_THROW(trailer.AttachTo("T2"), InvalidVehicleException);
    trailer.Detach();
    EXPECT_FALSE(trailer.IsAttached());
    EXPECT_TRUE(trailer.CanCarry(5000.0, 20.0));
    EXPECT_FALSE(trailer.CanCarry(5001.0, 20.0));
    EXPECT_FALSE(trailer.CanCarry(5000.0, 21.0));
    EXPECT_FALSE(trailer.CanCarry(-1.0, 1.0));
    EXPECT_FALSE(trailer.CanCarry(1.0, -1.0));
    EXPECT_TRUE(trailer.IsRefrigerated());
    EXPECT_EQ(trailer.Id(), "TR1");
    EXPECT_EQ(trailer.PlateNumber(), "TRL1");
    EXPECT_DOUBLE_EQ(trailer.CapacityKg(), 5000.0);
    EXPECT_DOUBLE_EQ(trailer.VolumeM3(), 20.0);
}

TEST(VehicleFleetTest, AddsFindsAndRemovesVehicles) {
    VehicleFleet fleet;
    EXPECT_EQ(fleet.VehicleCount(), 0U);
    EXPECT_DOUBLE_EQ(fleet.AvailabilityPercent(), 0.0);
    EXPECT_DOUBLE_EQ(fleet.TotalCapacityKg(), 0.0);
    EXPECT_THROW(fleet.AddVehicle(nullptr), InvalidVehicleException);
    fleet.AddVehicle(MakeTruck());
    fleet.AddVehicle(std::make_unique<Van>("V1", "AC1", 500.0, 5.0, 0.0, 0.0, false, 20));
    EXPECT_EQ(fleet.VehicleCount(), 2U);
    EXPECT_DOUBLE_EQ(fleet.TotalCapacityKg(), 1500.0);
    EXPECT_NE(fleet.FindVehicle("T1"), nullptr);
    EXPECT_EQ(fleet.FindVehicle("missing"), nullptr);
    const VehicleFleet& constFleet = fleet;
    EXPECT_NE(constFleet.FindVehicle("V1"), nullptr);
    EXPECT_EQ(constFleet.FindVehicle("missing"), nullptr);
    EXPECT_EQ(fleet.AvailableVehicles().size(), 2U);
    EXPECT_DOUBLE_EQ(fleet.AvailabilityPercent(), 100.0);
    EXPECT_THROW(fleet.AddVehicle(MakeTruck()), InvalidVehicleException);
    EXPECT_TRUE(fleet.RemoveVehicle("T1"));
    EXPECT_FALSE(fleet.RemoveVehicle("T1"));
    EXPECT_EQ(fleet.VehicleCount(), 1U);
    fleet.AddVehicle(MakeTruck());
    EXPECT_EQ(fleet.VehicleCount(), 2U);
}

TEST(VehicleFleetTest, RecordsMaintenanceAndFuel) {
    VehicleFleet fleet;
    fleet.AddVehicle(MakeTruck());
    const MaintenanceRecord record = MakeMaintenance();
    fleet.ScheduleMaintenance(record);
    EXPECT_EQ(fleet.MaintenanceRecords().size(), 1U);
    EXPECT_DOUBLE_EQ(fleet.AvailabilityPercent(), 0.0);
    EXPECT_THROW(fleet.ScheduleMaintenance(record), VehicleNotAvailableException);
    EXPECT_EQ(fleet.MaintenanceRecords().size(), 1U);
    const MaintenanceRecord unknown("M2", "missing", 1, "oil", 10.0, 100.0, "");
    EXPECT_THROW(fleet.ScheduleMaintenance(unknown), InvalidVehicleException);
    fleet.RecordFuel(MakeFuel());
    EXPECT_EQ(fleet.FuelRecords().size(), 1U);
    const FuelRecord badFuel("missing", 10.0, 2.0, 100.0, 1);
    EXPECT_THROW(fleet.RecordFuel(badFuel), InvalidVehicleException);
    fleet.FindVehicle("T1")->ReturnToService();
    EXPECT_DOUBLE_EQ(fleet.AvailabilityPercent(), 100.0);
}

}  // namespace logistics
