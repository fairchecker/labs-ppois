#pragma once

#include <string>
#include <vector>

/**
 * @file finance.hpp
 * @brief Invoicing, payments, tariffs, insurance and company finance.
 */

namespace logistics {

/// @brief Payment instrument used by a client.
enum class PaymentMethod { Card, BankTransfer, Cash, Credit };
/// @brief Resolution state of an insurance claim.
enum class ClaimStatus { Open, Approved, Rejected };
/// @brief Percentage scale shared by financial calculations.
inline constexpr double kFinancePercentScale = 100.0;

/**
 * @class Invoice
 * @brief Amount owed by a client for an order.
 */
class Invoice {
    public:
    /**
     * @brief Creates an invoice.
     * @throws InvalidPaymentException for an empty identifier or invalid amount/day.
     */
    Invoice(std::string id, std::string clientId, std::string orderId, double amount,
            int issuedDay, int dueDay);

    /// @brief Returns the invoice identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the client identifier.
    const std::string& ClientId() const { return clientId_; }

    /// @brief Returns the order identifier.
    const std::string& OrderId() const { return orderId_; }

    /// @brief Returns the invoiced amount.
    double Amount() const { return amount_; }

    /// @brief Returns the issue day.
    int IssuedDay() const { return issuedDay_; }

    /// @brief Returns the due day.
    int DueDay() const { return dueDay_; }

    /// @brief Checks whether the invoice was paid.
    bool IsPaid() const { return paid_; }

    /**
     * @brief Checks whether the invoice is unpaid and past due.
     * @param currentDay current calendar day.
     */
    bool IsOverdue(int currentDay) const;

    /**
     * @brief Marks the invoice as paid.
     * @throws InvalidPaymentException when it is already paid.
     */
    void MarkPaid();

    /**
     * @brief Days left before the due date.
     * @param currentDay current calendar day.
     */
    int DaysUntilDue(int currentDay) const;

    /// @brief Builds a one-line invoice summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string clientId_;
    std::string orderId_;
    double amount_;
    int issuedDay_;
    int dueDay_;
    bool paid_;
};

/**
 * @class Payment
 * @brief Money transfer applied to an invoice.
 */
class Payment {
    public:
    /**
     * @brief Creates a payment.
     * @throws InvalidPaymentException for an empty identifier/reference or invalid amount/day.
     */
    Payment(std::string id, std::string invoiceId, double amount, PaymentMethod method, int paidDay,
            std::string reference);

    /// @brief Returns the payment identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the invoice identifier.
    const std::string& InvoiceId() const { return invoiceId_; }

    /// @brief Returns the transferred amount.
    double Amount() const { return amount_; }

    /// @brief Returns the payment method.
    PaymentMethod Method() const { return method_; }

    /// @brief Returns the payment day.
    int PaidDay() const { return paidDay_; }

    /// @brief Returns the bank reference.
    const std::string& Reference() const { return reference_; }

    /**
     * @brief Checks whether the payment covers an expected amount.
     * @param expectedAmount expected invoice amount.
     */
    bool IsComplete(double expectedAmount) const;

    /**
     * @brief Checks whether the payment is still refundable.
     * @param currentDay current calendar day.
     * @param windowDays refund window in days.
     */
    bool IsRefundable(int currentDay, int windowDays) const;

    /// @brief Returns the payment method name.
    std::string MethodName() const;

    /// @brief Builds a one-line payment summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string invoiceId_;
    double amount_;
    PaymentMethod method_;
    int paidDay_;
    std::string reference_;
};

/**
 * @class Tariff
 * @brief Price rules for a cargo type.
 */
class Tariff {
    public:
    /**
     * @brief Creates a tariff.
     * @throws InvalidTariffException for an empty identifier/type or negative rates.
     */
    Tariff(std::string id, std::string cargoType, double pricePerKm, double minimumCharge,
           double weightRatePerKg, double volumeRatePerM3);

    /// @brief Returns the tariff identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the cargo type served by the tariff.
    const std::string& CargoType() const { return cargoType_; }

    /// @brief Returns the price per kilometre.
    double PricePerKm() const { return pricePerKm_; }

    /// @brief Returns the minimum charge.
    double MinimumCharge() const { return minimumCharge_; }

    /// @brief Returns the price per kilogram.
    double WeightRatePerKg() const { return weightRatePerKg_; }

    /// @brief Returns the price per cubic metre.
    double VolumeRatePerM3() const { return volumeRatePerM3_; }

    /**
     * @brief Calculates a shipment price.
     * @param distanceKm route length.
     * @param weightKg cargo weight.
     * @param volumeM3 cargo volume.
     * @throws InvalidTariffException for non-positive measures.
     */
    double Calculate(double distanceKm, double weightKg, double volumeM3) const;

