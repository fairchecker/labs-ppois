/**
 * @file test_people.cpp
 * @brief Unit tests for personnel, drivers and role behaviour.
 */

#include <gtest/gtest.h>

#include <string>

#include "logistics/exceptions.hpp"
#include "logistics/people.hpp"

namespace logistics {
namespace {

DrivingLicense License() { return DrivingLicense("LIC-1", "C", 0, 3650); }

Driver MakeDriver() {
    return Driver("D1", "Pavel Ivanov", "+375", "pavel@example.com", "EMP-1", 2020, 1000.0,
                  License(), 5, 4.0);
}

Employee MakeEmployee() {
    return Employee("E1", "Anna Petrova", "+375", "anna@example.com", "EMP-2", 2020, 1000.0,
                    "finance");
}

}  // namespace

TEST(PersonTest, ValidatesIdentityAndContact) {
    EXPECT_THROW(Person("", "Name", "1", "a@b"), InvalidClientException);
    EXPECT_THROW(Person("P", "", "1", "a@b"), InvalidClientException);
    EXPECT_THROW(Person("P", "Name", "1", "invalid"), InvalidClientException);
    const Person person("P1", "Ivan Petrov", "+375", "ivan@example.com");
    EXPECT_EQ(person.Describe(), "Ivan Petrov (P1), +375, ivan@example.com");
    EXPECT_TRUE(person.HasContactInfo());
    EXPECT_EQ(person.Id(), "P1");
    EXPECT_EQ(person.FullName(), "Ivan Petrov");
    EXPECT_EQ(person.Phone(), "+375");
    EXPECT_EQ(person.Email(), "ivan@example.com");
}

TEST(PersonTest, UpdatesContactDetails) {
    Person person("P1", "Ivan Petrov", "", "");
    EXPECT_FALSE(person.HasContactInfo());
    person.UpdateContact("+376", "new@example.com");
    EXPECT_TRUE(person.HasContactInfo());
    EXPECT_EQ(person.Phone(), "+376");
    EXPECT_THROW(person.UpdateContact("+376", ""), InvalidClientException);
    EXPECT_THROW(person.UpdateContact("+376", "bad"), InvalidClientException);
}

TEST(EmployeeTest, ValidatesPayrollRecord) {
    EXPECT_THROW(Employee("E", "N", "", "", "", 2020, 100.0, "dep"), InvalidClientException);
    EXPECT_THROW(Employee("E", "N", "", "", "E1", 2020, 100.0, ""), InvalidClientException);
    EXPECT_THROW(Employee("E", "N", "", "", "E1", 1800, 100.0, "dep"), InvalidClientException);
    EXPECT_THROW(Employee("E", "N", "", "", "E1", 2101, 100.0, "dep"), InvalidClientException);
    EXPECT_THROW(Employee("E", "N", "", "", "E1", 2020, -1.0, "dep"), InvalidClientException);
    const Employee employee = MakeEmployee();
    EXPECT_EQ(employee.EmployeeId(), "EMP-2");
    EXPECT_EQ(employee.HiredYear(), 2020);
    EXPECT_EQ(employee.Department(), "finance");
    EXPECT_DOUBLE_EQ(employee.MonthlySalary(), 1000.0);
    EXPECT_DOUBLE_EQ(employee.AnnualSalary(), 12000.0);
    EXPECT_EQ(employee.Role(), "employee");
}

TEST(EmployeeTest, PromotesWithinRange) {
    Employee employee = MakeEmployee();
    employee.Promote(10.0);
    EXPECT_DOUBLE_EQ(employee.MonthlySalary(), 1100.0);
    EXPECT_THROW(employee.Promote(0.0), InvalidClientException);
    EXPECT_THROW(employee.Promote(50.1), InvalidClientException);
}

TEST(EmployeeTest, ChecksSeniorityBoundaries) {
    const Employee employee = MakeEmployee();
    EXPECT_FALSE(employee.IsSenior(2019, 0));
    EXPECT_FALSE(employee.IsSenior(2024, -1));
    EXPECT_TRUE(employee.IsSenior(2020, 0));
    EXPECT_TRUE(employee.IsSenior(2024, 4));
    EXPECT_FALSE(employee.IsSenior(2024, 5));
}

TEST(DriverTest, ValidatesExperienceAndRating) {
    EXPECT_THROW(Driver("D", "N", "", "", "E", 2020, 100.0, License(), -1, 4.0),
                 InvalidClientException);
    EXPECT_THROW(Driver("D", "N", "", "", "E", 2020, 100.0, License(), 61, 4.0),
                 InvalidClientException);
    EXPECT_THROW(Driver("D", "N", "", "", "E", 2020, 100.0, License(), 1, -0.1),
                 InvalidClientException);
    EXPECT_THROW(Driver("D", "N", "", "", "E", 2020, 100.0, License(), 1, 5.1),
                 InvalidClientException);
    const Driver driver = MakeDriver();
    EXPECT_EQ(driver.ExperienceYears(), 5);
    EXPECT_DOUBLE_EQ(driver.Rating(), 4.0);
    EXPECT_EQ(driver.Status(), DriverStatus::Available);
    EXPECT_EQ(driver.License().Category(), "C");
    EXPECT_EQ(driver.Role(), "driver");
}

TEST(DriverTest, ManagesTripStatus) {
    Driver driver = MakeDriver();
    EXPECT_TRUE(driver.CanDrive("C"));
    EXPECT_FALSE(driver.CanDrive("B"));
    driver.BeginTrip();
    EXPECT_EQ(driver.Status(), DriverStatus::OnTrip);
    EXPECT_FALSE(driver.CanDrive("C"));
    EXPECT_THROW(driver.BeginTrip(), DriverNotAvailableException);
    driver.EndTrip();
    EXPECT_EQ(driver.Status(), DriverStatus::Available);
    EXPECT_THROW(driver.EndTrip(), DriverNotAvailableException);
    driver.UpdateRating(5.0);
    EXPECT_DOUBLE_EQ(driver.Rating(), 5.0);
    EXPECT_THROW(driver.UpdateRating(-1.0), InvalidClientException);
    EXPECT_THROW(driver.UpdateRating(5.1), InvalidClientException);
}

TEST(DispatcherTest, ManagesShipmentWorkload) {
    EXPECT_THROW(Dispatcher("D", "N", "", "", "E", 2020, 100.0, ""), InvalidClientException);
    Dispatcher dispatcher("D1", "N", "", "", "E1", 2020, 100.0, "morning");
    EXPECT_EQ(dispatcher.Shift(), "morning");
    EXPECT_EQ(dispatcher.Workload(), 0U);
    dispatcher.AssignShipment("S1");
    dispatcher.AssignShipment("S2");
    EXPECT_THROW(dispatcher.AssignShipment("S1"), InvalidShipmentException);
    EXPECT_TRUE(dispatcher.IsOverloaded(1));
    EXPECT_FALSE(dispatcher.IsOverloaded(2));
    EXPECT_FALSE(dispatcher.IsOverloaded(-1));
    EXPECT_TRUE(dispatcher.ReleaseShipment("S1"));
    EXPECT_FALSE(dispatcher.ReleaseShipment("S1"));
    EXPECT_EQ(dispatcher.AssignedShipmentIds().size(), 1U);
    EXPECT_EQ(dispatcher.Role(), "dispatcher");
}

TEST(LogisticsManagerTest, ApprovesRoutesWithinBudget) {
    EXPECT_THROW(LogisticsManager("D", "N", "", "", "E", 2020, 100.0, "", 10.0),
                 InvalidClientException);
    EXPECT_THROW(LogisticsManager("D", "N", "", "", "E", 2020, 100.0, "west", -1.0),
                 InvalidClientException);
    LogisticsManager manager("D1", "N", "", "", "E1", 2020, 100.0, "west", 100.0);
    EXPECT_TRUE(manager.IsWithinBudget(100.0));
    EXPECT_FALSE(manager.IsWithinBudget(0.0));
    EXPECT_FALSE(manager.IsWithinBudget(101.0));
    EXPECT_THROW(manager.ApproveRoute("", 10.0), InvalidRouteException);
    EXPECT_THROW(manager.ApproveRoute("R1", 0.0), InvalidRouteException);
    EXPECT_THROW(manager.ApproveRoute("R1", 101.0), InvalidRouteException);
    manager.ApproveRoute("R1", 50.0);
    EXPECT_THROW(manager.ApproveRoute("R1", 10.0), InvalidRouteException);
    EXPECT_EQ(manager.ApprovedCount(), 1U);
    EXPECT_EQ(manager.Region(), "west");
    EXPECT_DOUBLE_EQ(manager.BudgetLimit(), 100.0);
    EXPECT_EQ(manager.Role(), "logistics manager");
}

TEST(WarehouseManagerTest, TracksOperationsAndVolume) {
    EXPECT_THROW(WarehouseManager("D", "N", "", "", "E", 2020, 100.0, "", "day"),
                 InvalidClientException);
    EXPECT_THROW(WarehouseManager("D", "N", "", "", "E", 2020, 100.0, "W1", ""),
                 InvalidClientException);
    WarehouseManager manager("D1", "N", "", "", "E1", 2020, 100.0, "W1", "day");
    EXPECT_EQ(manager.ProcessedOperations(), 0);
    manager.RegisterOperation();
    EXPECT_EQ(manager.ProcessedOperations(), 1);
    EXPECT_TRUE(manager.CanHandle(500.0));
    EXPECT_FALSE(manager.CanHandle(0.0));
    EXPECT_FALSE(manager.CanHandle(500.1));
    EXPECT_EQ(manager.WarehouseId(), "W1");
    EXPECT_EQ(manager.Shift(), "day");
    EXPECT_EQ(manager.Role(), "warehouse manager");
}

TEST(MechanicTest, RepairsSpecializedAndUniversalVehicles) {
    EXPECT_THROW(Mechanic("D", "N", "", "", "E", 2020, 100.0, "", 1), InvalidClientException);
    EXPECT_THROW(Mechanic("D", "N", "", "", "E", 2020, 100.0, "truck", 0), InvalidClientException);
    EXPECT_THROW(Mechanic("D", "N", "", "", "E", 2020, 100.0, "truck", 4), InvalidClientException);
    Mechanic specialist("D1", "N", "", "", "E1", 2020, 100.0, "truck", 2);
    Mechanic universal("D2", "N", "", "", "E2", 2020, 100.0, "truck", 3);
    EXPECT_TRUE(specialist.CanRepair("truck"));
    EXPECT_FALSE(specialist.CanRepair("van"));
    EXPECT_TRUE(universal.CanRepair("van"));
    specialist.CompleteRepair();
    EXPECT_EQ(specialist.CompletedRepairs(), 1);
    EXPECT_EQ(specialist.Specialization(), "truck");
    EXPECT_EQ(specialist.CertificationLevel(), 2);
    EXPECT_EQ(specialist.Role(), "mechanic");
}

TEST(HRManagerTest, HiresAndAllocatesTraining) {
    EXPECT_THROW(HRManager("D", "N", "", "", "E", 2020, 100.0, -1.0, 4.0),
                 InvalidClientException);
    EXPECT_THROW(HRManager("D", "N", "", "", "E", 2020, 100.0, 100.0, 5.1),
                 InvalidClientException);
    HRManager manager("D1", "N", "", "", "E1", 2020, 100.0, 1000.0, 4.0);
    const Driver driver = MakeDriver();
    EXPECT_TRUE(manager.CanHire(driver));
    EXPECT_DOUBLE_EQ(manager.TrainingBudget(), 1000.0);
    EXPECT_DOUBLE_EQ(manager.MinRating(), 4.0);
    EXPECT_THROW(manager.AllocateTraining(0.0), InvalidClientException);
    EXPECT_THROW(manager.AllocateTraining(1001.0), InsufficientFundsException);
    manager.AllocateTraining(400.0);
    EXPECT_DOUBLE_EQ(manager.TrainingBudget(), 600.0);
    EXPECT_EQ(manager.Role(), "hr manager");
}

TEST(AccountantTest, AppliesTaxRate) {
    EXPECT_THROW(Accountant("D", "N", "", "", "E", 2020, 100.0, -1.0), InvalidClientException);
    EXPECT_THROW(Accountant("D", "N", "", "", "E", 2020, 100.0, 101.0), InvalidClientException);
    Accountant accountant("D1", "N", "", "", "E1", 2020, 100.0, 20.0);
    EXPECT_DOUBLE_EQ(accountant.NetAmount(100.0), 80.0);
    EXPECT_THROW(accountant.NetAmount(-1.0), InvalidClientException);
    accountant.RegisterInvoice();
    EXPECT_EQ(accountant.ProcessedInvoices(), 1);
    EXPECT_DOUBLE_EQ(accountant.TaxRatePercent(), 20.0);
    EXPECT_EQ(accountant.Role(), "accountant");
}

}  // namespace logistics
