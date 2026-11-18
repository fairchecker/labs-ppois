#include "logistics/finance.hpp"

#include <algorithm>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr double kPercentScale = kFinancePercentScale;
constexpr double kMinimumMonths = 1.0;
constexpr int kFirstMonth = 1;
constexpr int kFirstDay = 0;
constexpr double kZero = 0.0;
constexpr double kMaxBonusPercent = 100.0;

bool IsNonNegative(double value) { return value >= kZero; }

}  // namespace

Invoice::Invoice(std::string id, std::string clientId, std::string orderId, double amount,
                 int issuedDay, int dueDay)
    : id_(std::move(id)),
      clientId_(std::move(clientId)),
      orderId_(std::move(orderId)),
      amount_(amount),
      issuedDay_(issuedDay),
      dueDay_(dueDay),
      paid_(false) {
    const bool daysValid = issuedDay_ >= kFirstDay && dueDay_ >= issuedDay_;
    if (id_.empty() || clientId_.empty() || orderId_.empty() || amount_ <= 0.0 || !daysValid) {
        throw InvalidPaymentException("Invoice has an invalid identifier, amount or due date");
    }
}

bool Invoice::IsOverdue(int currentDay) const { return !paid_ && currentDay > dueDay_; }

void Invoice::MarkPaid() {
    if (paid_) {
        throw InvalidPaymentException("Invoice is already paid");
    }
    paid_ = true;
}

int Invoice::DaysUntilDue(int currentDay) const { return std::max(0, dueDay_ - currentDay); }

std::string Invoice::Describe() const {
    const std::string state = paid_ ? "paid" : "unpaid";
    return id_ + " for " + clientId_ + ", " + std::to_string(amount_) + ", " + state;
}

Payment::Payment(std::string id, std::string invoiceId, double amount, PaymentMethod method,
                 int paidDay, std::string reference)
    : id_(std::move(id)),
      invoiceId_(std::move(invoiceId)),
      amount_(amount),
      method_(method),
      paidDay_(paidDay),
      reference_(std::move(reference)) {
    if (id_.empty() || invoiceId_.empty() || amount_ <= 0.0 || paidDay_ < kFirstDay ||
        reference_.empty()) {
        throw InvalidPaymentException("Payment has an invalid identifier, amount or day");
    }
}

bool Payment::IsComplete(double expectedAmount) const {
    return expectedAmount > 0.0 && amount_ >= expectedAmount;
}

bool Payment::IsRefundable(int currentDay, int windowDays) const {
    if (windowDays < 0 || currentDay < paidDay_) {
        return false;
    }
    return currentDay - paidDay_ <= windowDays;
}

std::string Payment::MethodName() const {
    switch (method_) {
        case PaymentMethod::Card:
            return "card";
        case PaymentMethod::BankTransfer:
            return "bank transfer";
        case PaymentMethod::Cash:
            return "cash";
        case PaymentMethod::Credit:
            return "credit";
    }
    return "unknown";
}

std::string Payment::Describe() const {
    return id_ + " for invoice " + invoiceId_ + " by " + MethodName() + ", " +
           std::to_string(amount_);
}

Tariff::Tariff(std::string id, std::string cargoType, double pricePerKm, double minimumCharge,
               double weightRatePerKg, double volumeRatePerM3)
    : id_(std::move(id)),
      cargoType_(std::move(cargoType)),
      pricePerKm_(pricePerKm),
      minimumCharge_(minimumCharge),
      weightRatePerKg_(weightRatePerKg),
      volumeRatePerM3_(volumeRatePerM3) {
    const bool ratesValid = IsNonNegative(pricePerKm_) && IsNonNegative(minimumCharge_) &&
                            IsNonNegative(weightRatePerKg_) && IsNonNegative(volumeRatePerM3_);
    if (id_.empty() || cargoType_.empty() || !ratesValid) {
        throw InvalidTariffException("Tariff requires an identifier, cargo type and non-negative rates");
    }
}

double Tariff::Calculate(double distanceKm, double weightKg, double volumeM3) const {
    if (distanceKm <= 0.0 || weightKg <= 0.0 || volumeM3 <= 0.0) {
        throw InvalidTariffException("Tariff measures must be positive");
    }
    const double distanceCost = distanceKm * pricePerKm_;
    const double weightCost = weightKg * weightRatePerKg_;
    const double volumeCost = volumeM3 * volumeRatePerM3_;
    return std::max(minimumCharge_, distanceCost + weightCost + volumeCost);
}

