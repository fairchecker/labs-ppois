/**
 * @file test_orders.cpp
 * @brief Unit tests for orders, shipments, delivery slots and the registry.
 */

#include <gtest/gtest.h>

#include <memory>

#include "logistics/exceptions.hpp"
#include "logistics/orders.hpp"

namespace logistics {
namespace {

Address Depot() {
    return Address("Depot", "Minsk", "220000", GeoPoint(53.9, 27.56));
}

Address Destination() {
    return Address("Port", "Brest", "224000", GeoPoint(52.09, 23.73));
}

std::unique_ptr<Order> MakeOrder(const std::string& id = "O1") {
    return std::make_unique<Order>(id, "C1", "CGO1", Depot(), Destination(), 0, 100.0,
                                   OrderPriority::Standard);
}

std::unique_ptr<Shipment> MakeShipment() {
    return std::make_unique<Shipment>("S1", "O1", "T1", "D1", "R1");
}

}  // namespace

TEST(OrderTest, ValidatesOrderRecord) {
    const Address origin = Depot();
    const Address destination = Destination();
    EXPECT_THROW(Order("", "C1", "CGO", origin, destination, 0, 10.0, OrderPriority::Standard),
                 InvalidOrderException);
    EXPECT_THROW(Order("O1", "", "CGO", origin, destination, 0, 10.0, OrderPriority::Standard),
                 InvalidOrderException);
    EXPECT_THROW(Order("O1", "C1", "", origin, destination, 0, 10.0, OrderPriority::Standard),
                 InvalidOrderException);
    EXPECT_THROW(Order("O1", "C1", "CGO", origin, destination, -1, 10.0, OrderPriority::Standard),
                 InvalidOrderException);
    EXPECT_THROW(Order("O1", "C1", "CGO", origin, destination, 0, -1.0, OrderPriority::Standard),
                 InvalidOrderException);
    EXPECT_THROW(
        Order("O1", "C1", "CGO", origin, destination, 0, kMaxOrderPrice + 1.0, OrderPriority::Standard),
        InvalidOrderException);
}

TEST(OrderTest, EnforcesLifecycle) {
    const std::unique_ptr<Order> order = MakeOrder();
    EXPECT_EQ(order->Status(), OrderStatus::Draft);
    EXPECT_FALSE(order->IsActive());
    EXPECT_THROW(order->StartTransit(), InvalidOrderException);
    EXPECT_THROW(order->Complete(), InvalidOrderException);
    order->Confirm();
    EXPECT_EQ(order->Status(), OrderStatus::Confirmed);
    EXPECT_TRUE(order->IsActive());
    EXPECT_THROW(order->Confirm(), InvalidOrderException);
    order->StartTransit();
    EXPECT_EQ(order->Status(), OrderStatus::InTransit);
    EXPECT_THROW(order->StartTransit(), InvalidOrderException);
    order->Complete();
    EXPECT_EQ(order->Status(), OrderStatus::Delivered);
    EXPECT_FALSE(order->IsActive());
    EXPECT_THROW(order->Complete(), InvalidOrderException);
    EXPECT_THROW(order->Cancel(), InvalidOrderException);
}

TEST(OrderTest, CancelsDraftAndConfirmedOrders) {
    const std::unique_ptr<Order> draft = MakeOrder();
    draft->Cancel();
    EXPECT_EQ(draft->Status(), OrderStatus::Cancelled);
    EXPECT_THROW(draft->Cancel(), InvalidOrderException);
    const std::unique_ptr<Order> confirmed = MakeOrder("O2");
    confirmed->Confirm();
    confirmed->Cancel();
    EXPECT_EQ(confirmed->Status(), OrderStatus::Cancelled);
}

TEST(OrderTest, ExposesOrderDetails) {
    const std::unique_ptr<Order> order = MakeOrder();
    EXPECT_EQ(order->Id(), "O1");
    EXPECT_EQ(order->ClientId(), "C1");
    EXPECT_EQ(order->CargoId(), "CGO1");
    EXPECT_EQ(order->CreatedDay(), 0);
    EXPECT_DOUBLE_EQ(order->Price(), 100.0);
    EXPECT_EQ(order->Priority(), OrderPriority::Standard);
    EXPECT_EQ(order->Origin().City(), "Minsk");
    EXPECT_EQ(order->Destination().City(), "Brest");
    EXPECT_EQ(order->TypeName(), "standard");
    EXPECT_EQ(order->Describe(), "standard order O1 for C1, price 100.000000");
}

TEST(ExpressOrderTest, ValidatesAndAppliesSurcharge) {
    EXPECT_THROW(ExpressOrder("O1", "C1", "CGO", Depot(), Destination(), 5, 100.0, 4, 20.0),
                 InvalidOrderException);
    EXPECT_THROW(ExpressOrder("O1", "C1", "CGO", Depot(), Destination(), 0, 100.0, 5, 0.0),
                 InvalidOrderException);
    ExpressOrder order("O1", "C1", "CGO", Depot(), Destination(), 0, 100.0, 5, 20.0);
    EXPECT_DOUBLE_EQ(order.ApplySurcharge(100.0), 120.0);
    EXPECT_THROW(order.ApplySurcharge(-1.0), InvalidOrderException);
    EXPECT_FALSE(order.IsLate(5));
    EXPECT_TRUE(order.IsLate(6));
    order.Confirm();
    order.StartTransit();
    order.Complete();
    EXPECT_FALSE(order.IsLate(6));
    EXPECT_EQ(order.TypeName(), "express");
    EXPECT_EQ(order.DeadlineDay(), 5);
    EXPECT_DOUBLE_EQ(order.SurchargePercent(), 20.0);
    EXPECT_EQ(order.Priority(), OrderPriority::Express);
}

TEST(BulkOrderTest, ComputesHandlingCost) {
    EXPECT_THROW(BulkOrder("O1", "C1", "CGO", Depot(), Destination(), 0, 100.0, true, 0.0),
                 InvalidOrderException);
    BulkOrder order("O1", "C1", "CGO", Depot(), Destination(), 0, 100.0, true, 2.5);
    EXPECT_TRUE(order.RequiresCrane());
    EXPECT_DOUBLE_EQ(order.LoadingHours(), 2.5);
    EXPECT_DOUBLE_EQ(order.LoadingCost(10.0), 25.0);
    EXPECT_THROW(order.LoadingCost(0.0), InvalidOrderException);
    EXPECT_DOUBLE_EQ(order.TotalHandlingCost(10.0, 5.0), 30.0);
    EXPECT_THROW(order.TotalHandlingCost(10.0, -1.0), InvalidOrderException);
    BulkOrder manual("O2", "C1", "CGO", Depot(), Destination(), 0, 100.0, false, 1.0);
    EXPECT_DOUBLE_EQ(manual.TotalHandlingCost(10.0, 5.0), 10.0);
    EXPECT_EQ(order.TypeName(), "bulk");
}

TEST(ShipmentTest, ValidatesTransportAssignment) {
    EXPECT_THROW(Shipment("", "O1", "T1", "D1", "R1"), InvalidShipmentException);
    EXPECT_THROW(Shipment("S1", "", "T1", "D1", "R1"), InvalidShipmentException);
    EXPECT_THROW(Shipment("S1", "O1", "", "D1", "R1"), InvalidShipmentException);
    EXPECT_THROW(Shipment("S1", "O1", "T1", "", "R1"), InvalidShipmentException);
    EXPECT_THROW(Shipment("S1", "O1", "T1", "D1", ""), InvalidShipmentException);
    const std::unique_ptr<Shipment> shipment = MakeShipment();
    EXPECT_EQ(shipment->Id(), "S1");
    EXPECT_EQ(shipment->OrderId(), "O1");
    EXPECT_EQ(shipment->VehicleId(), "T1");
    EXPECT_EQ(shipment->DriverId(), "D1");
    EXPECT_EQ(shipment->RouteId(), "R1");
    EXPECT_EQ(shipment->Status(), ShipmentStatus::Planned);
    EXPECT_EQ(shipment->TransitDays(5), 0);
}

TEST(ShipmentTest, EnforcesTransitLifecycle) {
    const std::unique_ptr<Shipment> shipment = MakeShipment();
    EXPECT_THROW(shipment->Arrive(1), InvalidShipmentException);
    EXPECT_THROW(shipment->MarkDelayed(), InvalidShipmentException);
    EXPECT_THROW(shipment->Depart(-1), InvalidShipmentException);
    shipment->Depart(2);
    EXPECT_THROW(shipment->Depart(3), InvalidShipmentException);
    EXPECT_EQ(shipment->DepartureDay(), 2);
    EXPECT_EQ(shipment->TransitDays(5), 3);
    shipment->MarkDelayed();
    EXPECT_EQ(shipment->Status(), ShipmentStatus::Delayed);
    EXPECT_THROW(shipment->MarkDelayed(), InvalidShipmentException);
    EXPECT_THROW(shipment->Arrive(1), InvalidShipmentException);
    shipment->Arrive(6);
    EXPECT_TRUE(shipment->IsDelivered());
    EXPECT_EQ(shipment->ArrivalDay(), 6);
    EXPECT_EQ(shipment->TransitDays(10), 4);
    EXPECT_THROW(shipment->Arrive(7), InvalidShipmentException);
    EXPECT_EQ(shipment->Describe(), "shipment S1 of order O1 by vehicle T1");
}

TEST(ShipmentTest, ReassignsVehicleBeforeDeparture) {
    const std::unique_ptr<Shipment> shipment = MakeShipment();
    EXPECT_THROW(shipment->ReassignVehicle(""), InvalidShipmentException);
    shipment->ReassignVehicle("T2");
    EXPECT_EQ(shipment->VehicleId(), "T2");
    shipment->Depart(1);
    EXPECT_THROW(shipment->ReassignVehicle("T3"), InvalidShipmentException);
}

TEST(DeliverySlotTest, ValidatesWindow) {
    EXPECT_THROW(DeliverySlot("", "O1", 0, 8, 10), InvalidOrderException);
    EXPECT_THROW(DeliverySlot("S1", "", 0, 8, 10), InvalidOrderException);
    EXPECT_THROW(DeliverySlot("S1", "O1", -1, 8, 10), InvalidOrderException);
    EXPECT_THROW(DeliverySlot("S1", "O1", 0, 8, 8), InvalidOrderException);
    EXPECT_THROW(DeliverySlot("S1", "O1", 0, -1, 10), InvalidOrderException);
    EXPECT_THROW(DeliverySlot("S1", "O1", 0, 8, 25), InvalidOrderException);
}

TEST(DeliverySlotTest, ComparesWindows) {
    const DeliverySlot slot("S1", "O1", 1, 8, 12);
    EXPECT_EQ(slot.DurationHours(), 4);
    EXPECT_TRUE(slot.ContainsHour(1, 8));
    EXPECT_TRUE(slot.ContainsHour(1, 11));
    EXPECT_FALSE(slot.ContainsHour(1, 12));
    EXPECT_FALSE(slot.ContainsHour(2, 8));
    EXPECT_TRUE(slot.Overlaps(DeliverySlot("S2", "O2", 1, 10, 14)));
    EXPECT_FALSE(slot.Overlaps(DeliverySlot("S3", "O3", 1, 12, 14)));
    EXPECT_FALSE(slot.Overlaps(DeliverySlot("S4", "O4", 2, 8, 12)));
    EXPECT_EQ(slot.Id(), "S1");
    EXPECT_EQ(slot.OrderId(), "O1");
    EXPECT_EQ(slot.StartHour(), 8);
    EXPECT_EQ(slot.EndHour(), 12);
}

TEST(OrderRegistryTest, RegistersLookupsAndFilters) {
    OrderRegistry registry;
    EXPECT_EQ(registry.Count(), 0U);
    EXPECT_THROW(registry.Place(nullptr), InvalidOrderException);
    registry.Place(MakeOrder("O1"));
    EXPECT_THROW(registry.Place(MakeOrder("O1")), InvalidOrderException);
    registry.Place(MakeOrder("O2"));
    EXPECT_EQ(registry.Count(), 2U);
    EXPECT_NE(registry.Find("O1"), nullptr);
    EXPECT_EQ(registry.Find("missing"), nullptr);
    EXPECT_THROW(registry.Require("missing"), OrderNotFoundException);
    EXPECT_EQ(registry.Require("O1").Id(), "O1");
    EXPECT_EQ(registry.ActiveCount(), 0U);
    registry.Require("O1").Confirm();
    EXPECT_EQ(registry.ActiveCount(), 1U);
    EXPECT_EQ(registry.ByStatus(OrderStatus::Confirmed).size(), 1U);
    EXPECT_EQ(registry.ByStatus(OrderStatus::Draft).size(), 1U);
    EXPECT_DOUBLE_EQ(registry.TotalValue(), 200.0);
    EXPECT_TRUE(registry.Remove("O2"));
    EXPECT_FALSE(registry.Remove("O2"));
    EXPECT_EQ(registry.Orders().size(), 1U);
}

}  // namespace logistics
