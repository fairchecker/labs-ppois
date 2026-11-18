#include "logistics/clients.hpp"

#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr double kMaxRating = 5.0;
constexpr double kTierDiscountPercent = 5.0;
constexpr int kPointsPerTier = 100;
constexpr int kMinLoyaltyTier = 1;

}  // namespace

Client::Client(std::string id, std::string companyName, std::string contactPerson,
               double creditLimit, double balance, double rating)
    : id_(std::move(id)),
      companyName_(std::move(companyName)),
      contactPerson_(std::move(contactPerson)),
      creditLimit_(creditLimit),
      balance_(balance),
      rating_(rating) {
    const bool valuesValid = creditLimit_ >= 0.0 && rating_ >= 0.0 && rating_ <= kMaxRating;
    if (id_.empty() || companyName_.empty() || contactPerson_.empty() || !valuesValid) {
        throw InvalidClientException("Client has an invalid identifier, contact or financial value");
    }
}

bool Client::CanAfford(double amount) const {
    return amount >= 0.0 && amount <= balance_ + creditLimit_;
}

void Client::Charge(double amount) {
    if (amount <= 0.0) {
        throw InvalidClientException("Charge amount must be positive");
    }
    if (!CanAfford(amount)) {
        throw InsufficientFundsException("Charge exceeds the client credit limit");
    }
    balance_ -= amount;
}

void Client::TopUp(double amount) {
    if (amount <= 0.0) {
        throw InvalidClientException("Top-up amount must be positive");
    }
    balance_ += amount;
}

void Client::UpdateRating(double rating) {
    if (rating < 0.0 || rating > kMaxRating) {
        throw InvalidClientException("Client rating is out of range");
    }
    rating_ = rating;
}

bool Client::IsReliable() const {
    return rating_ >= kReliableClientRating && balance_ >= 0.0;
}

std::string Client::Describe() const {
    return companyName_ + " (" + id_ + "), balance " + std::to_string(balance_);
}

LoyaltyProgram::LoyaltyProgram(std::string clientId, int tier)
    : clientId_(std::move(clientId)), points_(0), tier_(tier) {
    if (clientId_.empty() || tier_ < kMinLoyaltyTier || tier_ > kMaxLoyaltyTier) {
        throw InvalidClientException("Loyalty account requires a client and a valid tier");
    }
}

void LoyaltyProgram::AddPoints(double amount) {
    if (amount < 0.0) {
        throw InvalidClientException("Loyalty purchase amount cannot be negative");
    }
    const int earned = static_cast<int>(amount * kLoyaltyPointsPerCurrency);
    points_ += earned;
}

void LoyaltyProgram::Redeem(int points) {
    if (points <= 0 || points > points_) {
        throw InvalidClientException("Loyalty redemption exceeds the available points");
    }
    points_ -= points;
}

double LoyaltyProgram::DiscountPercent() const {
    return static_cast<double>(tier_) * kTierDiscountPercent;
}

void LoyaltyProgram::UpgradeTier() {
    const int eligibleTier = (points_ / kPointsPerTier) + kMinLoyaltyTier;
    tier_ = eligibleTier < kMaxLoyaltyTier ? eligibleTier : kMaxLoyaltyTier;
}

CustomerFeedback::CustomerFeedback(std::string id, std::string clientId, std::string orderId,
                                   double rating, std::string comment)
    : id_(std::move(id)),
      clientId_(std::move(clientId)),
      orderId_(std::move(orderId)),
      rating_(rating),
      comment_(std::move(comment)) {
    if (id_.empty() || clientId_.empty() || orderId_.empty() || rating_ < 0.0 ||
        rating_ > kMaxRating) {
        throw InvalidClientException("Feedback has an invalid identifier or rating");
    }
}

bool CustomerFeedback::IsPositive() const { return rating_ >= kPositiveFeedbackRating; }

bool CustomerFeedback::IsComplaint() const { return rating_ <= kComplaintRating; }

std::string CustomerFeedback::Summary() const {
    return std::to_string(rating_) + "/5: " + comment_;
}

}  // namespace logistics
