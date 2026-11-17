#include "museum/budget.hpp"

#include <utility>

#include "museum/constants.hpp"
#include "museum/donation.hpp"
#include "museum/exceptions.hpp"
#include "museum/expense.hpp"
#include "museum/grant.hpp"
#include "museum/insurance_policy.hpp"
#include "museum/invoice.hpp"
#include "museum/loan_agreement.hpp"
#include "museum/museum_shop.hpp"

namespace museum {

Budget::Budget(double totalAmount, std::string category)
    : totalAmount_(totalAmount), category_(std::move(category)) {
    if (totalAmount_ < 0.0) {
        throw InvalidBudgetException("budget amount must not be negative");
    }
    if (category_.empty()) {
        throw InvalidBudgetException("budget category must not be empty");
    }
}

void Budget::Allocate(double amount) {
    if (amount <= 0.0) {
        throw InvalidBudgetException("allocation must be positive");
    }
    totalAmount_ += amount;
}

void Budget::Spend(double amount) {
    if (amount <= 0.0) {
        throw InvalidBudgetException("spending amount must be positive");
    }
    if (amount > Remaining()) {
        throw BudgetExceededException("spending exceeds the remaining budget");
    }
    spentAmount_ += amount;
}

double Budget::Remaining() const { return totalAmount_ - spentAmount_; }

bool Budget::IsOverBudget() const { return spentAmount_ > totalAmount_; }

Expense::Expense(std::string id, std::string category, double amount, int day)
    : id_(std::move(id)),
      category_(std::move(category)),
      amount_(amount),
      day_(day),
      status_("pending") {
    if (id_.empty() || category_.empty()) {
        throw InvalidBudgetException("expense id and category must not be empty");
    }
    if (amount_ < 0.0 || day_ < 0) {
        throw InvalidBudgetException("expense amount and day must not be negative");
    }
}

void Expense::Approve() {
    if (!IsPending()) {
        throw InvalidBudgetException("only pending expenses can be approved");
    }
    status_ = "approved";
}

void Expense::Reject() {
    if (!IsPending()) {
        throw InvalidBudgetException("only pending expenses can be rejected");
    }
    status_ = "rejected";
}

Donation::Donation(std::string donorName, double amount, std::string purpose, bool anonymous)
    : donorName_(std::move(donorName)),
      amount_(amount),
      purpose_(std::move(purpose)),
      anonymous_(anonymous) {
    if (donorName_.empty() || purpose_.empty()) {
        throw InvalidBudgetException("donor name and purpose must not be empty");
    }
    if (amount_ <= 0.0) {
        throw InvalidBudgetException("donation amount must be positive");
    }
}

bool Donation::IsLarge() const { return amount_ >= kLargeDonation; }

std::string Donation::DisplayName() const { return anonymous_ ? "Anonymous" : donorName_; }

void Donation::ApplyToBudget(Budget& budget) const { budget.Allocate(amount_); }

ShopItem::ShopItem(std::string name, double price, int stock, std::string category)
    : name_(std::move(name)), price_(price), stock_(stock), category_(std::move(category)) {
    if (name_.empty() || category_.empty()) {
        throw InvalidShopItemException("shop item name and category must not be empty");
    }
    if (price_ < 0.0 || stock_ < 0) {
        throw InvalidShopItemException("shop item price and stock must not be negative");
    }
}

double ShopItem::Sell(int count) {
    if (count <= 0) {
        throw InvalidShopItemException("sale count must be positive");
    }
    if (stock_ < count) {
        throw InvalidShopItemException("insufficient stock for sale");
    }
    stock_ -= count;
    return static_cast<double>(count) * price_;
}

void ShopItem::Restock(int count) {
    if (count <= 0) {
        throw InvalidShopItemException("restock count must be positive");
    }
    stock_ += count;
}

bool ShopItem::IsLowStock() const { return stock_ <= kLowStockLevel; }

bool MuseumShop::AddItem(const ShopItem& item) {
    for (const ShopItem& existing : items_) {
        if (existing.Name() == item.Name()) {
            return false;
        }
    }
    items_.push_back(item);
    return true;
}

double MuseumShop::SellItem(const std::string& name, int count) {
    for (ShopItem& item : items_) {
        if (item.Name() == name) {
            const double revenue = item.Sell(count);
            revenue_ += revenue;
            return revenue;
        }
    }
    throw InvalidShopItemException("unknown shop item: " + name);
}

int MuseumShop::TotalStock() const {
    int total = 0;
    for (const ShopItem& item : items_) {
        total += item.Stock();
    }
    return total;
}

Invoice::Invoice(std::string id, std::string vendor, double amount, int dueDay)
    : id_(std::move(id)), vendor_(std::move(vendor)), amount_(amount), dueDay_(dueDay) {
    if (id_.empty() || vendor_.empty()) {
        throw InvalidBudgetException("invoice id and vendor must not be empty");
    }
    if (amount_ < 0.0 || dueDay_ < 0) {
        throw InvalidBudgetException("invoice amount and due day must not be negative");
    }
}

void Invoice::Pay(double amount) {
    if (amount <= 0.0 || amount > Amount() - paidAmount_) {
        throw InvalidBudgetException("invoice payment is invalid");
    }
    paidAmount_ += amount;
}

bool Invoice::IsOverdue(int currentDay) const {
    return !IsFullyPaid() && currentDay > dueDay_;
}

bool Invoice::IsFullyPaid() const { return paidAmount_ >= amount_; }

Grant::Grant(std::string title, double amount, int deadlineDay)
    : title_(std::move(title)), amount_(amount), deadlineDay_(deadlineDay) {
    if (title_.empty()) {
        throw InvalidBudgetException("grant title must not be empty");
    }
    if (amount_ < 0.0 || deadlineDay_ < 0) {
        throw InvalidBudgetException("grant amount and deadline must not be negative");
    }
}

bool Grant::Apply() { return !awarded_; }

bool Grant::IsExpired(int currentDay) const { return currentDay > deadlineDay_; }

void Grant::Award() {
    if (awarded_) {
        throw InvalidBudgetException("grant already awarded");
    }
    if (amount_ <= 0.0) {
        throw InvalidBudgetException("grant has no value");
    }
    awarded_ = true;
}

LoanAgreement::LoanAgreement(std::string exhibitId, std::string borrower, int startDay,
                             int endDay, double dailyFee)
    : exhibitId_(std::move(exhibitId)),
      borrower_(std::move(borrower)),
      startDay_(startDay),
      endDay_(endDay),
      dailyFee_(dailyFee) {
    if (exhibitId_.empty() || borrower_.empty()) {
        throw LoanException("loan exhibit id and borrower must not be empty");
    }
    if (startDay_ < 0 || endDay_ < startDay_ || dailyFee_ < 0.0) {
        throw LoanException("loan dates or fee are invalid");
    }
}

bool LoanAgreement::IsActiveOn(int day) const { return day >= startDay_ && day <= endDay_; }

double LoanAgreement::CalculateFee() const {
    const int durationDays = endDay_ - startDay_ + 1;
    return static_cast<double>(durationDays) * dailyFee_;
}

void LoanAgreement::Extend(int days) {
    if (days <= 0) {
        throw LoanException("loan extension must be positive");
    }
    endDay_ += days;
}

InsurancePolicy::InsurancePolicy(std::string exhibitId, std::string provider, double premium,
                                 double coverage)
    : exhibitId_(std::move(exhibitId)),
      provider_(std::move(provider)),
      premium_(premium),
      coverage_(coverage) {
    if (exhibitId_.empty() || provider_.empty()) {
        throw InsuranceException("policy exhibit id and provider must not be empty");
    }
    if (premium_ < 0.0 || coverage_ < 0.0) {
        throw InsuranceException("policy premium and coverage must not be negative");
    }
}

bool InsurancePolicy::IsUnderinsured(double exhibitValue) const {
    return coverage_ < exhibitValue;
}

double InsurancePolicy::Claim(double damage) {
    if (damage <= 0.0) {
        throw InsuranceException("claim amount must be positive");
    }
    if (damage > coverage_) {
        throw InsuranceException("claim exceeds the remaining coverage");
    }
    coverage_ -= damage;
    return damage;
}

void InsurancePolicy::Renew(double extraCoverage) {
    if (extraCoverage <= 0.0) {
        throw InsuranceException("extra coverage must be positive");
    }
    coverage_ += extraCoverage;
}

}  // namespace museum
