#pragma once

#include <string>

/**
 * @file event.hpp
 * @brief Special museum event with a fixed capacity.
 */

namespace museum {

/**
 * @class Event
 * @brief Evening lecture or opening with ticketed attendance.
 */
class Event {
    public:
    /**
     * @brief Creates an event.
     * @throws InvalidExhibitionException when fields are invalid.
     */
    Event(std::string id, std::string name, int day, int capacity, double ticketPrice);

    /**
     * @brief Sells a number of tickets.
     * @param count strictly positive number of tickets.
     * @return the total number of sold tickets after the sale.
     * @throws InvalidTicketException when count is not positive.
     * @throws TicketSoldOutException when the sale exceeds the capacity.
     */
    int SellTickets(int count);

    /**
     * @brief Checks whether every seat is sold.
     * @return true when tickets sold reached the capacity.
     */
    bool IsSoldOut() const;

    /**
     * @brief Computes revenue from sold tickets.
     * @return tickets sold multiplied by the ticket price.
     */
    double Revenue() const;

    /// @brief Returns the event identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the event name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the event day.
    int Day() const { return day_; }

    /// @brief Returns the seat capacity.
    int Capacity() const { return capacity_; }

    /// @brief Returns the number of sold tickets.
    int TicketsSold() const { return ticketsSold_; }

    private:
    /// Event identifier.
    std::string id_;
    /// Event name.
    std::string name_;
    /// Day of the event.
    int day_;
    /// Maximum number of seats.
    int capacity_;
    /// Number of tickets sold so far.
    int ticketsSold_ = 0;
    /// Price of one ticket.
    double ticketPrice_;
};

}  // namespace museum
