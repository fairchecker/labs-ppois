#pragma once

#include <string>

/**
 * @file ticket.hpp
 * @brief Admission ticket with price, day and refund rules.
 */

namespace museum {

class Visitor;

/**
 * @class Ticket
 * @brief Single admission ticket sold by the museum.
 */
class Ticket {
    public:
    /**
     * @brief Creates a ticket.
     * @throws InvalidTicketException when fields are empty, the price is negative
     *         or the day is negative.
     */
    Ticket(std::string id, std::string visitorId, std::string type, double price, int day);

    /**
     * @brief Applies a percentage discount to the ticket price.
     * @param percent discount in [0, 100].
     * @return the new price.
     * @throws InvalidTicketException when the percentage is out of range.
     */
    double ApplyDiscount(double percent);

    /**
     * @brief Checks whether the ticket is valid on a day.
     * @param day day number.
     * @return true when the day matches the ticket day.
     */
    bool IsValidOn(int day) const;

    /**
     * @brief Checks whether the ticket can be refunded.
     * @return false for VIP tickets.
     */
    bool IsRefundable() const;

    /**
     * @brief Checks whether the ticket belongs to a visitor.
     * @param visitor visitor to compare.
     * @return true when the visitor identifier matches.
     */
    bool BelongsTo(const Visitor& visitor) const;

    /// @brief Returns the ticket identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the visitor identifier.
    const std::string& VisitorId() const { return visitorId_; }

    /// @brief Returns the ticket type.
    const std::string& Type() const { return type_; }

    /// @brief Returns the current price.
    double Price() const { return price_; }

    /// @brief Returns the visit day.
    int Day() const { return day_; }

    private:
    /// Ticket identifier.
    std::string id_;
    /// Identifier of the visitor who owns the ticket.
    std::string visitorId_;
    /// Ticket type, for example "full" or "vip".
    std::string type_;
    /// Current ticket price.
    double price_;
    /// Day the ticket is valid for.
    int day_;
};

}  // namespace museum
