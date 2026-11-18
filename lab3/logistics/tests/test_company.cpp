/**
 * @file test_company.cpp
 * @brief Unit tests for the transport company aggregate root.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "logistics/company.hpp"
#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

Address Depot() {
    return Address("Depot", "Minsk", "220000", GeoPoint(53.9, 27.56));
}

Address Destination() {
    return Address("Port", "Brest", "224000", GeoPoint(52.09, 23.73));
}

Driver MakeDriver() {
    return Driver("D1", "Pavel Ivanov", "+375", "pavel@example.com", "EMP-1", 2020, 1000.0,
                  DrivingLicense("LIC-1", "C", 0, 3650), 5, 4.5);
}

TransportCompany MakeCompany() {
    TransportCompany company("TestCo", 1000.0);
    company.OnboardClient(Client("C1", "Acme", "Olga", 2000.0, 5000.0, 4.5));
    company.AddWarehouse(Warehouse("W1", "Hub", Depot(), 10000.0, 100.0));
    company.RegisterVehicle(
        std::make_unique<Truck>("T1", "AB1", 1000.0, 10.0, 0.0, 100.0, 3, 5000.0));
    company.AddTariff(Tariff("TR1", "general", 1.0, 50.0, 0.1, 1.0));
    company.PlaceOrder(std::make_unique<Order>("O1", "C1", "CGO1", Depot(), Destination(), 0, 100.0,
                                               OrderPriority::Standard));
    company.Orders().Require("O1").Confirm();
    return company;
}

}  // namespace

TEST(TransportCompanyTest, ValidatesConstruction) {
    EXPECT_THROW(TransportCompany("", 0.0), InvalidPaymentException);
    EXPECT_THROW(TransportCompany("Co", -1.0), InvalidPaymentException);
    const TransportCompany company("Co", 0.0);
    EXPECT_EQ(company.Name(), "Co");
    EXPECT_EQ(company.ClientCount(), 0U);
    EXPECT_EQ(company.WarehouseCount(), 0U);
    EXPECT_EQ(company.ShipmentCount(), 0U);
    EXPECT_EQ(company.InvoiceCount(), 0U);
    EXPECT_DOUBLE_EQ(company.Fleet().TotalCapacityKg(), 0.0);
    EXPECT_DOUBLE_EQ(company.Finance().Balance(), 0.0);
}

TEST(TransportCompanyTest, RegistersClients) {
    TransportCompany company = MakeCompany();
    EXPECT_EQ(company.ClientCount(), 1U);
    EXPECT_NE(company.FindClient("C1"), nullptr);
    EXPECT_EQ(company.FindClient("missing"), nullptr);
    EXPECT_EQ(company.RequireClient("C1").CompanyName(), "Acme");
    EXPECT_THROW(company.RequireClient("missing"), InvalidClientException);
    EXPECT_THROW(company.OnboardClient(Client("C1", "Dup", "X", 0.0, 0.0, 4.0)),
                 InvalidClientException);
}

TEST(TransportCompanyTest, RegistersWarehousesAndFleet) {
    TransportCompany company = MakeCompany();
    EXPECT_NE(company.FindWarehouse("W1"), nullptr);
    EXPECT_EQ(company.FindWarehouse("missing"), nullptr);
    EXPECT_THROW(company.AddWarehouse(Warehouse("W1", "Dup", Depot(), 10.0, 1.0)),
                 InvalidCargoException);
    EXPECT_THROW(company.RegisterVehicle(
                     std::make_unique<Truck>("T1", "AB2", 1000.0, 10.0, 0.0, 0.0, 3, 5000.0)),
                 InvalidVehicleException);
    EXPECT_THROW(company.AddTariff(Tariff("TR2", "general", 1.0, 10.0, 0.1, 1.0)),
                 InvalidTariffException);
    EXPECT_EQ(company.WarehouseCount(), 1U);
    EXPECT_EQ(company.Fleet().VehicleCount(), 1U);
    EXPECT_EQ(company.Pricing().TariffCount(), 1U);
}

TEST(TransportCompanyTest, PlacesOrders) {
    TransportCompany company = MakeCompany();
    EXPECT_THROW(company.PlaceOrder(nullptr), InvalidOrderException);
    EXPECT_THROW(company.PlaceOrder(std::make_unique<Order>("O2", "missing", "CGO", Depot(),
                                                            Destination(), 0, 100.0,
                                                            OrderPriority::Standard)),
                 InvalidClientException);
    EXPECT_THROW(company.PlaceOrder(std::make_unique<Order>("O1", "C1", "CGO", Depot(),
                                                            Destination(), 0, 100.0,
                                                            OrderPriority::Standard)),
                 InvalidOrderException);
    EXPECT_EQ(company.Orders().Count(), 1U);
    EXPECT_DOUBLE_EQ(company.Quote("general", 100.0, 100.0, 10.0), 126.0);
    EXPECT_THROW(company.Quote("missing", 1.0, 1.0, 1.0), InvalidTariffException);
}

TEST(TransportCompanyTest, DispatchesConfirmedOrder) {
    TransportCompany company = MakeCompany();
    Driver driver = MakeDriver();
    Vehicle* vehicle = company.Fleet().FindVehicle("T1");
    ASSERT_NE(vehicle, nullptr);
    Order& order = company.Orders().Require("O1");
    Shipment& shipment = company.Dispatch(order, *vehicle, driver, "C", "R1", 1);
    EXPECT_EQ(shipment.Id(), "SHP-1");
    EXPECT_EQ(shipment.OrderId(), "O1");
    EXPECT_EQ(shipment.VehicleId(), "T1");
    EXPECT_EQ(shipment.DriverId(), "D1");
    EXPECT_EQ(shipment.RouteId(), "R1");
    EXPECT_EQ(shipment.Status(), ShipmentStatus::InTransit);
    EXPECT_EQ(order.Status(), OrderStatus::InTransit);
    EXPECT_EQ(vehicle->Status(), VehicleStatus::OnRoute);
    EXPECT_EQ(driver.Status(), DriverStatus::OnTrip);
    EXPECT_EQ(company.ShipmentCount(), 1U);
    EXPECT_EQ(&company.RequireShipment("SHP-1"), &shipment);
}

TEST(TransportCompanyTest, RejectsInvalidDispatch) {
    TransportCompany company = MakeCompany();
    Driver driver = MakeDriver();
    Vehicle* vehicle = company.Fleet().FindVehicle("T1");
    ASSERT_NE(vehicle, nullptr);
    Order& order = company.Orders().Require("O1");
    EXPECT_THROW(company.Dispatch(order, *vehicle, driver, "C", "R1", -1), InvalidShipmentException);
    EXPECT_THROW(company.Dispatch(order, *vehicle, driver, "B", "R1", 1),
                 DriverNotAvailableException);
    vehicle->MarkOnRoute();
    EXPECT_THROW(company.Dispatch(order, *vehicle, driver, "C", "R1", 1),
                 VehicleNotAvailableException);
}

TEST(TransportCompanyTest, RejectsDispatchOfDraftOrder) {
    TransportCompany company = MakeCompany();
    company.PlaceOrder(std::make_unique<Order>("O2", "C1", "CGO2", Depot(), Destination(), 0, 100.0,
                                               OrderPriority::Standard));
    Driver driver = MakeDriver();
    Vehicle* vehicle = company.Fleet().FindVehicle("T1");
    ASSERT_NE(vehicle, nullptr);
    EXPECT_THROW(company.Dispatch(company.Orders().Require("O2"), *vehicle, driver, "C", "R1", 1),
                 InvalidOrderException);
}

TEST(TransportCompanyTest, CompletesDeliveries) {
    TransportCompany company = MakeCompany();
    Driver driver = MakeDriver();
    Vehicle* vehicle = company.Fleet().FindVehicle("T1");
    ASSERT_NE(vehicle, nullptr);
    company.Dispatch(company.Orders().Require("O1"), *vehicle, driver, "C", "R1", 1);
    company.CompleteDelivery("SHP-1", 3);
    EXPECT_TRUE(company.RequireShipment("SHP-1").IsDelivered());
    EXPECT_EQ(company.Orders().Require("O1").Status(), OrderStatus::Delivered);
    EXPECT_EQ(vehicle->Status(), VehicleStatus::Available);
    EXPECT_THROW(company.CompleteDelivery("SHP-1", 4), InvalidShipmentException);
    EXPECT_THROW(company.CompleteDelivery("missing", 4), InvalidShipmentException);
    EXPECT_THROW(company.RequireShipment("missing"), InvalidShipmentException);
}

TEST(TransportCompanyTest, FailsDeliveryWhenOrderDisappeared) {
    TransportCompany company = MakeCompany();
    Driver driver = MakeDriver();
    Vehicle* vehicle = company.Fleet().FindVehicle("T1");
    ASSERT_NE(vehicle, nullptr);
    company.Dispatch(company.Orders().Require("O1"), *vehicle, driver, "C", "R1", 1);
    EXPECT_TRUE(company.Orders().Remove("O1"));
    EXPECT_THROW(company.CompleteDelivery("SHP-1", 2), OrderNotFoundException);
}

TEST(TransportCompanyTest, BillsClients) {
    TransportCompany company = MakeCompany();
    Invoice& invoice = company.BillClient("C1", "O1", 100.0, 0, 10);
    EXPECT_EQ(invoice.Id(), "INV-1");
    EXPECT_DOUBLE_EQ(invoice.Amount(), 100.0);
    EXPECT_DOUBLE_EQ(company.RequireClient("C1").Balance(), 4900.0);
    EXPECT_EQ(company.InvoiceCount(), 1U);
    EXPECT_THROW(company.BillClient("missing", "O1", 10.0, 0, 10), InvalidClientException);
    EXPECT_THROW(company.BillClient("C1", "missing", 10.0, 0, 10), OrderNotFoundException);
    EXPECT_THROW(company.BillClient("C1", "O1", 8000.0, 0, 10), InsufficientFundsException);
    company.OnboardClient(Client("C2", "Other", "Petr", 0.0, 0.0, 4.0));
    company.PlaceOrder(std::make_unique<Order>("O2", "C2", "CGO2", Depot(), Destination(), 0, 100.0,
                                               OrderPriority::Standard));
    EXPECT_THROW(company.BillClient("C1", "O2", 10.0, 0, 10), InvalidClientException);
}

TEST(TransportCompanyTest, ReceivesPayments) {
    TransportCompany company = MakeCompany();
    company.BillClient("C1", "O1", 100.0, 0, 10);
    EXPECT_THROW(company.ReceivePayment("missing", 100.0, PaymentMethod::Card, 1, "REF"),
                 InvalidPaymentException);
    EXPECT_THROW(company.ReceivePayment("INV-1", 99.0, PaymentMethod::Card, 1, "REF"),
                 InvalidPaymentException);
    const Payment payment =
        company.ReceivePayment("INV-1", 100.0, PaymentMethod::Card, 1, "REF");
    EXPECT_EQ(payment.Id(), "PAY-1");
    EXPECT_EQ(company.Payments().size(), 1U);
    EXPECT_DOUBLE_EQ(company.Finance().Revenue(), 100.0);
    EXPECT_DOUBLE_EQ(company.Finance().Balance(), 1100.0);
    EXPECT_TRUE(company.Invoices().front().IsPaid());
    EXPECT_THROW(company.ReceivePayment("INV-1", 100.0, PaymentMethod::Card, 2, "REF"),
                 InvalidPaymentException);
}

TEST(TransportCompanyTest, RecordsExpensesAndTracking) {
    TransportCompany company = MakeCompany();
    company.RecordExpense(100.0);
    EXPECT_DOUBLE_EQ(company.Finance().Balance(), 900.0);
    EXPECT_THROW(company.RecordExpense(1000.0), InsufficientFundsException);
    company.RecordTrackingPoint(TrackPoint("SHP-1", GeoPoint(53.9, 27.56), 1, 10, 60.0));
    EXPECT_EQ(company.Tracking().TrackedShipments(), 1U);
    EXPECT_EQ(company.Tracking().Latest("SHP-1").Day(), 1);
}

TEST(TransportCompanyTest, ReportsFinancialState) {
    TransportCompany company = MakeCompany();
    company.BillClient("C1", "O1", 100.0, 0, 10);
    company.ReceivePayment("INV-1", 100.0, PaymentMethod::Card, 1, "REF");
    const std::string report = company.GenerateReport();
    EXPECT_NE(report.find("company: TestCo"), std::string::npos);
    EXPECT_NE(report.find("clients: 1"), std::string::npos);
    EXPECT_NE(report.find("invoices: 1, paid: 1"), std::string::npos);
    EXPECT_NE(report.find("balance: 1100.00"), std::string::npos);
    EXPECT_EQ(company.Fleet().FindVehicle("T1")->Id(), "T1");
    EXPECT_EQ(company.Finance().TaxAmount(), 20.0);
}

}  // namespace logistics