    /**
     * @brief Returns the kilometre rate with a fuel surcharge.
     * @param percent surcharge percentage.
     * @throws InvalidTariffException when the percentage is negative.
     */
    double WithFuelSurcharge(double percent) const;

    /**
     * @brief Checks whether the tariff applies to a cargo type.
     * @param cargoType cargo type name.
     */
    bool IsApplicable(const std::string& cargoType) const;

    private:
    std::string id_;
    std::string cargoType_;
    double pricePerKm_;
    double minimumCharge_;
    double weightRatePerKg_;
    double volumeRatePerM3_;
};

/**
 * @class PricingEngine
 * @brief Selects tariffs and applies the fuel surcharge.
 */
class PricingEngine {
    public:
    /**
     * @brief Creates an empty pricing engine.
     * @throws InvalidTariffException when the fuel surcharge is negative.
     */
    explicit PricingEngine(double fuelSurchargePercent);

    /// @brief Returns the configured fuel surcharge.
    double FuelSurchargePercent() const { return fuelSurchargePercent_; }

    /// @brief Returns the registered tariffs.
    const std::vector<Tariff>& Tariffs() const { return tariffs_; }

    /**
     * @brief Registers a tariff.
     * @throws InvalidTariffException when the identifier or cargo type is already served.
     */
    void AddTariff(const Tariff& tariff);

    /**
     * @brief Finds the tariff for a cargo type.
     * @throws InvalidTariffException when no tariff applies.
     */
    const Tariff& FindFor(const std::string& cargoType) const;

    /**
     * @brief Quotes a shipment price.
     * @throws InvalidTariffException when no tariff applies.
     */
    double Quote(const std::string& cargoType, double distanceKm, double weightKg,
                 double volumeM3) const;

    /// @brief Average kilometre rate across tariffs.
    double AverageBaseRate() const;

    /// @brief Returns the number of tariffs.
    std::size_t TariffCount() const { return tariffs_.size(); }

    private:
    const Tariff* FindForOrNull(const std::string& cargoType) const;

    std::vector<Tariff> tariffs_;
    double fuelSurchargePercent_;
};

/**
 * @class InsurancePolicy
 * @brief Vehicle insurance with premium and coverage.
 */
class InsurancePolicy {
    public:
    /**
     * @brief Creates a policy.
     * @throws InsuranceClaimException for an empty identifier/provider or invalid values.
     */
    InsurancePolicy(std::string id, std::string vehicleId, std::string provider,
                    double monthlyPremium, double coverageAmount, int validUntilDay);

    /// @brief Returns the policy identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the insured vehicle identifier.
    const std::string& VehicleId() const { return vehicleId_; }

    /// @brief Returns the insurance provider.
    const std::string& Provider() const { return provider_; }

    /// @brief Returns the monthly premium.
    double MonthlyPremium() const { return monthlyPremium_; }

    /// @brief Returns the coverage amount.
    double CoverageAmount() const { return coverageAmount_; }

    /// @brief Returns the last covered day.
    int ValidUntilDay() const { return validUntilDay_; }

    /**
     * @brief Premium for a number of months.
     * @throws InsuranceClaimException when the period is not positive.
     */
    double PremiumFor(int months) const;

    /**
     * @brief Checks whether an amount is covered.
     * @param amount claimed amount.
     */
    bool Covers(double amount) const;

    /**
     * @brief Checks whether the policy is valid on a day.
     * @param day calendar day.
     */
    bool IsValidOn(int day) const;

    /**
     * @brief Ratio of a claim to the coverage amount.
     * @throws InsuranceClaimException when the claim exceeds the coverage.
     */
    double ClaimRatio(double amount) const;

    private:
    std::string id_;
    std::string vehicleId_;
    std::string provider_;
    double monthlyPremium_;
    double coverageAmount_;
    int validUntilDay_;
};

/**
 * @class Claim
 * @brief Insurance claim raised for a shipment.
 */
class Claim {
    public:
    /**
     * @brief Creates an open claim.
     * @throws InsuranceClaimException for empty identifiers or invalid amount/day.
     */
    Claim(std::string id, std::string shipmentId, std::string reason, InsurancePolicy policy,
          double amount, int filedDay);

    /// @brief Returns the claim identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the shipment identifier.
    const std::string& ShipmentId() const { return shipmentId_; }

    /// @brief Returns the claim reason.
    const std::string& Reason() const { return reason_; }

    /// @brief Returns the insurance policy backing the claim.
    const InsurancePolicy& Policy() const { return policy_; }

    /// @brief Checks whether the policy covers the claimed amount.
    bool IsCoveredByPolicy() const { return policy_.Covers(amount_); }

