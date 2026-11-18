/**
 * @file test_clients.cpp
 * @brief Unit tests for clients, loyalty and feedback.
 */

#include <gtest/gtest.h>

#include "logistics/clients.hpp"
#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

Client MakeClient() { return Client("C1", "Acme Retail", "Olga", 100.0, 50.0, 4.5); }

}  // namespace

TEST(ClientTest, ValidatesRecord) {
    EXPECT_THROW(Client("", "Acme", "Olga", 0.0, 0.0, 4.0), InvalidClientException);
    EXPECT_THROW(Client("C1", "", "Olga", 0.0, 0.0, 4.0), InvalidClientException);
    EXPECT_THROW(Client("C1", "Acme", "", 0.0, 0.0, 4.0), InvalidClientException);
    EXPECT_THROW(Client("C1", "Acme", "Olga", -1.0, 0.0, 4.0), InvalidClientException);
    EXPECT_THROW(Client("C1", "Acme", "Olga", 0.0, 0.0, -0.1), InvalidClientException);
    EXPECT_THROW(Client("C1", "Acme", "Olga", 0.0, 0.0, 5.1), InvalidClientException);
}

TEST(ClientTest, ChargesWithinCreditAndFunds) {
    Client client = MakeClient();
    EXPECT_TRUE(client.CanAfford(150.0));
    EXPECT_FALSE(client.CanAfford(150.1));
    EXPECT_FALSE(client.CanAfford(-1.0));
    EXPECT_THROW(client.Charge(0.0), InvalidClientException);
    EXPECT_THROW(client.Charge(151.0), InsufficientFundsException);
    client.Charge(120.0);
    EXPECT_DOUBLE_EQ(client.Balance(), -70.0);
    EXPECT_FALSE(client.IsReliable());
    client.TopUp(100.0);
    EXPECT_DOUBLE_EQ(client.Balance(), 30.0);
    EXPECT_THROW(client.TopUp(0.0), InvalidClientException);
    EXPECT_TRUE(client.IsReliable());
    EXPECT_EQ(client.Describe(), "Acme Retail (C1), balance 30.000000");
    EXPECT_EQ(client.Id(), "C1");
    EXPECT_EQ(client.CompanyName(), "Acme Retail");
    EXPECT_EQ(client.ContactPerson(), "Olga");
    EXPECT_DOUBLE_EQ(client.CreditLimit(), 100.0);
    EXPECT_EQ(client.Rating(), 4.5);
}

TEST(ClientTest, UpdatesRating) {
    Client client = MakeClient();
    client.UpdateRating(3.0);
    EXPECT_DOUBLE_EQ(client.Rating(), 3.0);
    EXPECT_THROW(client.UpdateRating(-1.0), InvalidClientException);
    EXPECT_THROW(client.UpdateRating(5.1), InvalidClientException);
}

TEST(LoyaltyProgramTest, AccruesAndRedeemsPoints) {
    EXPECT_THROW(LoyaltyProgram("", 1), InvalidClientException);
    EXPECT_THROW(LoyaltyProgram("C1", 0), InvalidClientException);
    EXPECT_THROW(LoyaltyProgram("C1", 4), InvalidClientException);
    LoyaltyProgram program("C1", 1);
    EXPECT_THROW(program.AddPoints(-1.0), InvalidClientException);
    program.AddPoints(1500.0);
    EXPECT_EQ(program.Points(), 150);
    EXPECT_DOUBLE_EQ(program.DiscountPercent(), 5.0);
    program.UpgradeTier();
    EXPECT_EQ(program.Tier(), 2);
    EXPECT_DOUBLE_EQ(program.DiscountPercent(), 10.0);
    program.AddPoints(1000.0);
    program.UpgradeTier();
    EXPECT_EQ(program.Tier(), 3);
    EXPECT_DOUBLE_EQ(program.DiscountPercent(), 15.0);
    program.UpgradeTier();
    EXPECT_EQ(program.Tier(), 3);
    EXPECT_THROW(program.Redeem(0), InvalidClientException);
    EXPECT_THROW(program.Redeem(251), InvalidClientException);
    program.Redeem(250);
    EXPECT_EQ(program.Points(), 0);
    EXPECT_EQ(program.ClientId(), "C1");
}

TEST(CustomerFeedbackTest, ClassifiesRating) {
    EXPECT_THROW(CustomerFeedback("", "C1", "O1", 4.0, "ok"), InvalidClientException);
    EXPECT_THROW(CustomerFeedback("F1", "", "O1", 4.0, "ok"), InvalidClientException);
    EXPECT_THROW(CustomerFeedback("F1", "C1", "", 4.0, "ok"), InvalidClientException);
    EXPECT_THROW(CustomerFeedback("F1", "C1", "O1", 5.1, "ok"), InvalidClientException);
    const CustomerFeedback positive("F1", "C1", "O1", 5.0, "great");
    const CustomerFeedback complaint("F2", "C2", "O2", 2.0, "late");
    EXPECT_TRUE(positive.IsPositive());
    EXPECT_FALSE(positive.IsComplaint());
    EXPECT_FALSE(complaint.IsPositive());
    EXPECT_TRUE(complaint.IsComplaint());
    EXPECT_EQ(positive.Summary(), "5.000000/5: great");
    EXPECT_EQ(positive.Id(), "F1");
    EXPECT_EQ(positive.ClientId(), "C1");
    EXPECT_EQ(positive.OrderId(), "O1");
    EXPECT_EQ(positive.Comment(), "great");
}

}  // namespace logistics
