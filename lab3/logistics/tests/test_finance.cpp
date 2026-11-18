/**
 * @file test_finance.cpp
 * @brief Unit tests for invoicing, payments, tariffs, insurance and payroll.
 */

#include <gtest/gtest.h>

#include "logistics/exceptions.hpp"
#include "logistics/finance.hpp"

namespace logistics {
namespace {

Invoice MakeInvoice() { return Invoice("INV-1", "C1", "O1", 100.0, 0, 10); }

Tariff MakeTariff(const std::string& id = "TR1", const std::string& type = "general") {
    return Tariff(id, type, 2.0, 100.0, 0.5, 10.0);
}

}  // namespace

TEST(InvoiceTest, ValidatesRecord) {
    EXPECT_THROW(Invoice("", "C1", "O1", 100.0, 0, 10), InvalidPaymentException);
    EXPECT_THROW(Invoice("INV", "", "O1", 100.0, 0, 10), InvalidPaymentException);
    EXPECT_THROW(Invoice("INV", "C1", "", 100.0, 0, 10), InvalidPaymentException);
    EXPECT_THROW(Invoice("INV", "C1", "O1", 0.0, 0, 10), InvalidPaymentException);
    EXPECT_THROW(Invoice("INV", "C1", "O1", 100.0, -1, 10), InvalidPaymentException);
    EXPECT_THROW(Invoice("INV", "C1", "O1", 100.0, 5, 4), InvalidPaymentException);
}

TEST(InvoiceTest, TracksDueDateAndPayment) {
    Invoice invoice = MakeInvoice();
    EXPECT_FALSE(invoice.IsOverdue(10));
    EXPECT_TRUE(invoice.IsOverdue(11));
    EXPECT_EQ(invoice.DaysUntilDue(5), 5);
    EXPECT_EQ(invoice.DaysUntilDue(15), 0);
    EXPECT_FALSE(invoice.IsPaid());
    invoice.MarkPaid();
    EXPECT_TRUE(invoice.IsPaid());
    EXPECT_FALSE(invoice.IsOverdue(20));
    EXPECT_THROW(invoice.MarkPaid(), InvalidPaymentException);
    EXPECT_EQ(invoice.Describe(), "INV-1 for C1, 100.000000, paid");
    EXPECT_EQ(invoice.Id(), "INV-1");
    EXPECT_EQ(invoice.ClientId(), "C1");
    EXPECT_EQ(invoice.OrderId(), "O1");
    EXPECT_DOUBLE_EQ(invoice.Amount(), 100.0);
    EXPECT_EQ(invoice.IssuedDay(), 0);
    EXPECT_EQ(invoice.DueDay(), 10);
}

TEST(PaymentTest, ValidatesRecord) {
    EXPECT_THROW(Payment("", "INV", 10.0, PaymentMethod::Card, 0, "REF"), InvalidPaymentException);
    EXPECT_THROW(Payment("P", "", 10.0, PaymentMethod::Card, 0, "REF"), InvalidPaymentException);
    EXPECT_THROW(Payment("P", "INV", 0.0, PaymentMethod::Card, 0, "REF"), InvalidPaymentException);
    EXPECT_THROW(Payment("P", "INV", 10.0, PaymentMethod::Card, -1, "REF"), InvalidPaymentException);
    EXPECT_THROW(Payment("P", "INV", 10.0, PaymentMethod::Card, 0, ""), InvalidPaymentException);
}

TEST(PaymentTest, ChecksAmountAndRefundWindow) {
    const Payment payment("P1", "INV-1", 100.0, PaymentMethod::Card, 5, "REF-1");
    EXPECT_TRUE(payment.IsComplete(100.0));
    EXPECT_TRUE(payment.IsComplete(99.0));
    EXPECT_FALSE(payment.IsComplete(101.0));
    EXPECT_FALSE(payment.IsComplete(0.0));
    EXPECT_FALSE(payment.IsRefundable(4, 5));
    EXPECT_TRUE(payment.IsRefundable(5, 0));
    EXPECT_TRUE(payment.IsRefundable(10, 5));
    EXPECT_FALSE(payment.IsRefundable(11, 5));
    EXPECT_FALSE(payment.IsRefundable(10, -1));
    EXPECT_EQ(payment.MethodName(), "card");
    EXPECT_EQ(payment.Describe(), "P1 for invoice INV-1 by card, 100.000000");
    EXPECT_EQ(payment.Id(), "P1");
    EXPECT_EQ(payment.InvoiceId(), "INV-1");
    EXPECT_DOUBLE_EQ(payment.Amount(), 100.0);
    EXPECT_EQ(payment.Method(), PaymentMethod::Card);
    EXPECT_EQ(payment.PaidDay(), 5);
    EXPECT_EQ(payment.Reference(), "REF-1");
}

TEST(PaymentTest, NamesEveryMethod) {
    EXPECT_EQ(Payment("P1", "I", 1.0, PaymentMethod::BankTransfer, 0, "R").MethodName(),
              "bank transfer");
    EXPECT_EQ(Payment("P2", "I", 1.0, PaymentMethod::Cash, 0, "R").MethodName(), "cash");
    EXPECT_EQ(Payment("P3", "I", 1.0, PaymentMethod::Credit, 0, "R").MethodName(), "credit");
}

TEST(TariffTest, ValidatesRates) {
    EXPECT_THROW(Tariff("", "general", 1.0, 10.0, 1.0, 1.0), InvalidTariffException);
    EXPECT_THROW(Tariff("TR", "", 1.0, 10.0, 1.0, 1.0), InvalidTariffException);
    EXPECT_THROW(Tariff("TR", "general", -1.0, 10.0, 1.0, 1.0), InvalidTariffException);
    EXPECT_THROW(Tariff("TR", "general", 1.0, -1.0, 1.0, 1.0), InvalidTariffException);
    EXPECT_THROW(Tariff("TR", "general", 1.0, 10.0, -1.0, 1.0), InvalidTariffException);
    EXPECT_THROW(Tariff("TR", "general", 1.0, 10.0, 1.0, -1.0), InvalidTariffException);
}

TEST(TariffTest, CalculatesPriceAndSurcharge) {
    const Tariff tariff = MakeTariff();
    EXPECT_DOUBLE_EQ(tariff.Calculate(10.0, 100.0, 1.0), 100.0);
    EXPECT_DOUBLE_EQ(tariff.Calculate(100.0, 100.0, 1.0), 260.0);
    EXPECT_THROW(tariff.Calculate(0.0, 1.0, 1.0), InvalidTariffException);
    EXPECT_THROW(tariff.Calculate(1.0, 0.0, 1.0), InvalidTariffException);
    EXPECT_THROW(tariff.Calculate(1.0, 1.0, 0.0), InvalidTariffException);
    EXPECT_DOUBLE_EQ(tariff.WithFuelSurcharge(10.0), 2.2);
    EXPECT_THROW(tariff.WithFuelSurcharge(-1.0), InvalidTariffException);
    EXPECT_TRUE(tariff.IsApplicable("general"));
    EXPECT_FALSE(tariff.IsApplicable("fragile"));
    EXPECT_EQ(tariff.Id(), "TR1");
    EXPECT_EQ(tariff.CargoType(), "general");
    EXPECT_DOUBLE_EQ(tariff.PricePerKm(), 2.0);
    EXPECT_DOUBLE_EQ(tariff.MinimumCharge(), 100.0);
    EXPECT_DOUBLE_EQ(tariff.WeightRatePerKg(), 0.5);
    EXPECT_DOUBLE_EQ(tariff.VolumeRatePerM3(), 10.0);
}

TEST(PricingEngineTest, RejectsNegativeSurcharge) {
    EXPECT_THROW(PricingEngine(-1.0), InvalidTariffException);
    const PricingEngine engine(5.0);
    EXPECT_DOUBLE_EQ(engine.FuelSurchargePercent(), 5.0);
    EXPECT_DOUBLE_EQ(engine.AverageBaseRate(), 0.0);
    EXPECT_EQ(engine.TariffCount(), 0U);
}

TEST(PricingEngineTest, RegistersAndQuotesTariffs) {
    PricingEngine engine(10.0);
    engine.AddTariff(MakeTariff());
    engine.AddTariff(MakeTariff("TR2", "fragile"));
    EXPECT_EQ(engine.TariffCount(), 2U);
    EXPECT_DOUBLE_EQ(engine.AverageBaseRate(), 2.0);
    EXPECT_THROW(engine.AddTariff(MakeTariff("TR3", "general")), InvalidTariffException);
    EXPECT_EQ(engine.FindFor("general").Id(), "TR1");
    EXPECT_THROW(engine.FindFor("missing"), InvalidTariffException);
    EXPECT_DOUBLE_EQ(engine.Quote("general", 100.0, 100.0, 1.0), 286.0);
    EXPECT_DOUBLE_EQ(engine.Tariffs().size(), 2U);
}

TEST(InsurancePolicyTest, ValidatesRecord) {
    EXPECT_THROW(InsurancePolicy("", "T1", "P", 10.0, 100.0, 10), InsuranceClaimException);
    EXPECT_THROW(InsurancePolicy("I", "", "P", 10.0, 100.0, 10), InsuranceClaimException);
    EXPECT_THROW(InsurancePolicy("I", "T1", "", 10.0, 100.0, 10), InsuranceClaimException);
    EXPECT_THROW(InsurancePolicy("I", "T1", "P", -1.0, 100.0, 10), InsuranceClaimException);
    EXPECT_THROW(InsurancePolicy("I", "T1", "P", 10.0, 0.0, 10), InsuranceClaimException);
    EXPECT_THROW(InsurancePolicy("I", "T1", "P", 10.0, 100.0, -1), InsuranceClaimException);
}

TEST(InsurancePolicyTest, ComputesPremiumAndClaims) {
    const InsurancePolicy policy("I1", "T1", "SafeCo", 100.0, 1000.0, 30);
    EXPECT_DOUBLE_EQ(policy.PremiumFor(3), 300.0);
    EXPECT_THROW(policy.PremiumFor(0), InsuranceClaimException);
    EXPECT_TRUE(policy.Covers(1000.0));
    EXPECT_FALSE(policy.Covers(1001.0));
    EXPECT_FALSE(policy.Covers(-1.0));
    EXPECT_TRUE(policy.IsValidOn(0));
    EXPECT_TRUE(policy.IsValidOn(30));
    EXPECT_FALSE(policy.IsValidOn(31));
    EXPECT_DOUBLE_EQ(policy.ClaimRatio(500.0), 0.5);
    EXPECT_THROW(policy.ClaimRatio(1001.0), InsuranceClaimException);
    EXPECT_EQ(policy.Id(), "I1");
    EXPECT_EQ(policy.VehicleId(), "T1");
    EXPECT_EQ(policy.Provider(), "SafeCo");
    EXPECT_DOUBLE_EQ(policy.MonthlyPremium(), 100.0);
    EXPECT_DOUBLE_EQ(policy.CoverageAmount(), 1000.0);
    EXPECT_EQ(policy.ValidUntilDay(), 30);
}

TEST(ClaimTest, ValidatesAndResolves) {
    const InsurancePolicy policy("I1", "T1", "SafeCo", 100.0, 1000.0, 30);
    EXPECT_THROW(Claim("", "S1", "damage", policy, 10.0, 0), InsuranceClaimException);
    EXPECT_THROW(Claim("CL1", "", "damage", policy, 10.0, 0), InsuranceClaimException);
    EXPECT_THROW(Claim("CL1", "S1", "", policy, 10.0, 0), InsuranceClaimException);
    EXPECT_THROW(Claim("CL1", "S1", "damage", policy, 0.0, 0), InsuranceClaimException);
    EXPECT_THROW(Claim("CL1", "S1", "damage", policy, 10.0, -1), InsuranceClaimException);
    Claim claim("CL1", "S1", "damage", policy, 10.0, 1);
    EXPECT_TRUE(claim.IsOpen());
    EXPECT_TRUE(claim.IsCoveredByPolicy());
    EXPECT_EQ(claim.Policy().Id(), "I1");
    claim.Approve();
    EXPECT_FALSE(claim.IsOpen());
    EXPECT_EQ(claim.Status(), ClaimStatus::Approved);
    EXPECT_THROW(claim.Approve(), InsuranceClaimException);
    EXPECT_THROW(claim.Reject(), InsuranceClaimException);
    Claim rejected("CL2", "S2", "water", policy, 20.0, 2);
    rejected.Reject();
    EXPECT_EQ(rejected.Status(), ClaimStatus::Rejected);
    EXPECT_THROW(rejected.Reject(), InsuranceClaimException);
    EXPECT_EQ(rejected.Describe(), "claim CL2 for shipment S2, amount 20.000000");
    EXPECT_EQ(rejected.Id(), "CL2");
    EXPECT_EQ(rejected.ShipmentId(), "S2");
    EXPECT_EQ(rejected.Reason(), "water");
    EXPECT_DOUBLE_EQ(rejected.Amount(), 20.0);
    EXPECT_EQ(rejected.FiledDay(), 2);
    const Claim uncovered("CL3", "S3", "fire", policy, 2000.0, 3);
    EXPECT_FALSE(uncovered.IsCoveredByPolicy());
}

TEST(CompanyFinanceTest, ValidatesLedger) {
    EXPECT_THROW(CompanyFinance(-1.0, 0.0, 0.0, 20.0), InvalidPaymentException);
    EXPECT_THROW(CompanyFinance(0.0, -1.0, 0.0, 20.0), InvalidPaymentException);
    EXPECT_THROW(CompanyFinance(0.0, 0.0, -1.0, 20.0), InvalidPaymentException);
    EXPECT_THROW(CompanyFinance(0.0, 0.0, 0.0, -1.0), InvalidPaymentException);
    EXPECT_THROW(CompanyFinance(0.0, 0.0, 0.0, 101.0), InvalidPaymentException);
}

TEST(CompanyFinanceTest, TracksProfitAndTax) {
    CompanyFinance finance(1000.0, 0.0, 0.0, 20.0);
    finance.RecordRevenue(100.0);
    EXPECT_DOUBLE_EQ(finance.Balance(), 1100.0);
    EXPECT_DOUBLE_EQ(finance.Revenue(), 100.0);
    finance.RecordExpense(50.0);
    EXPECT_DOUBLE_EQ(finance.Balance(), 1050.0);
    EXPECT_DOUBLE_EQ(finance.Expenses(), 50.0);
    EXPECT_DOUBLE_EQ(finance.GrossProfit(), 50.0);
    EXPECT_DOUBLE_EQ(finance.TaxAmount(), 10.0);
    EXPECT_DOUBLE_EQ(finance.NetProfit(), 40.0);
    EXPECT_TRUE(finance.IsSolvent());
    finance.PayTax();
    EXPECT_DOUBLE_EQ(finance.Balance(), 1040.0);
    EXPECT_DOUBLE_EQ(finance.TaxPaid(), 10.0);
    EXPECT_DOUBLE_EQ(finance.TaxAmount(), 0.0);
    finance.PayTax();
    EXPECT_DOUBLE_EQ(finance.Balance(), 1040.0);
    EXPECT_EQ(finance.TaxRatePercent(), 20.0);
}

TEST(CompanyFinanceTest, RejectsInvalidMovements) {
    CompanyFinance finance(10.0, 0.0, 0.0, 20.0);
    EXPECT_THROW(finance.RecordRevenue(0.0), InvalidPaymentException);
    EXPECT_THROW(finance.RecordExpense(0.0), InvalidPaymentException);
    EXPECT_THROW(finance.RecordExpense(11.0), InsufficientFundsException);
    CompanyFinance poor(0.0, 100.0, 0.0, 20.0);
    EXPECT_FALSE(poor.IsSolvent());
    EXPECT_THROW(poor.PayTax(), InsufficientFundsException);
}

TEST(TaxCalculatorTest, ValidatesAndComputes) {
    EXPECT_THROW(TaxCalculator(-1.0, 0.0), InvalidTariffException);
    EXPECT_THROW(TaxCalculator(101.0, 0.0), InvalidTariffException);
    EXPECT_THROW(TaxCalculator(10.0, -1.0), InvalidTariffException);
    const TaxCalculator calculator(20.0, 200.0);
    EXPECT_DOUBLE_EQ(calculator.TaxFor(1000.0), 160.0);
    EXPECT_DOUBLE_EQ(calculator.TaxFor(100.0), 0.0);
    EXPECT_THROW(calculator.TaxFor(-1.0), InvalidTariffException);
    EXPECT_DOUBLE_EQ(calculator.EffectiveRate(1000.0), 16.0);
    EXPECT_DOUBLE_EQ(calculator.EffectiveRate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(calculator.RatePercent(), 20.0);
    EXPECT_DOUBLE_EQ(calculator.Deduction(), 200.0);
}

TEST(SalaryTest, ValidatesRecord) {
    EXPECT_THROW(Salary("", 100.0, 0.0, 0.0, "2024-01"), InvalidPaymentException);
    EXPECT_THROW(Salary("E1", -1.0, 0.0, 0.0, "2024-01"), InvalidPaymentException);
    EXPECT_THROW(Salary("E1", 100.0, -1.0, 0.0, "2024-01"), InvalidPaymentException);
    EXPECT_THROW(Salary("E1", 100.0, 101.0, 0.0, "2024-01"), InvalidPaymentException);
    EXPECT_THROW(Salary("E1", 100.0, 0.0, -1.0, "2024-01"), InvalidPaymentException);
    EXPECT_THROW(Salary("E1", 100.0, 0.0, 0.0, ""), InvalidPaymentException);
}

TEST(SalaryTest, ComputesNetPay) {
    Salary salary("E1", 1000.0, 10.0, 100.0, "2024-01");
    EXPECT_DOUBLE_EQ(salary.Gross(), 1100.0);
    EXPECT_DOUBLE_EQ(salary.Net(), 1000.0);
    salary.ApplyBonus(20.0);
    EXPECT_DOUBLE_EQ(salary.BonusPercent(), 30.0);
    EXPECT_THROW(salary.ApplyBonus(0.0), InvalidPaymentException);
    EXPECT_THROW(salary.ApplyBonus(71.0), InvalidPaymentException);
    EXPECT_EQ(salary.Describe(), "E1 2024-01, net 1200.000000");
    EXPECT_EQ(salary.EmployeeId(), "E1");
    EXPECT_DOUBLE_EQ(salary.BaseSalary(), 1000.0);
    EXPECT_DOUBLE_EQ(salary.Deduction(), 100.0);
    EXPECT_EQ(salary.PeriodMonth(), "2024-01");
    Salary deducted("E2", 100.0, 0.0, 500.0, "2024-02");
    EXPECT_DOUBLE_EQ(deducted.Net(), 0.0);
}

}  // namespace logistics
