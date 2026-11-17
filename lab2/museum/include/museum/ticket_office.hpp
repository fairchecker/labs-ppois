#pragma once

#include <string>
#include <vector>

#include "museum/ticket.hpp"
#include "museum/visitor.hpp"

/**
 * @file ticket_office.hpp
 * @brief Point of sale that issues and refunds tickets.
 */

namespace museum {

/**
 * @class TicketOffice
 * @brief Sells a limited number of tickets and tracks daily revenue.
 */
class TicketOffice {
    public:
    /**
     * @brief Creates a ticket office.
     * @param dailyCapacity maximum tickets per day.
     * @throws InvalidTicketException when capacity is not positive.
     */
    explicit TicketOffice(int dailyCapacity);

    /**
     * @brief Sells a discounted ticket to a visitor.
     * @param visitor visitor whose ticket type determines the discount.
     * @param type ticket type.
     * @param basePrice base price before the visitor discount.
     * @param day visit day.
     * @return the issued ticket.
     * @throws TicketSoldOutException when the daily capacity is reached.
     */
    Ticket SellTicket(Visitor& visitor, const std::string& type, double basePrice, int day);

    /**
     * @brief Refunds a ticket by identifier.
     * @param ticketId ticket to refund.
     * @return true when the ticket was found and is refundable.
     */
    bool Refund(const std::string& ticketId);

    /**
     * @brief Sums revenue for a specific day.
     * @param day day number.
     * @return revenue from tickets valid on that day.
     */
    double RevenueForDay(int day) const;

    /// @brief Returns the number of sold tickets still held by the office.
    std::size_t TicketsSold() const { return tickets_.size(); }

    /// @brief Returns the total recorded revenue.
    double TotalRevenue() const { return dailyRevenue_; }

    private:
    /// Tickets issued and not refunded.
    std::vector<Ticket> tickets_;
    /// Revenue accumulated from sold tickets.
    double dailyRevenue_ = 0.0;
    /// Maximum tickets the office may sell.
    int dailyCapacity_;
    /// Sequence used to generate ticket identifiers.
    int nextTicketNumber_ = 1;
};

}  // namespace museum
