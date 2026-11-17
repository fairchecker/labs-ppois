#pragma once

#include <string>

#include "museum/visitor.hpp"

/**
 * @file tour.hpp
 * @brief Guided tour with a guide, capacity and price.
 */

namespace museum {

/**
 * @class Tour
 * @brief Scheduled walk through the museum led by a guide.
 */
class Tour {
    public:
    /**
     * @brief Creates a tour.
     * @throws BookingException when identifiers are empty, capacity is not positive
     *         or the price is negative.
     */
    Tour(std::string id, std::string guideId, std::string routeName, int maxVisitors,
         double price);

    /**
     * @brief Books a place for a visitor.
     * @param visitor visitor who can afford the tour.
     * @return true when the booking was added.
     * @throws BookingException when the tour is full or the visitor cannot pay.
     */
    bool Book(Visitor& visitor);

    /**
     * @brief Cancels one booked place.
     * @return true when a place was released.
     */
    bool Cancel();

    /**
     * @brief Checks whether the tour reached its capacity.
     * @return true when no places remain.
     */
    bool IsFull() const;

    /**
     * @brief Computes revenue from booked places.
     * @return booked visitors multiplied by the price.
     */
    double Revenue() const;

    /// @brief Returns the tour identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the guide identifier.
    const std::string& GuideId() const { return guideId_; }

    /// @brief Returns the route name.
    const std::string& RouteName() const { return routeName_; }

    /// @brief Returns the maximum number of visitors.
    int MaxVisitors() const { return maxVisitors_; }

    /// @brief Returns the number of booked places.
    int BookedVisitors() const { return bookedVisitors_; }

    /// @brief Returns the price per visitor.
    double Price() const { return price_; }

    private:
    /// Tour identifier.
    std::string id_;
    /// Identifier of the assigned guide.
    std::string guideId_;
    /// Route name.
    std::string routeName_;
    /// Maximum number of visitors.
    int maxVisitors_;
    /// Number of booked places.
    int bookedVisitors_ = 0;
    /// Price per visitor.
    double price_;
};

}  // namespace museum