double Tariff::WithFuelSurcharge(double percent) const {
    if (percent < 0.0) {
        throw InvalidTariffException("Fuel surcharge cannot be negative");
    }
    return pricePerKm_ * (1.0 + percent / kPercentScale);
}

bool Tariff::IsApplicable(const std::string& cargoType) const { return cargoType_ == cargoType; }

PricingEngine::PricingEngine(double fuelSurchargePercent)
    : fuelSurchargePercent_(fuelSurchargePercent) {
    if (fuelSurchargePercent_ < 0.0) {
        throw InvalidTariffException("Fuel surcharge cannot be negative");
    }
}

const Tariff* PricingEngine::FindForOrNull(const std::string& cargoType) const {
    for (const Tariff& tariff : tariffs_) {
        if (tariff.IsApplicable(cargoType)) {
            return &tariff;
        }
    }
    return nullptr;
}

void PricingEngine::AddTariff(const Tariff& tariff) {
    if (FindForOrNull(tariff.CargoType()) != nullptr) {
        throw InvalidTariffException("A tariff for this cargo type already exists");
    }
    tariffs_.push_back(tariff);
}

const Tariff& PricingEngine::FindFor(const std::string& cargoType) const {
    const Tariff* tariff = FindForOrNull(cargoType);
    if (tariff == nullptr) {
        throw InvalidTariffException("No tariff for cargo type " + cargoType);
    }
    return *tariff;
}

double PricingEngine::Quote(const std::string& cargoType, double distanceKm, double weightKg,
                            double volumeM3) const {
    const Tariff& tariff = FindFor(cargoType);
    const double basePrice = tariff.Calculate(distanceKm, weightKg, volumeM3);
    return basePrice * (1.0 + fuelSurchargePercent_ / kPercentScale);
}

double PricingEngine::AverageBaseRate() const {
    if (tariffs_.empty()) {
        return 0.0;
    }
    double total = 0.0;
    for (const Tariff& tariff : tariffs_) {
        total += tariff.PricePerKm();
    }
    return total / static_cast<double>(tariffs_.size());
}

InsurancePolicy::InsurancePolicy(std::string id, std::string vehicleId, std::string provider,
                                 double monthlyPremium, double coverageAmount, int validUntilDay)
    : id_(std::move(id)),
      vehicleId_(std::move(vehicleId)),
      provider_(std::move(provider)),
      monthlyPremium_(monthlyPremium),
      coverageAmount_(coverageAmount),
      validUntilDay_(validUntilDay) {
    const bool valuesValid = monthlyPremium_ >= 0.0 && coverageAmount_ > 0.0 && validUntilDay_ >= 0;
    if (id_.empty() || vehicleId_.empty() || provider_.empty() || !valuesValid) {
        throw InsuranceClaimException("Insurance policy has an invalid identifier or value");
    }
}

double InsurancePolicy::PremiumFor(int months) const {
    if (months < static_cast<int>(kMinimumMonths)) {
        throw InsuranceClaimException("Insurance period must be at least one month");
    }
    return monthlyPremium_ * static_cast<double>(months);
}

bool InsurancePolicy::Covers(double amount) const {
    return amount >= 0.0 && amount <= coverageAmount_;
}

bool InsurancePolicy::IsValidOn(int day) const { return day >= kFirstDay && day <= validUntilDay_; }

double InsurancePolicy::ClaimRatio(double amount) const {
    if (!Covers(amount)) {
        throw InsuranceClaimException("Claim amount exceeds the policy coverage");
    }
    return amount / coverageAmount_;
}

Claim::Claim(std::string id, std::string shipmentId, std::string reason, InsurancePolicy policy,
             double amount, int filedDay)
    : id_(std::move(id)),
      shipmentId_(std::move(shipmentId)),
      reason_(std::move(reason)),
      policy_(std::move(policy)),
      amount_(amount),
      status_(ClaimStatus::Open),
      filedDay_(filedDay) {
    if (id_.empty() || shipmentId_.empty() || reason_.empty() || amount_ <= 0.0 ||
        filedDay_ < kFirstDay) {
        throw InsuranceClaimException("Claim has an invalid identifier, reason, amount or day");
    }
}

void Claim::Approve() {
    if (!IsOpen()) {
        throw InsuranceClaimException("Only an open claim can be approved");
    }
    status_ = ClaimStatus::Approved;
}

