#include "logistics/orders.hpp"

#include <algorithm>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kHoursPerDay = 24;
constexpr double kPercentScale = 100.0;
constexpr int kFirstHour = 0;

bool IsActiveStatus(OrderStatus status) {
    return status == OrderStatus::Confirmed || status == OrderStatus::InTransit;
}

}  // namespace

Order::Order(std::string id, std::string clientId, std::string cargoId, Address origin,
             Address destination, int createdDay, double price, OrderPriority priority)
    : id_(std::move(id)),
      clientId_(std::move(clientId)),
      cargoId_(std::move(cargoId)),
      origin_(std::move(origin)),
      destination_(std::move(destination)),
      createdDay_(createdDay),
      status_(OrderStatus::Draft),
      price_(price),
      priority_(priority) {
    const bool priceValid = price_ >= 0.0 && price_ <= kMaxOrderPrice;
    if (id_.empty() || clientId_.empty() || cargoId_.empty() || createdDay_ < 0 || !priceValid) {
        throw InvalidOrderException("Order has an invalid identifier, day or price");
    }
}

void Order::Confirm() {
    if (status_ != OrderStatus::Draft) {
        throw InvalidOrderException("Only a draft order can be confirmed");
    }
    status_ = OrderStatus::Confirmed;
}

void Order::StartTransit() {
    if (status_ != OrderStatus::Confirmed) {
        throw InvalidOrderException("Only a confirmed order can start transit");
    }
    status_ = OrderStatus::InTransit;
}

void Order::Complete() {
    if (status_ != OrderStatus::InTransit) {
        throw InvalidOrderException("Only an order in transit can be completed");
    }
    status_ = OrderStatus::Delivered;
}

void Order::Cancel() {
    if (status_ == OrderStatus::Delivered || status_ == OrderStatus::Cancelled) {
        throw InvalidOrderException("A delivered or cancelled order cannot be cancelled");
    }
    status_ = OrderStatus::Cancelled;
}

bool Order::IsActive() const { return IsActiveStatus(status_); }

std::string Order::Describe() const {
    return TypeName() + " order " + id_ + " for " + clientId_ + ", price " + std::to_string(price_);
}

ExpressOrder::ExpressOrder(std::string id, std::string clientId, std::string cargoId, Address origin,
                           Address destination, int createdDay, double price, int deadlineDay,
                           double surchargePercent)
    : Order(std::move(id), std::move(clientId), std::move(cargoId), std::move(origin),
            std::move(destination), createdDay, price, OrderPriority::Express),
      deadlineDay_(deadlineDay),
      surchargePercent_(surchargePercent) {
    if (deadlineDay_ < createdDay || surchargePercent_ <= 0.0) {
        throw InvalidOrderException("Express order requires a future deadline and positive surcharge");
    }
}

bool ExpressOrder::IsLate(int currentDay) const {
    return status_ != OrderStatus::Delivered && currentDay > deadlineDay_;
}

double ExpressOrder::ApplySurcharge(double basePrice) const {
    if (basePrice < 0.0) {
        throw InvalidOrderException("Base price cannot be negative");
    }
    return basePrice * (1.0 + surchargePercent_ / kPercentScale);
}

BulkOrder::BulkOrder(std::string id, std::string clientId, std::string cargoId, Address origin,
                     Address destination, int createdDay, double price, bool requiresCrane,
                     double loadingHours)
    : Order(std::move(id), std::move(clientId), std::move(cargoId), std::move(origin),
            std::move(destination), createdDay, price, OrderPriority::Standard),
      requiresCrane_(requiresCrane),
      loadingHours_(loadingHours) {
    if (loadingHours_ <= 0.0) {
        throw InvalidOrderException("Bulk order requires positive loading hours");
    }
}

double BulkOrder::LoadingCost(double hourlyRate) const {
    if (hourlyRate <= 0.0) {
        throw InvalidOrderException("Loading hourly rate must be positive");
    }
    return loadingHours_ * hourlyRate;
}

double BulkOrder::TotalHandlingCost(double hourlyRate, double craneFee) const {
    if (craneFee < 0.0) {
        throw InvalidOrderException("Crane fee cannot be negative");
    }
    const double loadingCost = LoadingCost(hourlyRate);
    if (!requiresCrane_) {
        return loadingCost;
    }
    return loadingCost + craneFee;
}

Shipment::Shipment(std::string id, std::string orderId, std::string vehicleId,
                   std::string driverId, std::string routeId)
    : id_(std::move(id)),
      orderId_(std::move(orderId)),
      vehicleId_(std::move(vehicleId)),
      driverId_(std::move(driverId)),
      routeId_(std::move(routeId)),
      departureDay_(-1),
      arrivalDay_(-1),
      status_(ShipmentStatus::Planned) {
    if (id_.empty() || orderId_.empty() || vehicleId_.empty() || driverId_.empty() ||
        routeId_.empty()) {
        throw InvalidShipmentException("Shipment requires order, vehicle, driver and route identifiers");
    }
}

