#pragma once

#include <string>

/**
 * @file clients.hpp
 * @brief Corporate clients, loyalty and feedback records.
 */

namespace logistics {

/// @brief Rating at or above which a client is considered reliable.
inline constexpr double kReliableClientRating = 4.0;
/// @brief Rating at or above which feedback is positive.
inline constexpr double kPositiveFeedbackRating = 4.0;
/// @brief Rating at or below which feedback is a complaint.
inline constexpr double kComplaintRating = 2.0;
/// @brief Loyalty points earned per currency unit of spend.
inline constexpr double kLoyaltyPointsPerCurrency = 0.1;
/// @brief Maximum loyalty tier.
inline constexpr int kMaxLoyaltyTier = 3;

/**
 * @class Client
 * @brief Company that orders transportation services.
 */
class Client {
    public:
    /**
     * @brief Creates a client.
     * @throws InvalidClientException for an empty required field or invalid limit/rating.
     */
    Client(std::string id, std::string companyName, std::string contactPerson, double creditLimit,
           double balance, double rating);

    /// @brief Returns the client identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the company name.
    const std::string& CompanyName() const { return companyName_; }

    /// @brief Returns the contact person.
    const std::string& ContactPerson() const { return contactPerson_; }

    /// @brief Returns the credit limit.
    double CreditLimit() const { return creditLimit_; }

    /// @brief Returns the current balance, possibly negative when credit is used.
    double Balance() const { return balance_; }

    /// @brief Returns the client rating in [0, 5].
    double Rating() const { return rating_; }

    /**
     * @brief Checks whether the client can cover a charge.
     * @param amount amount to charge.
     */
    bool CanAfford(double amount) const;

    /**
     * @brief Charges the client account.
     * @throws InsufficientFundsException when the credit limit would be exceeded.
     * @throws InvalidClientException when the amount is not positive.
     */
    void Charge(double amount);

    /**
     * @brief Adds funds to the balance.
     * @throws InvalidClientException when the amount is not positive.
     */
    void TopUp(double amount);

    /**
     * @brief Updates the client rating.
     * @throws InvalidClientException when the rating is out of range.
     */
    void UpdateRating(double rating);

    /// @brief Checks whether the client is reliable and has no debt.
    bool IsReliable() const;

    /// @brief Builds a one-line client summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string companyName_;
    std::string contactPerson_;
    double creditLimit_;
    double balance_;
    double rating_;
};

/**
 * @class LoyaltyProgram
 * @brief Accrues points and grants tier-based discounts.
 */
class LoyaltyProgram {
    public:
    /**
     * @brief Creates a loyalty account.
     * @throws InvalidClientException for an empty client identifier or invalid tier.
     */
    LoyaltyProgram(std::string clientId, int tier);

    /// @brief Returns the client identifier.
    const std::string& ClientId() const { return clientId_; }

    /// @brief Returns the current points.
    int Points() const { return points_; }

    /// @brief Returns the loyalty tier.
    int Tier() const { return tier_; }

    /**
     * @brief Accrues points for a payment.
     * @param amount paid amount.
     * @throws InvalidClientException when the amount is negative.
     */
    void AddPoints(double amount);

    /**
     * @brief Redeems points.
     * @throws InvalidClientException when there are not enough points.
     */
    void Redeem(int points);

    /// @brief Returns the discount percentage for the current tier.
    double DiscountPercent() const;

    /// @brief Upgrades the tier when enough points have been collected.
    void UpgradeTier();

    private:
    std::string clientId_;
    int points_;
    int tier_;
};

/**
 * @class CustomerFeedback
 * @brief Rating and comment left after a delivery.
 */
class CustomerFeedback {
    public:
    /**
     * @brief Creates a feedback entry.
     * @throws InvalidClientException for an empty identifier or invalid rating.
     */
    CustomerFeedback(std::string id, std::string clientId, std::string orderId, double rating,
                     std::string comment);

    /// @brief Returns the feedback identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the client identifier.
    const std::string& ClientId() const { return clientId_; }

    /// @brief Returns the reviewed order identifier.
    const std::string& OrderId() const { return orderId_; }

    /// @brief Returns the rating in [0, 5].
    double Rating() const { return rating_; }

    /// @brief Returns the comment text.
    const std::string& Comment() const { return comment_; }

    /// @brief Checks whether the rating is positive.
    bool IsPositive() const;

    /// @brief Checks whether the rating is a complaint.
    bool IsComplaint() const;

    /// @brief Builds a one-line feedback summary.
    std::string Summary() const;

    private:
    std::string id_;
    std::string clientId_;
    std::string orderId_;
    double rating_;
    std::string comment_;
};

}  // namespace logistics
