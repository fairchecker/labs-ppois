/**
 * @file test_cargo.cpp
 * @brief Unit tests for the cargo hierarchy, packaging and manifests.
 */

#include <gtest/gtest.h>

#include <string>

#include "logistics/cargo.hpp"
#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

Cargo MakeCargo() { return Cargo("C1", "Pallets", 100.0, 2.0, 500.0, "CL1"); }

FragileCargo MakeFragile(int grade) {
    return FragileCargo("C2", "Glass", 30.0, 1.0, 200.0, "CL1", 40.0, grade);
}

}  // namespace

TEST(CargoTest, RejectsInvalidRecord) {
    EXPECT_THROW(Cargo("", "Pallets", 100.0, 2.0, 500.0, "CL1"), InvalidCargoException);
    EXPECT_THROW(Cargo("C1", "", 100.0, 2.0, 500.0, "CL1"), InvalidCargoException);
    EXPECT_THROW(Cargo("C1", "Pallets", 0.0, 2.0, 500.0, "CL1"), InvalidCargoException);
    EXPECT_THROW(Cargo("C1", "Pallets", 100.0, 0.0, 500.0, "CL1"), InvalidCargoException);
    EXPECT_THROW(Cargo("C1", "Pallets", 100.0, 2.0, -1.0, "CL1"), InvalidCargoException);
    EXPECT_THROW(Cargo("C1", "Pallets", 100.0, 2.0, 500.0, ""), InvalidCargoException);
}

TEST(CargoTest, ComputesMetrics) {
    const Cargo cargo = MakeCargo();
    EXPECT_DOUBLE_EQ(cargo.DensityKgPerM3(), 50.0);
    EXPECT_DOUBLE_EQ(cargo.ValuePerKg(), 5.0);
    EXPECT_TRUE(cargo.IsOverweight(90.0));
    EXPECT_FALSE(cargo.IsOverweight(100.0));
    EXPECT_EQ(cargo.Category(), "general");
    EXPECT_EQ(cargo.Describe(), "general: Pallets, 100.000000 kg");
    EXPECT_EQ(cargo.OwnerId(), "CL1");
    EXPECT_EQ(cargo.Description(), "Pallets");
}

TEST(FragileCargoTest, ValidatesPackaging) {
    EXPECT_THROW(FragileCargo("C2", "Glass", 30.0, 1.0, 200.0, "CL1", 0.0, 3),
                 InvalidCargoException);
    EXPECT_THROW(FragileCargo("C2", "Glass", 30.0, 1.0, 200.0, "CL1", 20.0, 0),
                 InvalidCargoException);
    EXPECT_THROW(FragileCargo("C2", "Glass", 30.0, 1.0, 200.0, "CL1", 20.0, 6),
                 InvalidCargoException);
}

TEST(FragileCargoTest, GradesHandling) {
    const FragileCargo secure = MakeFragile(3);
    const FragileCargo weak = MakeFragile(2);
    EXPECT_TRUE(secure.IsSecurelyPacked());
    EXPECT_FALSE(weak.IsSecurelyPacked());
    EXPECT_EQ(secure.Category(), "fragile");
    EXPECT_EQ(secure.HandlingInstructions(), "fragile: handle with care, secure packaging required");
    EXPECT_EQ(weak.HandlingInstructions(),
              "fragile: handle with care, reinforced packaging required");
    EXPECT_DOUBLE_EQ(secure.MaxDropHeightCm(), 40.0);
    EXPECT_EQ(secure.PackingGrade(), 3);
}

TEST(PerishableCargoTest, RejectsInvalidRangeOrShelfLife) {
    EXPECT_THROW(PerishableCargo("C3", "Milk", 20.0, 1.0, 100.0, "CL1", 5.0, 5.0, 24),
                 InvalidCargoException);
    EXPECT_THROW(PerishableCargo("C3", "Milk", 20.0, 1.0, 100.0, "CL1", 2.0, 5.0, 0),
                 InvalidCargoException);
}

TEST(PerishableCargoTest, TracksTemperatureAndShelfLife) {
    const PerishableCargo cargo("C3", "Milk", 20.0, 1.0, 100.0, "CL1", 2.0, 6.0, 48);
    EXPECT_TRUE(cargo.IsTemperatureCompatible(2.0));
    EXPECT_TRUE(cargo.IsTemperatureCompatible(6.0));
    EXPECT_FALSE(cargo.IsTemperatureCompatible(1.9));
    EXPECT_FALSE(cargo.IsTemperatureCompatible(6.1));
    EXPECT_FALSE(cargo.IsExpired(47));
    EXPECT_TRUE(cargo.IsExpired(48));
    EXPECT_EQ(cargo.RemainingShelfLifeHours(20), 28);
    EXPECT_EQ(cargo.RemainingShelfLifeHours(100), 0);
    EXPECT_EQ(cargo.Category(), "perishable");
    EXPECT_DOUBLE_EQ(cargo.MinTemperatureC(), 2.0);
    EXPECT_DOUBLE_EQ(cargo.MaxTemperatureC(), 6.0);
    EXPECT_EQ(cargo.ShelfLifeHours(), 48);
}

TEST(HazardousCargoTest, RejectsInvalidHazardClass) {
    EXPECT_THROW(HazardousCargo("C4", "Fuel", 50.0, 1.0, 500.0, "CL1", 0, "UN1203", true),
                 InvalidCargoException);
    EXPECT_THROW(HazardousCargo("C4", "Fuel", 50.0, 1.0, 500.0, "CL1", 10, "UN1203", true),
                 InvalidCargoException);
    EXPECT_THROW(HazardousCargo("C4", "Fuel", 50.0, 1.0, 500.0, "CL1", 3, "", true),
                 InvalidCargoException);
}

