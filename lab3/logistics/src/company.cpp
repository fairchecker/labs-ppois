#include "logistics/company.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kReportPrecision = 2;

std::string MakeShipmentId(std::size_t index) { return "SHP-" + std::to_string(index + 1); }

std::string MakeInvoiceId(std::size_t index) { return "INV-" + std::to_string(index + 1); }

std::string MakePaymentId(std::size_t index) { return "PAY-" + std::to_string(index + 1); }

}  // namespace

TransportCompany::TransportCompany(std::string name, double initialBalance)
    : name_(std::move(name)),
      finance_(initialBalance, 0.0, 0.0, kDefaultTaxRatePercent),
      pricing_(kDefaultFuelSurchargePercent) {
    if (name_.empty() || initialBalance < 0.0) {
        throw InvalidPaymentException("Company requires a name and a non-negative initial balance");
    }
}

void TransportCompany::OnboardClient(const Client& client) {
    if (FindClient(client.Id()) != nullptr) {
        throw InvalidClientException("Client identifier is already registered");
    }
    clients_.push_back(client);
}

const Client* TransportCompany::FindClient(const std::string& clientId) const {
    for (const Client& client : clients_) {
        if (client.Id() == clientId) {
            return &client;
        }
    }
    return nullptr;
}

Client& TransportCompany::RequireClient(const std::string& clientId) {
    for (Client& client : clients_) {
        if (client.Id() == clientId) {
            return client;
        }
    }
    throw InvalidClientException("Client not found: " + clientId);
}

void TransportCompany::AddWarehouse(const Warehouse& warehouse) {
    if (FindWarehouse(warehouse.Id()) != nullptr) {
        throw InvalidCargoException("Warehouse identifier is already registered");
    }
    warehouses_.push_back(warehouse);
}

const Warehouse* TransportCompany::FindWarehouse(const std::string& warehouseId) const {
    for (const Warehouse& warehouse : warehouses_) {
        if (warehouse.Id() == warehouseId) {
            return &warehouse;
        }
    }
    return nullptr;
}

void TransportCompany::RegisterVehicle(std::unique_ptr<Vehicle> vehicle) {
    fleet_.AddVehicle(std::move(vehicle));
}

void TransportCompany::AddTariff(const Tariff& tariff) { pricing_.AddTariff(tariff); }

void TransportCompany::PlaceOrder(std::unique_ptr<Order> order) {
    if (!order) {
        throw InvalidOrderException("Cannot place an empty order");
    }
    if (FindClient(order->ClientId()) == nullptr) {
        throw InvalidClientException("Order refers to an unknown client");
    }
    orders_.Place(std::move(order));
}

double TransportCompany::Quote(const std::string& cargoType, double distanceKm, double weightKg,
                               double volumeM3) const {
    return pricing_.Quote(cargoType, distanceKm, weightKg, volumeM3);
}

Shipment& TransportCompany::Dispatch(Order& order, Vehicle& vehicle, Driver& driver,
                                     const std::string& licenseCategory, const std::string& routeId,
                                     int day) {
    if (order.Status() != OrderStatus::Confirmed) {
        throw InvalidOrderException("Only a confirmed order can be dispatched");
    }
    if (!vehicle.IsAvailable()) {
        throw VehicleNotAvailableException("Vehicle is not available for dispatch");
    }
    if (!driver.CanDrive(licenseCategory)) {
        throw DriverNotAvailableException("Driver cannot operate the requested vehicle category");
    }
    if (day < 0) {
        throw InvalidShipmentException("Dispatch day cannot be negative");
    }
    Shipment shipment(MakeShipmentId(shipments_.size()), order.Id(), vehicle.Id(), driver.Id(),
                      routeId);
    order.StartTransit();
    vehicle.MarkOnRoute();
    driver.BeginTrip();
    shipment.Depart(day);
    shipments_.push_back(std::move(shipment));
    return shipments_.back();
}

