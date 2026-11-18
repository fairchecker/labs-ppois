#pragma once

#include <memory>
#include <string>
#include <vector>

#include "logistics/clients.hpp"
#include "logistics/finance.hpp"
#include "logistics/orders.hpp"
#include "logistics/people.hpp"
#include "logistics/tracking.hpp"
#include "logistics/vehicles.hpp"
#include "logistics/warehouse.hpp"

/**
 * @file company.hpp
 * @brief Aggregate root that coordinates the transport company subsystems.
 */

namespace logistics {

/// @brief Fuel surcharge applied by the default pricing engine.
inline constexpr double kDefaultFuelSurchargePercent = 5.0;
/// @brief Profit tax rate used by the default finance ledger.
inline constexpr double kDefaultTaxRatePercent = 20.0;

/**
 * @class TransportCompany
 * @brief Owns clients, warehouses, fleet, orders, shipments and finances.
 */
class TransportCompany {
    public:
    /**
     * @brief Creates an empty company.
     * @throws InvalidPaymentException when the name is empty or the balance is negative.
     */
    TransportCompany(std::string name, double initialBalance);

    /// @brief Returns the company name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the vehicle fleet.
    VehicleFleet& Fleet() { return fleet_; }

    /// @brief Returns the vehicle fleet.
    const VehicleFleet& Fleet() const { return fleet_; }

    /// @brief Returns the warehouses.
    const std::vector<Warehouse>& Warehouses() const { return warehouses_; }

    /// @brief Returns the clients.
    const std::vector<Client>& Clients() const { return clients_; }

    /// @brief Returns the orders.
    OrderRegistry& Orders() { return orders_; }

    /// @brief Returns the orders.
    const OrderRegistry& Orders() const { return orders_; }

    /// @brief Returns the tracking service.
    TrackingService& Tracking() { return tracking_; }

    /// @brief Returns the tracking service.
    const TrackingService& Tracking() const { return tracking_; }

    /// @brief Returns the finance ledger.
    CompanyFinance& Finance() { return finance_; }

    /// @brief Returns the finance ledger.
    const CompanyFinance& Finance() const { return finance_; }

    /// @brief Returns the pricing engine.
    PricingEngine& Pricing() { return pricing_; }

    /// @brief Returns the pricing engine.
    const PricingEngine& Pricing() const { return pricing_; }

    /// @brief Returns the shipments.
    const std::vector<Shipment>& Shipments() const { return shipments_; }

    /// @brief Returns the invoices.
    const std::vector<Invoice>& Invoices() const { return invoices_; }

    /// @brief Returns the payments.
    const std::vector<Payment>& Payments() const { return payments_; }

    /**
     * @brief Registers a client.
     * @throws InvalidClientException when the identifier is already used.
     */
    void OnboardClient(const Client& client);

    /**
     * @brief Finds a client by identifier.
     * @return pointer to the client, or nullptr.
     */
    const Client* FindClient(const std::string& clientId) const;

    /**
     * @brief Finds a client and fails when it is missing.
     * @throws InvalidClientException when the client does not exist.
     */
    Client& RequireClient(const std::string& clientId);

    /**
     * @brief Adds a warehouse.
     * @throws InvalidCargoException when the identifier is already used.
     */
    void AddWarehouse(const Warehouse& warehouse);

    /**
     * @brief Finds a warehouse by identifier.
     * @return pointer to the warehouse, or nullptr.
     */
    const Warehouse* FindWarehouse(const std::string& warehouseId) const;

    /**
     * @brief Registers a vehicle in the fleet.
     * @throws InvalidVehicleException when the identifier is already used.
     */
    void RegisterVehicle(std::unique_ptr<Vehicle> vehicle);

    /// @brief Registers a tariff in the pricing engine.
    void AddTariff(const Tariff& tariff);

    /**
     * @brief Registers a new order.
     * @throws InvalidClientException when the client does not exist.
     */
    void PlaceOrder(std::unique_ptr<Order> order);

    /**
     * @brief Quotes a shipment price for a cargo type.
     * @throws InvalidTariffException when no tariff applies.
     */
    double Quote(const std::string& cargoType, double distanceKm, double weightKg,
                 double volumeM3) const;

    /**
     * @brief Dispatches a confirmed order.
     * @param order confirmed order.
     * @param vehicle available vehicle.
     * @param driver available driver with a matching license.
     * @param licenseCategory category required for the vehicle.
     * @param routeId route assigned to the shipment.
     * @param day departure day.
     * @throws InvalidOrderException when the order is not confirmed.
     * @throws VehicleNotAvailableException when the vehicle is unavailable.
     * @throws DriverNotAvailableException when the driver is unavailable or lacks the category.
     * @return the created shipment.
     */
    Shipment& Dispatch(Order& order, Vehicle& vehicle, Driver& driver,
                       const std::string& licenseCategory, const std::string& routeId, int day);

    /**
     * @brief Completes a shipment and its order.
     * @throws InvalidShipmentException when the shipment is missing or not in transit.
     */
    void CompleteDelivery(const std::string& shipmentId, int day);

    /**
     * @brief Finds a shipment by identifier.
     * @throws InvalidShipmentException when the shipment does not exist.
     */
    const Shipment& RequireShipment(const std::string& shipmentId) const;

    /**
     * @brief Issues an invoice and charges the client.
     * @throws InvalidClientException when the client or order is missing.
     * @throws InsufficientFundsException when the client cannot pay.
     */
    Invoice& BillClient(const std::string& clientId, const std::string& orderId, double amount,
                        int issuedDay, int dueDay);

    /**
     * @brief Applies a full payment to an invoice.
     * @throws InvalidPaymentException when the invoice is missing, paid or the amount differs.
     */
    Payment ReceivePayment(const std::string& invoiceId, double amount, PaymentMethod method,
                           int paidDay, const std::string& reference);

    /// @brief Records a company expense.
    void RecordExpense(double amount);

    /// @brief Stores a tracking point.
    void RecordTrackingPoint(const TrackPoint& point);

    /// @brief Builds a multi-line company report.
    std::string GenerateReport() const;

    /// @brief Returns the number of clients.
    std::size_t ClientCount() const { return clients_.size(); }

    /// @brief Returns the number of warehouses.
    std::size_t WarehouseCount() const { return warehouses_.size(); }

    /// @brief Returns the number of shipments.
    std::size_t ShipmentCount() const { return shipments_.size(); }

    /// @brief Returns the number of invoices.
    std::size_t InvoiceCount() const { return invoices_.size(); }

    private:
    void ValidateDispatch(const Order& order, const Vehicle& vehicle, const Driver& driver,
                          const std::string& licenseCategory, int day) const;
    Invoice* FindInvoice(const std::string& invoiceId);
    Shipment* FindShipment(const std::string& shipmentId);
    std::size_t CountPaidInvoices() const;

    std::string name_;
    VehicleFleet fleet_;
    std::vector<Warehouse> warehouses_;
    std::vector<Client> clients_;
    OrderRegistry orders_;
    TrackingService tracking_;
    CompanyFinance finance_;
    PricingEngine pricing_;
    std::vector<Shipment> shipments_;
    std::vector<Invoice> invoices_;
    std::vector<Payment> payments_;
};

}  // namespace logistics