TEST(HazardousCargoTest, ChecksCompatibility) {
    const HazardousCargo fuel("C4", "Fuel", 50.0, 1.0, 500.0, "CL1", 3, "UN1203", true);
    const HazardousCargo fuelOther("C5", "Solvent", 40.0, 1.0, 400.0, "CL1", 4, "UN1263", false);
    const HazardousCargo explosive("C6", "Dynamite", 20.0, 1.0, 900.0, "CL1", 1, "UN0081", true);
    const HazardousCargo radioactive("C7", "Isotope", 10.0, 1.0, 900.0, "CL1", 7, "UN2912", true);
    EXPECT_TRUE(fuel.IsCompatibleWith(fuel));
    EXPECT_TRUE(fuel.IsCompatibleWith(fuelOther));
    EXPECT_FALSE(fuel.IsCompatibleWith(explosive));
    EXPECT_FALSE(fuel.IsCompatibleWith(radioactive));
    EXPECT_EQ(fuel.Category(), "hazardous");
    EXPECT_EQ(fuel.UnNumber(), "UN1203");
    EXPECT_TRUE(fuel.RequiresEscort());
    EXPECT_FALSE(fuelOther.RequiresEscort());
}

TEST(OversizedCargoTest, ValidatesDimensions) {
    EXPECT_THROW(OversizedCargo("C8", "Turbine", 500.0, 9.0, 5000.0, "CL1", 0.0, 100.0, 100.0),
                 InvalidCargoException);
    EXPECT_THROW(OversizedCargo("C8", "Turbine", 500.0, 9.0, 5000.0, "CL1", 100.0, 0.0, 100.0),
                 InvalidCargoException);
    EXPECT_THROW(OversizedCargo("C8", "Turbine", 500.0, 9.0, 5000.0, "CL1", 100.0, 100.0, 0.0),
                 InvalidCargoException);
}

TEST(OversizedCargoTest, FindsLongestSide) {
    const OversizedCargo cargo("C8", "Turbine", 500.0, 9.0, 5000.0, "CL1", 100.0, 300.0, 120.0);
    EXPECT_DOUBLE_EQ(cargo.LongestSideCm(), 300.0);
    EXPECT_TRUE(cargo.NeedsPermit());
    EXPECT_EQ(cargo.Category(), "oversized");
    const OversizedCargo compact("C9", "Beam", 200.0, 3.0, 1000.0, "CL1", 100.0, 100.0, 100.0);
    EXPECT_FALSE(compact.NeedsPermit());
}

TEST(PackagingTest, ValidatesAndCharges) {
    EXPECT_THROW(Packaging(PackagingType::Box, -1.0, 10), InvalidCargoException);
    EXPECT_THROW(Packaging(PackagingType::Box, 1.0, -1), InvalidCargoException);
    Packaging packaging(PackagingType::Crate, 2.5, 4);
    EXPECT_DOUBLE_EQ(packaging.TotalCost(), 10.0);
    EXPECT_THROW(packaging.AddUnits(0), InvalidCargoException);
    packaging.AddUnits(2);
    EXPECT_EQ(packaging.Units(), 6);
    EXPECT_EQ(packaging.CostPerUnit(), 2.5);
    EXPECT_TRUE(packaging.IsReusable());
    EXPECT_EQ(packaging.TypeName(), "crate");
}

TEST(PackagingTest, ReportsTypeNames) {
    const Packaging box(PackagingType::Box, 1.0, 1);
    const Packaging pallet(PackagingType::Pallet, 1.0, 1);
    const Packaging bag(PackagingType::Bag, 1.0, 1);
    EXPECT_EQ(box.TypeName(), "box");
    EXPECT_EQ(pallet.TypeName(), "pallet");
    EXPECT_EQ(bag.TypeName(), "bag");
    EXPECT_FALSE(box.IsReusable());
    EXPECT_TRUE(pallet.IsReusable());
}

TEST(CargoManifestTest, AddsRemovesAndAverages) {
    const Cargo first = MakeCargo();
    const Cargo second("C2", "Steel", 400.0, 4.0, 2000.0, "CL1");
    CargoManifest manifest;
    EXPECT_TRUE(manifest.IsEmpty());
    EXPECT_DOUBLE_EQ(manifest.MaxWeightKg(), kDefaultManifestWeightKg);
    EXPECT_DOUBLE_EQ(manifest.AverageValuePerKg(), 0.0);
    manifest.AddCargo(first);
    manifest.AddCargo(second);
    EXPECT_EQ(manifest.Size(), 2U);
    EXPECT_DOUBLE_EQ(manifest.TotalWeightKg(), 500.0);
    EXPECT_DOUBLE_EQ(manifest.TotalValue(), 2500.0);
    EXPECT_DOUBLE_EQ(manifest.AverageValuePerKg(), 5.0);
    EXPECT_THROW(manifest.AddCargo(first), InvalidCargoException);
    EXPECT_TRUE(manifest.RemoveCargo(first));
    EXPECT_FALSE(manifest.RemoveCargo(first));
    EXPECT_FALSE(manifest.IsEmpty());
    EXPECT_EQ(manifest.CargoIds().size(), 1U);
}

TEST(CargoManifestTest, EnforcesWeightLimit) {
    EXPECT_THROW(CargoManifest(0.0), InvalidCargoException);
    CargoManifest manifest(150.0);
    manifest.AddCargo(MakeCargo());
    EXPECT_THROW(manifest.AddCargo(Cargo("C3", "Parts", 51.0, 1.0, 100.0, "CL1")),
                 CargoOverweightException);
    EXPECT_EQ(manifest.Size(), 1U);
}

}  // namespace logistics
