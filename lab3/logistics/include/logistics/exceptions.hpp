#pragma once

#include <stdexcept>
#include <string>

/**
 * @file exceptions.hpp
 * @brief Domain exception hierarchy of the transport and logistics model.
 */

namespace logistics {

/// @brief Base class for every domain error raised by the model.
class TransportException : public std::runtime_error {
    public:
    /**
     * @brief Creates a domain error with a human-readable message.
     * @param message description of the failed invariant.
     */
    explicit TransportException(const std::string& message) : std::runtime_error(message) {}
};

/// @brief A route, segment or waypoint violates its invariants.
class InvalidRouteException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A vehicle record is malformed.
class InvalidVehicleException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A vehicle cannot be used for the requested operation.
class VehicleNotAvailableException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A vehicle would exceed its weight or volume limit.
class VehicleOverloadException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A driver cannot take the requested assignment.
class DriverNotAvailableException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A driving license is malformed, expired or insufficient.
class InvalidLicenseException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A cargo record is malformed.
class InvalidCargoException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A cargo exceeds the allowed shipment weight.
class CargoOverweightException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief An order record or state transition is invalid.
class InvalidOrderException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief The requested order does not exist.
class OrderNotFoundException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A client record is malformed or the credit limit is exceeded.
class InvalidClientException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief An account has insufficient funds for the operation.
class InsufficientFundsException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A warehouse has no free capacity.
class WarehouseCapacityException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A shipment record or transition is invalid.
class InvalidShipmentException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief Tracking data is missing or malformed.
class TrackingException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A tariff is malformed or missing for a cargo type.
class InvalidTariffException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief A payment cannot be applied to the invoice.
class InvalidPaymentException : public TransportException {
    public:
    using TransportException::TransportException;
};

/// @brief An insurance claim violates its invariants.
class InsuranceClaimException : public TransportException {
    public:
    using TransportException::TransportException;
};

}  // namespace logistics
