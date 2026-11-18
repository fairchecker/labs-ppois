#pragma once

#include <memory>
#include <string>
#include <vector>

#include "logistics/geo.hpp"

/**
 * @file orders.hpp
 * @brief Orders, shipments and delivery scheduling.
 */

namespace logistics {

/// @brief Lifecycle state of an order.
enum class OrderStatus { Draft, Confirmed, InTransit, Delivered, Cancelled };
/// @brief Service priority of an order.
enum class OrderPriority { Standard, Express, Critical };
/// @brief Lifecycle state of a shipment.
enum class ShipmentStatus { Planned, InTransit, Delivered, Delayed };
/// @brief Highest accepted loyalty tier, reused as the tier ceiling.
inline constexpr double kMaxOrderPrice = 1000000.0;

/**
 * @class Order
 * @brief Customer request to move cargo between two addresses.
 */
class Order {
    public:
    /**
     * @brief Creates a draft order.
     * @throws InvalidOrderException for empty identifiers or a negative price.
     */
    Order(std::string id, std::string clientId, std::string cargoId, Address origin,
          Address destination, int createdDay, double price, OrderPriority priority);

    /// @brief Virtual destructor for polymorphic order collections.
    virtual ~Order() = default;

    /// @brief Returns the order identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the client identifier.
    const std::string& ClientId() const { return clientId_; }

    /// @brief Returns the cargo identifier.
    const std::string& CargoId() const { return cargoId_; }

    /// @brief Returns the pickup address.
    const Address& Origin() const { return origin_; }

    /// @brief Returns the delivery address.
    const Address& Destination() const { return destination_; }

    /// @brief Returns the creation day.
    int CreatedDay() const { return createdDay_; }

    /// @brief Returns the lifecycle status.
    OrderStatus Status() const { return status_; }

    /// @brief Returns the agreed price.
    double Price() const { return price_; }

    /// @brief Returns the service priority.
    OrderPriority Priority() const { return priority_; }

    /// @brief Confirms a draft order.
    void Confirm();

    /// @brief Starts transit for a confirmed order.
    void StartTransit();

    /// @brief Completes an order in transit.
    void Complete();

    /// @brief Cancels an order that is not delivered yet.
    void Cancel();

    /// @brief Checks whether the order still requires work.
    bool IsActive() const;

    /// @brief Builds a one-line order summary.
    std::string Describe() const;

    /// @brief Returns the order type name.
    virtual std::string TypeName() const { return "standard"; }

    protected:
    std::string id_;
    std::string clientId_;
    std::string cargoId_;
    Address origin_;
    Address destination_;
    int createdDay_;
    OrderStatus status_;
    double price_;
    OrderPriority priority_;
};

/**
 * @class ExpressOrder
 * @brief Order with a guaranteed deadline and a price surcharge.
 */
class ExpressOrder : public Order {
    public:
    /**
     * @brief Creates an express order.
     * @throws InvalidOrderException when the deadline precedes creation or the surcharge is invalid.
     */
    ExpressOrder(std::string id, std::string clientId, std::string cargoId, Address origin,
                 Address destination, int createdDay, double price, int deadlineDay,
                 double surchargePercent);

    /// @brief Returns the promised delivery day.
    int DeadlineDay() const { return deadlineDay_; }

    /// @brief Returns the surcharge percentage.
    double SurchargePercent() const { return surchargePercent_; }

    /**
     * @brief Checks whether the deadline has passed while the order is open.
     * @param currentDay current calendar day.
     */
    bool IsLate(int currentDay) const;

    /**
     * @brief Applies the express surcharge to a base price.
     * @param basePrice price before the surcharge.
     */
    double ApplySurcharge(double basePrice) const;

    /// @brief Returns the order type name.
    std::string TypeName() const override { return "express"; }

    private:
    int deadlineDay_;
    double surchargePercent_;
};

/**
 * @class BulkOrder
 * @brief Heavy order that may need a crane and extra loading time.
 */
class BulkOrder : public Order {
    public:
    /**
     * @brief Creates a bulk order.
     * @throws InvalidOrderException when loading hours are not positive.
     */
    BulkOrder(std::string id, std::string clientId, std::string cargoId, Address origin,
              Address destination, int createdDay, double price, bool requiresCrane,
              double loadingHours);

    /// @brief Checks whether a crane is required.
    bool RequiresCrane() const { return requiresCrane_; }

    /// @brief Returns the planned loading time in hours.
    double LoadingHours() const { return loadingHours_; }

    /**
     * @brief Loading cost at an hourly rate.
     * @param hourlyRate cost of one loading hour.
     */
    double LoadingCost(double hourlyRate) const;

    /**
     * @brief Total handling cost with an optional crane fee.
     * @param hourlyRate cost of one loading hour.
     * @param craneFee fixed crane fee.
     */
    double TotalHandlingCost(double hourlyRate, double craneFee) const;

    /// @brief Returns the order type name.
    std::string TypeName() const override { return "bulk"; }

