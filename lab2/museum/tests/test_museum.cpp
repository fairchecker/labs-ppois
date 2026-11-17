/**
 * @file test_museum.cpp
 * @brief Unit tests for the museum aggregate root and addresses.
 */

#include <gtest/gtest.h>

#include <memory>

#include "museum/address.hpp"
#include "museum/budget.hpp"
#include "museum/curator.hpp"
#include "museum/department.hpp"
#include "museum/exceptions.hpp"
#include "museum/exhibition.hpp"
#include "museum/fossil.hpp"
#include "museum/museum.hpp"
#include "museum/painting.hpp"

namespace museum {
namespace {

Address MakeAddress() { return Address("Main St", "Minsk", "220000", "5"); }

Budget MakeBudget() { return Budget(100000.0, "general"); }

std::shared_ptr<Painting> MakePainting() {
    return std::make_shared<Painting>("p1", "Landscape", "Modern", 80, 5000.0, 3.0, "France",
                                      "Monet", "oil", 100.0, 50.0);
}

std::shared_ptr<Fossil> MakeFossil() {
    return std::make_shared<Fossil>("f1", "Bone", "Jurassic", 60, 2000.0, 5.0, "Land",
                                    "Dinosaur", "Jurassic", 150000.0);
}

std::shared_ptr<Curator> MakeCurator() {
    return std::make_shared<Curator>("c1", "Ada", 40, 1000.0, 2015, "Archaeology", "Egypt");
}

}  // namespace

TEST(AddressTest, FormatsAndValidates) {
    Address address = MakeAddress();
    EXPECT_EQ(address.Format(), "Main St 5, 220000 Minsk");
    EXPECT_TRUE(address.Validate());
    EXPECT_EQ(address.Street(), "Main St");
    EXPECT_EQ(address.City(), "Minsk");
    EXPECT_EQ(address.PostalCode(), "220000");
    EXPECT_EQ(address.BuildingNumber(), "5");
    Address other("Other St", "Minsk", "220001", "7");
    Address anotherCity("Other St", "Grodno", "230000", "7");
    EXPECT_TRUE(address.IsSameCity(other));
    EXPECT_FALSE(address.IsSameCity(anotherCity));
}

TEST(AddressTest, RejectsInvalidData) {
    EXPECT_THROW(Address("", "Minsk", "220000", "5"), InvalidAddressException);
    EXPECT_THROW(Address("Main St", "", "220000", "5"), InvalidAddressException);
    EXPECT_THROW(Address("Main St", "Minsk", "", "5"), InvalidAddressException);
    EXPECT_THROW(Address("Main St", "Minsk", "220000", ""), InvalidAddressException);
    EXPECT_THROW(Address("Main St", "Minsk", "22000", "5"), InvalidAddressException);
    EXPECT_THROW(Address("Main St", "Minsk", "22000A", "5"), InvalidAddressException);
}

TEST(MuseumTest, ManagesDepartments) {
    Museum museum("City Museum", MakeAddress(), MakeBudget());
    const auto department = std::make_shared<Department>("Archaeology", 1, "c1", 5);
    EXPECT_TRUE(museum.AddDepartment(department));
    EXPECT_FALSE(museum.AddDepartment(department));
    EXPECT_EQ(museum.DepartmentCount(), 1U);
    EXPECT_EQ(museum.FindDepartment("Archaeology"), department);
    EXPECT_THROW(museum.FindDepartment("Art"), DepartmentNotFoundException);
    EXPECT_THROW(museum.AddDepartment(nullptr), InvalidDepartmentException);
}

TEST(MuseumTest, ManagesEmployees) {
    Museum museum("City Museum", MakeAddress(), MakeBudget());
    const auto curator = MakeCurator();
    EXPECT_TRUE(museum.Hire(curator));
    EXPECT_FALSE(museum.Hire(curator));
    EXPECT_EQ(museum.EmployeeCount(), 1U);
    EXPECT_EQ(museum.FindEmployee("c1"), curator);
    EXPECT_EQ(museum.FindEmployee("unknown"), nullptr);
    EXPECT_DOUBLE_EQ(museum.AnnualPayroll(), 12000.0);
    EXPECT_THROW(museum.Hire(nullptr), InvalidPersonException);
}

TEST(MuseumTest, ManagesCollectionAndExhibitions) {
    Museum museum("City Museum", MakeAddress(), MakeBudget());
    const auto painting = MakePainting();
    const auto fossil = MakeFossil();
    EXPECT_TRUE(museum.AddExhibit(painting));
    EXPECT_FALSE(museum.AddExhibit(painting));
    EXPECT_TRUE(museum.AddExhibit(fossil));
    EXPECT_EQ(museum.ExhibitCount(), 2U);
    EXPECT_EQ(museum.FindExhibit("p1"), painting);
    EXPECT_EQ(museum.FindExhibit("unknown"), nullptr);
    EXPECT_DOUBLE_EQ(museum.TotalCollectionValue(), 7000.0);
    EXPECT_EQ(museum.ExhibitsByCategory("painting").size(), 1U);
    EXPECT_EQ(museum.ExhibitsByCategory("fossil").size(), 1U);
    EXPECT_EQ(museum.ExhibitsByCategory("weapon").size(), 0U);
    EXPECT_THROW(museum.AddExhibit(nullptr), InvalidExhibitException);

    museum.OpenExhibition(Exhibition("ex1", "Ancient Worlds", 1, 30, 50.0));
    EXPECT_EQ(museum.ExhibitionCount(), 1U);
}

TEST(MuseumTest, ExposesIdentityAndBudget) {
    Museum museum("City Museum", MakeAddress(), MakeBudget());
    EXPECT_EQ(museum.Name(), "City Museum");
    EXPECT_EQ(museum.Location().City(), "Minsk");
    EXPECT_DOUBLE_EQ(museum.BudgetRef().Total(), 100000.0);
    museum.BudgetRef().Spend(500.0);
    EXPECT_DOUBLE_EQ(museum.BudgetRef().Remaining(), 99500.0);
}

TEST(MuseumTest, RejectsEmptyName) {
    EXPECT_THROW(Museum("", MakeAddress(), MakeBudget()), MuseumException);
}

}  // namespace museum
