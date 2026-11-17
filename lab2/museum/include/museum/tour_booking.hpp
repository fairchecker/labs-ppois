#pragma once

#include <string>

/**
 * @file tour_booking.hpp
 * @brief Booking record for a guided tour.
 */

namespace museum {

/**
 * @class TourBooking
 * @brief Tracks payment and cancellation state of a tour booking.
 */
class TourBooking {
    public:
    /**
     * @brief Creates a booking.
     * @throws BookingException when identifiers are empty or the amount is negative.
     */
    TourBooking(std::string tourId, std::string visitorId, int day, double amount);

    /**
     * @brief Pays for the booking.
     * @return true when the booking changed to paid.
     * @throws BookingException when the booking was cancelled.
     */
    bool Pay();

    /**
     * @brief Cancels the booking.
     * @return true when the booking changed to cancelled.
     * @throws BookingException when the booking is already paid.
     */
    bool Cancel();

    /// @brief Checks whether the booking is paid.
    bool IsPaid() const { return paid_; }

    /// @brief Checks whether the booking is cancelled.
    bool IsCancelled() const { return cancelled_; }

    /// @brief Returns the amount due.
    double Amount() const { return amount_; }

    /// @brief Returns the tour identifier.
    const std::string& TourId() const { return tourId_; }

    /// @brief Returns the visitor identifier.
    const std::string& VisitorId() const { return visitorId_; }

    /// @brief Returns the booking day.
    int Day() const { return day_; }

    private:
    /// Tour identifier.
    std::string tourId_;
    /// Visitor identifier.
    std::string visitorId_;
    /// Day of the tour.
    int day_;
    /// Amount due for the booking.
    double amount_;
    /// Whether the booking is paid.
    bool paid_ = false;
    /// Whether the booking is cancelled.
    bool cancelled_ = false;
};

}  // namespace museum