void Shipment::Depart(int day) {
    if (status_ != ShipmentStatus::Planned || day < 0) {
        throw InvalidShipmentException("Only a planned shipment can depart on a valid day");
    }
    departureDay_ = day;
    status_ = ShipmentStatus::InTransit;
}

void Shipment::Arrive(int day) {
    const bool canArrive = status_ == ShipmentStatus::InTransit || status_ == ShipmentStatus::Delayed;
    if (!canArrive || day < departureDay_) {
        throw InvalidShipmentException("Shipment cannot arrive in its current state");
    }
    arrivalDay_ = day;
    status_ = ShipmentStatus::Delivered;
}

void Shipment::MarkDelayed() {
    if (status_ != ShipmentStatus::InTransit) {
        throw InvalidShipmentException("Only a shipment in transit can be delayed");
    }
    status_ = ShipmentStatus::Delayed;
}

void Shipment::ReassignVehicle(const std::string& vehicleId) {
    if (status_ != ShipmentStatus::Planned || vehicleId.empty()) {
        throw InvalidShipmentException("Vehicle can only be reassigned before departure");
    }
    vehicleId_ = vehicleId;
}

int Shipment::TransitDays(int currentDay) const {
    if (status_ == ShipmentStatus::Planned) {
        return 0;
    }
    if (status_ == ShipmentStatus::Delivered) {
        return arrivalDay_ - departureDay_;
    }
    return std::max(0, currentDay - departureDay_);
}

std::string Shipment::Describe() const {
    return "shipment " + id_ + " of order " + orderId_ + " by vehicle " + vehicleId_;
}

DeliverySlot::DeliverySlot(std::string id, std::string orderId, int day, int startHour,
                           int endHour)
    : id_(std::move(id)),
      orderId_(std::move(orderId)),
      day_(day),
      startHour_(startHour),
      endHour_(endHour) {
    const bool windowValid = startHour_ >= kFirstHour && startHour_ < endHour_;
    if (id_.empty() || orderId_.empty() || day < 0 || !windowValid || endHour_ > kHoursPerDay) {
        throw InvalidOrderException("Delivery slot requires a valid day and hour window");
    }
}

int DeliverySlot::DurationHours() const { return endHour_ - startHour_; }

bool DeliverySlot::Overlaps(const DeliverySlot& other) const {
    if (day_ != other.day_) {
        return false;
    }
    return startHour_ < other.endHour_ && other.startHour_ < endHour_;
}

bool DeliverySlot::ContainsHour(int day, int hour) const {
    return day_ == day && hour >= startHour_ && hour < endHour_;
}

void OrderRegistry::Place(std::unique_ptr<Order> order) {
    if (!order) {
        throw InvalidOrderException("Cannot register an empty order");
    }
    if (Find(order->Id()) != nullptr) {
        throw InvalidOrderException("Order identifier is already registered");
    }
    orders_.push_back(std::move(order));
}

Order* OrderRegistry::Find(const std::string& orderId) {
    for (const std::unique_ptr<Order>& order : orders_) {
        if (order->Id() == orderId) {
            return order.get();
        }
    }
    return nullptr;
}

Order& OrderRegistry::Require(const std::string& orderId) {
    Order* order = Find(orderId);
    if (order == nullptr) {
        throw OrderNotFoundException("Order not found: " + orderId);
    }
    return *order;
}

std::vector<Order*> OrderRegistry::ByStatus(OrderStatus status) {
    std::vector<Order*> matching;
    for (const std::unique_ptr<Order>& order : orders_) {
        if (order->Status() == status) {
            matching.push_back(order.get());
        }
    }
    return matching;
}

bool OrderRegistry::Remove(const std::string& orderId) {
    const auto found = std::find_if(
        orders_.begin(), orders_.end(),
        [&orderId](const std::unique_ptr<Order>& order) { return order->Id() == orderId; });
    if (found == orders_.end()) {
        return false;
    }
    orders_.erase(found);
    return true;
}

std::size_t OrderRegistry::ActiveCount() const {
    const auto active = std::count_if(
        orders_.begin(), orders_.end(),
        [](const std::unique_ptr<Order>& order) { return order->IsActive(); });
    return static_cast<std::size_t>(active);
}

double OrderRegistry::TotalValue() const {
    double total = 0.0;
    for (const std::unique_ptr<Order>& order : orders_) {
        total += order->Price();
    }
    return total;
}

}  // namespace logistics