void Claim::Reject() {
    if (!IsOpen()) {
        throw InsuranceClaimException("Only an open claim can be rejected");
    }
    status_ = ClaimStatus::Rejected;
}

std::string Claim::Describe() const {
    return "claim " + id_ + " for shipment " + shipmentId_ + ", amount " + std::to_string(amount_);
}

CompanyFinance::CompanyFinance(double balance, double revenue, double expenses,
                               double taxRatePercent)
    : balance_(balance),
      revenue_(revenue),
      expenses_(expenses),
      taxRatePercent_(taxRatePercent),
      taxPaid_(0.0) {
    const bool valuesValid = balance_ >= 0.0 && revenue_ >= 0.0 && expenses_ >= 0.0 &&
                             taxRatePercent_ >= 0.0 && taxRatePercent_ <= kPercentScale;
    if (!valuesValid) {
        throw InvalidPaymentException("Company finance has invalid monetary values");
    }
}

void CompanyFinance::RecordRevenue(double amount) {
    if (amount <= 0.0) {
        throw InvalidPaymentException("Revenue must be positive");
    }
    balance_ += amount;
    revenue_ += amount;
}

void CompanyFinance::RecordExpense(double amount) {
    if (amount <= 0.0) {
        throw InvalidPaymentException("Expense must be positive");
    }
    if (amount > balance_) {
        throw InsufficientFundsException("Expense exceeds the company balance");
    }
    balance_ -= amount;
    expenses_ += amount;
}

double CompanyFinance::GrossProfit() const { return revenue_ - expenses_; }

double CompanyFinance::TaxAmount() const {
    const double taxable = std::max(0.0, GrossProfit());
    const double accrued = taxable * taxRatePercent_ / kPercentScale;
    return std::max(0.0, accrued - taxPaid_);
}

double CompanyFinance::NetProfit() const { return GrossProfit() - taxPaid_ - TaxAmount(); }

void CompanyFinance::PayTax() {
    const double tax = TaxAmount();
    if (tax <= 0.0) {
        return;
    }
    if (tax > balance_) {
        throw InsufficientFundsException("Balance cannot cover the profit tax");
    }
    balance_ -= tax;
    taxPaid_ += tax;
}

bool CompanyFinance::IsSolvent() const { return balance_ >= TaxAmount(); }

TaxCalculator::TaxCalculator(double ratePercent, double deduction)
    : ratePercent_(ratePercent), deduction_(deduction) {
    if (ratePercent_ < 0.0 || ratePercent_ > kPercentScale || deduction_ < 0.0) {
        throw InvalidTariffException("Tax calculator requires a valid rate and deduction");
    }
}

double TaxCalculator::TaxFor(double income) const {
    if (income < 0.0) {
        throw InvalidTariffException("Income cannot be negative");
    }
    const double taxable = std::max(0.0, income - deduction_);
    return taxable * ratePercent_ / kPercentScale;
}

double TaxCalculator::EffectiveRate(double income) const {
    if (income <= 0.0) {
        return 0.0;
    }
    return TaxFor(income) / income * kPercentScale;
}

Salary::Salary(std::string employeeId, double baseSalary, double bonusPercent, double deduction,
               std::string periodMonth)
    : employeeId_(std::move(employeeId)),
      baseSalary_(baseSalary),
      bonusPercent_(bonusPercent),
      deduction_(deduction),
      periodMonth_(std::move(periodMonth)) {
    const bool valuesValid = baseSalary_ >= 0.0 && bonusPercent_ >= 0.0 &&
                             bonusPercent_ <= kMaxBonusPercent && deduction_ >= 0.0;
    if (employeeId_.empty() || periodMonth_.empty() || !valuesValid) {
        throw InvalidPaymentException("Salary has an invalid employee, period or value");
    }
}

double Salary::Gross() const { return baseSalary_ * (1.0 + bonusPercent_ / kPercentScale); }

double Salary::Net() const {
    const double gross = Gross();
    return std::max(0.0, gross - deduction_);
}

void Salary::ApplyBonus(double percent) {
    const double updated = bonusPercent_ + percent;
    if (percent <= 0.0 || updated > kMaxBonusPercent) {
        throw InvalidPaymentException("Updated bonus percentage is out of range");
    }
    bonusPercent_ = updated;
}

std::string Salary::Describe() const {
    return employeeId_ + " " + periodMonth_ + ", net " + std::to_string(Net());
}

}  // namespace logistics