void TransportCompany::CompleteDelivery(const std::string& shipmentId, int day) {
    Shipment* shipment = FindShipment(shipmentId);
    if (shipment == nullptr) {
        throw InvalidShipmentException("Shipment not found: " + shipmentId);
    }
    Order* order = orders_.Find(shipment->OrderId());
    if (order == nullptr) {
        throw OrderNotFoundException("Order of the shipment was not found");
    }
    shipment->Arrive(day);
    order->Complete();
    Vehicle* vehicle = fleet_.FindVehicle(shipment->VehicleId());
    if (vehicle != nullptr && vehicle->Status() == VehicleStatus::OnRoute) {
        vehicle->CompleteRoute();
    }
}

const Shipment& TransportCompany::RequireShipment(const std::string& shipmentId) const {
    for (const Shipment& shipment : shipments_) {
        if (shipment.Id() == shipmentId) {
            return shipment;
        }
    }
    throw InvalidShipmentException("Shipment not found: " + shipmentId);
}

Shipment* TransportCompany::FindShipment(const std::string& shipmentId) {
    for (Shipment& shipment : shipments_) {
        if (shipment.Id() == shipmentId) {
            return &shipment;
        }
    }
    return nullptr;
}

Invoice& TransportCompany::BillClient(const std::string& clientId, const std::string& orderId,
                                      double amount, int issuedDay, int dueDay) {
    Client& client = RequireClient(clientId);
    Order& order = orders_.Require(orderId);
    if (order.ClientId() != clientId) {
        throw InvalidClientException("Order belongs to another client");
    }
    Invoice invoice(MakeInvoiceId(invoices_.size()), clientId, orderId, amount, issuedDay, dueDay);
    client.Charge(amount);
    invoices_.push_back(std::move(invoice));
    return invoices_.back();
}

Payment TransportCompany::ReceivePayment(const std::string& invoiceId, double amount,
                                         PaymentMethod method, int paidDay,
                                         const std::string& reference) {
    Invoice* invoice = FindInvoice(invoiceId);
    if (invoice == nullptr) {
        throw InvalidPaymentException("Invoice not found: " + invoiceId);
    }
    if (invoice->IsPaid()) {
        throw InvalidPaymentException("Invoice is already paid");
    }
    if (amount != invoice->Amount()) {
        throw InvalidPaymentException("Payment amount does not match the invoice");
    }
    Payment payment(MakePaymentId(payments_.size()), invoiceId, amount, method, paidDay, reference);
    invoice->MarkPaid();
    finance_.RecordRevenue(amount);
    payments_.push_back(std::move(payment));
    return payments_.back();
}

Invoice* TransportCompany::FindInvoice(const std::string& invoiceId) {
    for (Invoice& invoice : invoices_) {
        if (invoice.Id() == invoiceId) {
            return &invoice;
        }
    }
    return nullptr;
}

void TransportCompany::RecordExpense(double amount) { finance_.RecordExpense(amount); }

void TransportCompany::RecordTrackingPoint(const TrackPoint& point) { tracking_.Record(point); }

std::string TransportCompany::GenerateReport() const {
    std::ostringstream stream;
    stream << "company: " << name_ << '\n';
    stream << "clients: " << clients_.size() << ", warehouses: " << warehouses_.size()
           << ", vehicles: " << fleet_.VehicleCount() << '\n';
    stream << "orders: " << orders_.Count() << " (" << orders_.ActiveCount() << " active), shipments "
           << shipments_.size() << '\n';
    stream << "invoices: " << invoices_.size() << ", paid: " << CountPaidInvoices() << '\n';
    stream << std::fixed << std::setprecision(kReportPrecision);
    stream << "balance: " << finance_.Balance() << ", gross: " << finance_.GrossProfit()
           << ", net: " << finance_.NetProfit();
    return stream.str();
}

std::size_t TransportCompany::CountPaidInvoices() const {
    std::size_t paid = 0;
    for (const Invoice& invoice : invoices_) {
        if (invoice.IsPaid()) {
            ++paid;
        }
    }
    return paid;
}

}  // namespace logistics
