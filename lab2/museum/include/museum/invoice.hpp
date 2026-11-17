#pragma once

#include <string>

/**
 * @file invoice.hpp
 * @brief Vendor invoice with partial payment support.
 */

namespace museum {

/**
 * @class Invoice
 * @brief Bill that may be paid in several installments.
 */
class Invoice {
    public:
    /**
     * @brief Creates an invoice.
     * @throws InvalidBudgetException when the record is malformed.
     */
    Invoice(std::string id, std::string vendor, double amount, int dueDay);

    /**
     * @brief Pays part of the invoice.
     * @param amount strictly positive payment that does not exceed the outstanding balance.
     * @throws InvalidBudgetException when the payment is invalid.
     */
    void Pay(double amount);

    /**
     * @brief Checks whether the invoice is overdue.
     * @param currentDay current day.
     * @return true when not fully paid and currentDay is after the due day.
     */
    bool IsOverdue(int currentDay) const;

    /// @brief Checks whether the invoice is fully paid.
    bool IsFullyPaid() const;

    /// @brief Returns the invoice identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the vendor name.
    const std::string& Vendor() const { return vendor_; }

    /// @brief Returns the invoice amount.
    double Amount() const { return amount_; }

    /// @brief Returns the paid amount.
    double PaidAmount() const { return paidAmount_; }

    /// @brief Returns the due day.
    int DueDay() const { return dueDay_; }

    private:
    /// Invoice identifier.
    std::string id_;
    /// Vendor name.
    std::string vendor_;
    /// Total amount due.
    double amount_;
    /// Amount already paid.
    double paidAmount_ = 0.0;
    /// Day by which the invoice should be paid.
    int dueDay_;
};

}  // namespace museum
