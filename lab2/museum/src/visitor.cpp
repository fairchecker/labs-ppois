#include "museum/visitor.hpp"

#include <utility>

#include "museum/exceptions.hpp"
#include "museum/membership.hpp"

namespace museum {

Visitor::Visitor(std::string id, std::string fullName, int age, std::string ticketType,
                 double balance)
    : Person(std::move(id), std::move(fullName), age),
      ticketType_(std::move(ticketType)),
      balance_(balance) {
    if (ticketType_.empty()) {
        throw InvalidPersonException("ticket type must not be empty");
    }
    if (balance_ < 0.0) {
        throw InvalidPersonException("visitor balance must not be negative");
    }
}

std::string Visitor::Role() const { return "visitor"; }

bool Visitor::CanAfford(double amount) const {
    if (amount < 0.0) {
        throw InvalidPersonException("amount must not be negative");
    }
    return balance_ >= amount;
}

void Visitor::TopUp(double amount) {
    if (amount <= 0.0) {
        throw InvalidPersonException("top-up amount must be positive");
    }
    balance_ += amount;
}

double Visitor::ApplyDiscount(double price) const {
    if (price < 0.0) {
        throw InvalidTicketException("ticket price must not be negative");
    }
    if (ticketType_ == "child") {
        return price * (1.0 - kChildDiscount);
    }
    if (ticketType_ == "student") {
        return price * (1.0 - kStudentDiscount);
    }
    if (ticketType_ == "senior") {
        return price * (1.0 - kSeniorDiscount);
    }
    return price;
}

Membership::Membership(std::string visitorId, int level, int validUntilDay)
    : visitorId_(std::move(visitorId)), level_(level), validUntilDay_(validUntilDay) {
    if (visitorId_.empty()) {
        throw InvalidMembershipException("membership visitor id must not be empty");
    }
    if (level_ < 1 || level_ > 3) {
        throw InvalidMembershipException("membership level must be in [1, 3]");
    }
    if (validUntilDay_ < 0) {
        throw InvalidMembershipException("membership validity day must not be negative");
    }
}

bool Membership::IsActive(int day) const { return day <= validUntilDay_; }

double Membership::DiscountPercent() const {
    if (level_ == 1) {
        return kBasicMembershipDiscount;
    }
    if (level_ == 2) {
        return kGoldMembershipDiscount;
    }
    return kPlatinumMembershipDiscount;
}

void Membership::Extend(int days) {
    if (days <= 0) {
        throw InvalidMembershipException("extension must be positive");
    }
    validUntilDay_ += days;
}

bool Membership::BelongsTo(const Visitor& visitor) const { return visitorId_ == visitor.Id(); }

}  // namespace museum
