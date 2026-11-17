/**
 * @file test_finance.cpp
 * @brief Unit tests for budgets, shops, invoices, grants, loans and insurance.
 */

#include <gtest/gtest.h>

#include "museum/budget.hpp"
#include "museum/donation.hpp"
#include "museum/expense.hpp"
#include "museum/exceptions.hpp"
#include "museum/grant.hpp"
#include "museum/insurance_policy.hpp"
#include "museum/invoice.hpp"
#include "museum/loan_agreement.hpp"
#include "museum/museum_shop.hpp"
#include "museum/shop_item.hpp"

namespace museum {

TEST(BudgetTest, AllocatesAndSpends) {
    Budget budget(1000.0, "restoration");
    EXPECT_DOUBLE_EQ(budget.Remaining(), 1000.0);
    budget.Allocate(500.0);
    EXPECT_DOUBLE_EQ(budget.Total(), 1500.0);
    budget.Spend(400.0);
    EXPECT_DOUBLE_EQ(budget.Spent(), 400.0);
    EXPECT_DOUBLE_EQ(budget.Remaining(), 1100.0);
    EXPECT_FALSE(budget.IsOverBudget());
    EXPECT_EQ(budget.Category(), "restoration");
}

TEST(BudgetTest, RejectsOverspendingAndInvalidAmounts) {
    Budget budget(100.0, "events");
    EXPECT_THROW(budget.Spend(100.1), BudgetExceededException);
    EXPECT_THROW(budget.Spend(0.0), InvalidBudgetException);
    EXPECT_THROW(budget.Allocate(0.0), InvalidBudgetException);
    EXPECT_THROW(Budget(-1.0, "events"), InvalidBudgetException);
    EXPECT_THROW(Budget(10.0, ""), InvalidBudgetException);
}

TEST(ExpenseTest, ApprovesOrRejectsPendingRequest) {
    Expense approved("e1", "travel", 250.0, 5);
    EXPECT_TRUE(approved.IsPending());
    approved.Approve();
    EXPECT_EQ(approved.Status(), "approved");
    EXPECT_FALSE(approved.IsPending());
    EXPECT_THROW(approved.Approve(), InvalidBudgetException);
    EXPECT_THROW(approved.Reject(), InvalidBudgetException);

    Expense rejected("e2", "travel", 250.0, 5);
    rejected.Reject();
    EXPECT_EQ(rejected.Status(), "rejected");
    EXPECT_THROW(rejected.Approve(), InvalidBudgetException);
    EXPECT_EQ(approved.Id(), "e1");
    EXPECT_EQ(rejected.Id(), "e2");
    EXPECT_EQ(approved.Category(), "travel");
    EXPECT_DOUBLE_EQ(approved.Amount(), 250.0);
    EXPECT_EQ(approved.Day(), 5);
}

TEST(ExpenseTest, RejectsInvalidData) {
    EXPECT_THROW(Expense("", "travel", 10.0, 1), InvalidBudgetException);
    EXPECT_THROW(Expense("e", "", 10.0, 1), InvalidBudgetException);
    EXPECT_THROW(Expense("e", "travel", -1.0, 1), InvalidBudgetException);
    EXPECT_THROW(Expense("e", "travel", 10.0, -1), InvalidBudgetException);
}

TEST(DonationTest, AppliesToBudgetAndRespectsAnonymity) {
    Budget budget(100.0, "collection");
    Donation large("Patron", 10000.0, "new wing", false);
    Donation anonymous("Hidden", 50.0, "shop", true);
    EXPECT_TRUE(large.IsLarge());
    EXPECT_FALSE(anonymous.IsLarge());
    EXPECT_EQ(large.DisplayName(), "Patron");
    EXPECT_EQ(anonymous.DisplayName(), "Anonymous");
    large.ApplyToBudget(budget);
    EXPECT_DOUBLE_EQ(budget.Total(), 10100.0);
    EXPECT_DOUBLE_EQ(anonymous.Amount(), 50.0);
    EXPECT_EQ(anonymous.Purpose(), "shop");
    EXPECT_TRUE(anonymous.IsAnonymous());
}

TEST(DonationTest, RejectsInvalidData) {
    EXPECT_THROW(Donation("", 10.0, "purpose", false), InvalidBudgetException);
    EXPECT_THROW(Donation("Donor", 10.0, "", false), InvalidBudgetException);
    EXPECT_THROW(Donation("Donor", 0.0, "purpose", false), InvalidBudgetException);
}

TEST(ShopItemTest, SellsRestocksAndReportsLowStock) {
    ShopItem item("Postcard", 2.5, 10, "souvenir");
    EXPECT_FALSE(item.IsLowStock());
    EXPECT_DOUBLE_EQ(item.Sell(4), 10.0);
    EXPECT_EQ(item.Stock(), 6);
    EXPECT_FALSE(item.IsLowStock());
    item.Restock(4);
    EXPECT_EQ(item.Stock(), 10);
    EXPECT_THROW(item.Sell(11), InvalidShopItemException);
    EXPECT_THROW(item.Sell(0), InvalidShopItemException);
    EXPECT_THROW(item.Restock(0), InvalidShopItemException);
    EXPECT_EQ(item.Name(), "Postcard");
    EXPECT_DOUBLE_EQ(item.Price(), 2.5);
    EXPECT_EQ(item.Category(), "souvenir");
    ShopItem low("Magnet", 1.0, 5, "souvenir");
    EXPECT_TRUE(low.IsLowStock());
}

TEST(ShopItemTest, RejectsInvalidData) {
    EXPECT_THROW(ShopItem("", 1.0, 1, "cat"), InvalidShopItemException);
    EXPECT_THROW(ShopItem("Item", 1.0, 1, ""), InvalidShopItemException);
    EXPECT_THROW(ShopItem("Item", -1.0, 1, "cat"), InvalidShopItemException);
    EXPECT_THROW(ShopItem("Item", 1.0, -1, "cat"), InvalidShopItemException);
}

TEST(MuseumShopTest, ManagesCatalogAndRevenue) {
    MuseumShop shop;
    EXPECT_TRUE(shop.AddItem(ShopItem("Guide", 10.0, 5, "book")));
    EXPECT_FALSE(shop.AddItem(ShopItem("Guide", 10.0, 5, "book")));
    EXPECT_TRUE(shop.AddItem(ShopItem("Postcard", 2.0, 10, "souvenir")));
    EXPECT_EQ(shop.ItemCount(), 2U);
    EXPECT_EQ(shop.TotalStock(), 15);
    EXPECT_DOUBLE_EQ(shop.SellItem("Guide", 2), 20.0);
    EXPECT_DOUBLE_EQ(shop.Revenue(), 20.0);
    EXPECT_EQ(shop.TotalStock(), 13);
    EXPECT_THROW(shop.SellItem("Unknown", 1), InvalidShopItemException);
    EXPECT_THROW(shop.SellItem("Guide", 100), InvalidShopItemException);
}

TEST(InvoiceTest, TracksPartialAndFullPayments) {
    Invoice invoice("i1", "Paper Co", 100.0, 10);
    EXPECT_TRUE(invoice.IsOverdue(11));
    invoice.Pay(40.0);
    EXPECT_FALSE(invoice.IsFullyPaid());
    EXPECT_DOUBLE_EQ(invoice.PaidAmount(), 40.0);
    invoice.Pay(60.0);
    EXPECT_TRUE(invoice.IsFullyPaid());
    EXPECT_FALSE(invoice.IsOverdue(20));
    EXPECT_THROW(invoice.Pay(1.0), InvalidBudgetException);
    EXPECT_THROW(invoice.Pay(0.0), InvalidBudgetException);
    EXPECT_EQ(invoice.Id(), "i1");
    EXPECT_EQ(invoice.Vendor(), "Paper Co");
    EXPECT_DOUBLE_EQ(invoice.Amount(), 100.0);
    EXPECT_EQ(invoice.DueDay(), 10);
}

TEST(InvoiceTest, RejectsInvalidData) {
    EXPECT_THROW(Invoice("", "Vendor", 10.0, 1), InvalidBudgetException);
    EXPECT_THROW(Invoice("i", "", 10.0, 1), InvalidBudgetException);
    EXPECT_THROW(Invoice("i", "Vendor", -1.0, 1), InvalidBudgetException);
    EXPECT_THROW(Invoice("i", "Vendor", 10.0, -1), InvalidBudgetException);
}

TEST(GrantTest, AppliesAwardsAndExpires) {
    Grant grant("Digitization", 5000.0, 30);
    EXPECT_TRUE(grant.Apply());
    EXPECT_FALSE(grant.IsExpired(30));
    EXPECT_TRUE(grant.IsExpired(31));
    grant.Award();
    EXPECT_TRUE(grant.IsAwarded());
    EXPECT_FALSE(grant.Apply());
    EXPECT_THROW(grant.Award(), InvalidBudgetException);
    EXPECT_EQ(grant.Title(), "Digitization");
    EXPECT_DOUBLE_EQ(grant.Amount(), 5000.0);
    EXPECT_EQ(grant.DeadlineDay(), 30);
}

TEST(GrantTest, RejectsInvalidData) {
    EXPECT_THROW(Grant("", 10.0, 1), InvalidBudgetException);
    EXPECT_THROW(Grant("Grant", -1.0, 1), InvalidBudgetException);
    EXPECT_THROW(Grant("Grant", 10.0, -1), InvalidBudgetException);
    Grant emptyValue("Empty", 0.0, 1);
    EXPECT_THROW(emptyValue.Award(), InvalidBudgetException);
}

TEST(LoanAgreementTest, ComputesFeesAndExtends) {
    LoanAgreement loan("ex1", "City Gallery", 10, 20, 5.0);
    EXPECT_TRUE(loan.IsActiveOn(10));
    EXPECT_TRUE(loan.IsActiveOn(20));
    EXPECT_FALSE(loan.IsActiveOn(9));
    EXPECT_FALSE(loan.IsActiveOn(21));
    EXPECT_DOUBLE_EQ(loan.CalculateFee(), 55.0);
    loan.Extend(10);
    EXPECT_EQ(loan.EndDay(), 30);
    EXPECT_DOUBLE_EQ(loan.CalculateFee(), 105.0);
    EXPECT_THROW(loan.Extend(0), LoanException);
    EXPECT_EQ(loan.ExhibitId(), "ex1");
    EXPECT_EQ(loan.Borrower(), "City Gallery");
    EXPECT_EQ(loan.StartDay(), 10);
    EXPECT_DOUBLE_EQ(loan.DailyFee(), 5.0);
}

TEST(LoanAgreementTest, RejectsInvalidData) {
    EXPECT_THROW(LoanAgreement("", "Borrower", 1, 2, 1.0), LoanException);
    EXPECT_THROW(LoanAgreement("ex", "", 1, 2, 1.0), LoanException);
    EXPECT_THROW(LoanAgreement("ex", "Borrower", -1, 2, 1.0), LoanException);
    EXPECT_THROW(LoanAgreement("ex", "Borrower", 5, 2, 1.0), LoanException);
    EXPECT_THROW(LoanAgreement("ex", "Borrower", 1, 2, -1.0), LoanException);
}

TEST(InsurancePolicyTest, HandlesClaimsAndRenewal) {
    InsurancePolicy policy("ex1", "SafeCo", 200.0, 10000.0);
    EXPECT_TRUE(policy.IsUnderinsured(20000.0));
    EXPECT_FALSE(policy.IsUnderinsured(5000.0));
    EXPECT_DOUBLE_EQ(policy.Claim(4000.0), 4000.0);
    EXPECT_DOUBLE_EQ(policy.Coverage(), 6000.0);
    policy.Renew(1000.0);
    EXPECT_DOUBLE_EQ(policy.Coverage(), 7000.0);
    EXPECT_THROW(policy.Claim(7000.1), InsuranceException);
    EXPECT_THROW(policy.Claim(0.0), InsuranceException);
    EXPECT_THROW(policy.Renew(0.0), InsuranceException);
    EXPECT_EQ(policy.ExhibitId(), "ex1");
    EXPECT_EQ(policy.Provider(), "SafeCo");
    EXPECT_DOUBLE_EQ(policy.Premium(), 200.0);
}

TEST(InsurancePolicyTest, RejectsInvalidData) {
    EXPECT_THROW(InsurancePolicy("", "SafeCo", 10.0, 100.0), InsuranceException);
    EXPECT_THROW(InsurancePolicy("ex", "", 10.0, 100.0), InsuranceException);
    EXPECT_THROW(InsurancePolicy("ex", "SafeCo", -1.0, 100.0), InsuranceException);
    EXPECT_THROW(InsurancePolicy("ex", "SafeCo", 10.0, -1.0), InsuranceException);
}

}  // namespace museum
