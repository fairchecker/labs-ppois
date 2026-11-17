#pragma once

#include <stdexcept>
#include <string>

/**
 * @file exceptions.hpp
 * @brief Custom exception hierarchy for the museum domain.
 */

namespace museum {

/// Base class for every museum-specific error.
class MuseumException : public std::runtime_error {
    public:
    explicit MuseumException(const std::string& message) : std::runtime_error(message) {}
};

/// Thrown when a person record is malformed.
class InvalidPersonException : public MuseumException {
    public:
    explicit InvalidPersonException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when an address record is malformed.
class InvalidAddressException : public MuseumException {
    public:
    explicit InvalidAddressException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a membership record is malformed.
class InvalidMembershipException : public MuseumException {
    public:
    explicit InvalidMembershipException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when an exhibit record is malformed.
class InvalidExhibitException : public MuseumException {
    public:
    explicit InvalidExhibitException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when an exhibit cannot be found in its container.
class ExhibitNotFoundException : public MuseumException {
    public:
    explicit ExhibitNotFoundException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a condition score is outside the supported range.
class InvalidConditionException : public MuseumException {
    public:
    explicit InvalidConditionException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a department cannot be found.
class DepartmentNotFoundException : public MuseumException {
    public:
    explicit DepartmentNotFoundException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a department record is malformed.
class InvalidDepartmentException : public MuseumException {
    public:
    explicit InvalidDepartmentException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a restoration workflow is violated.
class RestorationException : public MuseumException {
    public:
    explicit RestorationException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a ticket record is malformed.
class InvalidTicketException : public MuseumException {
    public:
    explicit InvalidTicketException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when no more tickets are available.
class TicketSoldOutException : public MuseumException {
    public:
    explicit TicketSoldOutException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a booking operation cannot be completed.
class BookingException : public MuseumException {
    public:
    explicit BookingException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a budget record is malformed.
class InvalidBudgetException : public MuseumException {
    public:
    explicit InvalidBudgetException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when spending would exceed the available budget.
class BudgetExceededException : public MuseumException {
    public:
    explicit BudgetExceededException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a loan agreement is invalid.
class LoanException : public MuseumException {
    public:
    explicit LoanException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a security operation cannot be performed.
class SecurityException : public MuseumException {
    public:
    explicit SecurityException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a person lacks the required access level.
class AccessDeniedException : public MuseumException {
    public:
    explicit AccessDeniedException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a rating is outside the supported range.
class InvalidRatingException : public MuseumException {
    public:
    explicit InvalidRatingException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when an exhibition record is malformed.
class InvalidExhibitionException : public MuseumException {
    public:
    explicit InvalidExhibitionException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a shop operation is invalid.
class InvalidShopItemException : public MuseumException {
    public:
    explicit InvalidShopItemException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a climate reading is implausible.
class ClimateException : public MuseumException {
    public:
    explicit ClimateException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when an insurance policy cannot cover a claim.
class InsuranceException : public MuseumException {
    public:
    explicit InsuranceException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a library operation cannot be completed.
class LibraryException : public MuseumException {
    public:
    explicit LibraryException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a research workflow is violated.
class ResearchException : public MuseumException {
    public:
    explicit ResearchException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when visitor feedback cannot be processed.
class FeedbackException : public MuseumException {
    public:
    explicit FeedbackException(const std::string& message) : MuseumException(message) {}
};

/// Thrown when a museum facility operation is invalid.
class FacilityException : public MuseumException {
    public:
    explicit FacilityException(const std::string& message) : MuseumException(message) {}
};

}  // namespace museum