    /// @brief Returns the claimed amount.
    double Amount() const { return amount_; }

    /// @brief Returns the claim status.
    ClaimStatus Status() const { return status_; }

    /// @brief Returns the filing day.
    int FiledDay() const { return filedDay_; }

    /// @brief Approves an open claim.
    void Approve();

    /// @brief Rejects an open claim.
    void Reject();

    /// @brief Checks whether the claim is still open.
    bool IsOpen() const { return status_ == ClaimStatus::Open; }

    /// @brief Builds a one-line claim summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string shipmentId_;
    std::string reason_;
    InsurancePolicy policy_;
    double amount_;
    ClaimStatus status_;
    int filedDay_;
};

/**
 * @class CompanyFinance
 * @brief Cash balance, revenue, expenses and tax obligations.
 */
class CompanyFinance {
    public:
    /**
     * @brief Creates the finance ledger.
     * @throws InvalidPaymentException for invalid monetary values or tax rate.
     */
    CompanyFinance(double balance, double revenue, double expenses, double taxRatePercent);

    /// @brief Returns the cash balance.
    double Balance() const { return balance_; }

    /// @brief Returns accumulated revenue.
    double Revenue() const { return revenue_; }

    /// @brief Returns accumulated expenses.
    double Expenses() const { return expenses_; }

    /// @brief Returns the profit tax rate.
    double TaxRatePercent() const { return taxRatePercent_; }

    /// @brief Returns the profit tax already paid.
    double TaxPaid() const { return taxPaid_; }

    /**
     * @brief Records revenue and increases the balance.
     * @throws InvalidPaymentException when the amount is not positive.
     */
    void RecordRevenue(double amount);

    /**
     * @brief Records an expense and decreases the balance.
     * @throws InsufficientFundsException when the balance is too small.
     */
    void RecordExpense(double amount);

    /// @brief Returns revenue minus expenses.
    double GrossProfit() const;

    /// @brief Returns the tax due on the gross profit.
    double TaxAmount() const;

    /// @brief Returns profit after tax.
    double NetProfit() const;

    /**
     * @brief Pays the profit tax.
     * @throws InsufficientFundsException when the balance cannot cover the tax.
     */
    void PayTax();

    /// @brief Checks whether the balance covers the tax due.
    bool IsSolvent() const;

    private:
    double balance_;
    double revenue_;
    double expenses_;
    double taxRatePercent_;
    double taxPaid_;
};

/**
 * @class TaxCalculator
 * @brief Progressive-free income tax with a fixed deduction.
 */
class TaxCalculator {
    public:
    /**
     * @brief Creates a calculator.
     * @throws InvalidTariffException for a negative rate or deduction.
     */
    TaxCalculator(double ratePercent, double deduction);

    /// @brief Returns the tax rate.
    double RatePercent() const { return ratePercent_; }

    /// @brief Returns the fixed deduction.
    double Deduction() const { return deduction_; }

    /**
     * @brief Tax due for an income.
     * @param income gross income.
     * @throws InvalidTariffException when income is negative.
     */
    double TaxFor(double income) const;

    /**
     * @brief Effective tax rate for an income.
     * @param income gross income, zero yields a zero rate.
     */
    double EffectiveRate(double income) const;

    private:
    double ratePercent_;
    double deduction_;
};

/**
 * @class Salary
 * @brief Payroll record for one employee and month.
 */
class Salary {
    public:
    /**
     * @brief Creates a payroll record.
     * @throws InvalidPaymentException for an empty employee/period or invalid values.
     */
    Salary(std::string employeeId, double baseSalary, double bonusPercent, double deduction,
           std::string periodMonth);

    /// @brief Returns the employee identifier.
    const std::string& EmployeeId() const { return employeeId_; }

    /// @brief Returns the base salary.
    double BaseSalary() const { return baseSalary_; }

    /// @brief Returns the bonus percentage.
    double BonusPercent() const { return bonusPercent_; }

    /// @brief Returns the fixed deduction.
    double Deduction() const { return deduction_; }

    /// @brief Returns the payroll month.
    const std::string& PeriodMonth() const { return periodMonth_; }

    /// @brief Returns salary before deduction.
    double Gross() const;

    /// @brief Returns salary after deduction, never negative.
    double Net() const;

    /**
     * @brief Increases the bonus percentage.
     * @throws InvalidPaymentException when the resulting bonus is out of range.
     */
    void ApplyBonus(double percent);

    /// @brief Builds a one-line payroll summary.
    std::string Describe() const;

    private:
    std::string employeeId_;
    double baseSalary_;
    double bonusPercent_;
    double deduction_;
    std::string periodMonth_;
};

}  // namespace logistics
