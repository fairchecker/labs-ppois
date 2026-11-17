#pragma once

#include <string>

#include "museum/person.hpp"

/**
 * @file visitor.hpp
 * @brief Museum visitor with a ticket type and prepaid balance.
 */

namespace museum {

/**
 * @class Visitor
 * @brief Person who buys tickets and books tours.
 */
class Visitor : public Person {
    public:
    /**
     * @brief Creates a visitor.
     * @param ticketType discount category: "full", "child", "student" or "senior".
     * @param balance prepaid balance used to pay for services.
     */
    Visitor(std::string id, std::string fullName, int age, std::string ticketType,
            double balance);

    /// @brief Returns the visitor role.
    std::string Role() const override;

    /**
     * @brief Checks whether the visitor can pay the given amount.
     * @param amount non-negative amount.
     * @return true when balance is sufficient.
     * @throws InvalidPersonException when the amount is negative.
     */
    bool CanAfford(double amount) const;

    /**
     * @brief Adds money to the visitor balance.
     * @param amount strictly positive amount.
     * @throws InvalidPersonException when the amount is not positive.
     */
    void TopUp(double amount);

    /**
     * @brief Applies the ticket-type discount to a price.
     * @param price non-negative base price.
     * @return discounted price.
     * @throws InvalidTicketException when the price is negative.
     */
    double ApplyDiscount(double price) const;

    /// @brief Returns the ticket category.
    const std::string& TicketType() const { return ticketType_; }

    /// @brief Returns the current balance.
    double Balance() const { return balance_; }

    private:
    /// Discount category of the visitor.
    std::string ticketType_;
    /// Prepaid balance.
    double balance_;
};

}  // namespace museum