    private:
    bool requiresCrane_;
    double loadingHours_;
};

/**
 * @class Shipment
 * @brief Physical movement of an order by a vehicle and driver.
 */
class Shipment {
    public:
    /**
     * @brief Creates a planned shipment.
     * @throws InvalidShipmentException for empty identifiers or negative days.
     */
    Shipment(std::string id, std::string orderId, std::string vehicleId, std::string driverId,
             std::string routeId);

    /// @brief Returns the shipment identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the order identifier.
    const std::string& OrderId() const { return orderId_; }

    /// @brief Returns the assigned vehicle identifier.
    const std::string& VehicleId() const { return vehicleId_; }

    /// @brief Returns the assigned driver identifier.
    const std::string& DriverId() const { return driverId_; }

    /// @brief Returns the route identifier.
    const std::string& RouteId() const { return routeId_; }

    /// @brief Returns the departure day.
    int DepartureDay() const { return departureDay_; }

    /// @brief Returns the arrival day.
    int ArrivalDay() const { return arrivalDay_; }

    /// @brief Returns the shipment status.
    ShipmentStatus Status() const { return status_; }

    /**
     * @brief Starts the shipment.
     * @param day departure day.
     * @throws InvalidShipmentException when the shipment is not planned.
     */
    void Depart(int day);

    /**
     * @brief Completes the shipment.
     * @param day arrival day.
     * @throws InvalidShipmentException when the shipment is not in transit or the day is invalid.
     */
    void Arrive(int day);

    /**
     * @brief Marks an in-transit shipment as delayed.
     * @throws InvalidShipmentException when the shipment is not in transit.
     */
    void MarkDelayed();

    /**
     * @brief Replaces the vehicle before departure.
     * @throws InvalidShipmentException when the shipment has already departed.
     */
    void ReassignVehicle(const std::string& vehicleId);

    /// @brief Checks whether the shipment has been delivered.
    bool IsDelivered() const { return status_ == ShipmentStatus::Delivered; }

    /**
     * @brief Days spent or planned in transit.
     * @param currentDay current calendar day, used before arrival.
     */
    int TransitDays(int currentDay) const;

    /// @brief Builds a one-line shipment summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string orderId_;
    std::string vehicleId_;
    std::string driverId_;
    std::string routeId_;
    int departureDay_;
    int arrivalDay_;
    ShipmentStatus status_;
};

/**
 * @class DeliverySlot
 * @brief Time window promised to the client.
 */
class DeliverySlot {
    public:
    /**
     * @brief Creates a delivery slot.
     * @throws InvalidOrderException when the day or hour window is invalid.
     */
    DeliverySlot(std::string id, std::string orderId, int day, int startHour, int endHour);

    /// @brief Returns the slot identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the order identifier.
    const std::string& OrderId() const { return orderId_; }

    /// @brief Returns the delivery day.
    int Day() const { return day_; }

    /// @brief Returns the inclusive start hour.
    int StartHour() const { return startHour_; }

    /// @brief Returns the exclusive end hour.
    int EndHour() const { return endHour_; }

    /// @brief Returns the window length in hours.
    int DurationHours() const;

    /**
     * @brief Checks whether two slots overlap.
     * @param other slot on the same or another day.
     */
    bool Overlaps(const DeliverySlot& other) const;

    /**
     * @brief Checks whether an hour falls inside the slot.
     * @param day delivery day.
     * @param hour hour of the day.
     */
    bool ContainsHour(int day, int hour) const;

    private:
    std::string id_;
    std::string orderId_;
    int day_;
    int startHour_;
    int endHour_;
};

/**
 * @class OrderRegistry
 * @brief Owns orders and supports lookup and filtering.
 */
class OrderRegistry {
    public:
    /// @brief Creates an empty registry.
    OrderRegistry() = default;

    /// @brief Returns all orders.
    const std::vector<std::unique_ptr<Order>>& Orders() const { return orders_; }

    /**
     * @brief Registers an order.
     * @throws InvalidOrderException when the pointer is null or the identifier is duplicated.
     */
    void Place(std::unique_ptr<Order> order);

    /**
     * @brief Finds an order by identifier.
     * @return pointer to the order, or nullptr.
     */
    Order* Find(const std::string& orderId);

    /**
     * @brief Finds an order and fails when it is missing.
     * @throws OrderNotFoundException when the order does not exist.
     */
    Order& Require(const std::string& orderId);

    /// @brief Returns orders in a given status.
    std::vector<Order*> ByStatus(OrderStatus status);

    /**
     * @brief Removes an order.
     * @return true when the order was present.
     */
    bool Remove(const std::string& orderId);

    /// @brief Returns the number of active orders.
    std::size_t ActiveCount() const;

    /// @brief Returns the sum of prices of all orders.
    double TotalValue() const;

    /// @brief Returns the number of registered orders.
    std::size_t Count() const { return orders_.size(); }

    private:
    std::vector<std::unique_ptr<Order>> orders_;
};

}  // namespace logistics
