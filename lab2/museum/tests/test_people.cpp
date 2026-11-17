/**
 * @file test_people.cpp
 * @brief Unit tests for the personnel and membership model.
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "museum/curator.hpp"
#include "museum/employee.hpp"
#include "museum/exceptions.hpp"
#include "museum/guide.hpp"
#include "museum/membership.hpp"
#include "museum/researcher.hpp"
#include "museum/restorer.hpp"
#include "museum/visitor.hpp"

namespace museum {
namespace {

Employee MakeEmployee() {
    return Employee("e1", "Ada Curator", 34, 1000.0, 2020, "dep1");
}

}  // namespace

TEST(PersonTest, RejectsEmptyIdentifierOrName) {
    EXPECT_THROW(Employee("", "Name", 30, 100.0, 2020, "dep"), InvalidPersonException);
    EXPECT_THROW(Employee("id", "", 30, 100.0, 2020, "dep"), InvalidPersonException);
}

TEST(PersonTest, RejectsNegativeAge) {
    EXPECT_THROW(Employee("id", "Name", -1, 100.0, 2020, "dep"), InvalidPersonException);
}

TEST(PersonTest, AdultBoundary) {
    Employee minor("e1", "Young", 17, 100.0, 2020, "dep");
    Employee adult("e2", "Adult", 18, 100.0, 2020, "dep");
    EXPECT_FALSE(minor.IsAdult());
    EXPECT_TRUE(adult.IsAdult());
}

TEST(PersonTest, DescribeIncludesRoleNameAndAge) {
    Employee employee = MakeEmployee();
    EXPECT_EQ(employee.Describe(), "employee: Ada Curator (e1), age 34");
}

TEST(EmployeeTest, AnnualSalaryIsTwelveMonths) {
    Employee employee = MakeEmployee();
    EXPECT_DOUBLE_EQ(employee.AnnualSalary(), 12000.0);
    EXPECT_DOUBLE_EQ(employee.MonthlySalary(), 1000.0);
}

TEST(EmployeeTest, PromoteRaisesSalary) {
    Employee employee = MakeEmployee();
    employee.Promote(10.0);
    EXPECT_DOUBLE_EQ(employee.MonthlySalary(), 1100.0);
}

TEST(EmployeeTest, PromoteRejectsInvalidPercent) {
    Employee employee = MakeEmployee();
    EXPECT_THROW(employee.Promote(0.0), InvalidPersonException);
    EXPECT_THROW(employee.Promote(-5.0), InvalidPersonException);
    EXPECT_THROW(employee.Promote(50.1), InvalidPersonException);
}

TEST(EmployeeTest, RejectsInvalidRecord) {
    EXPECT_THROW(Employee("e", "Name", 30, -1.0, 2020, "dep"), InvalidPersonException);
    EXPECT_THROW(Employee("e", "Name", 30, 100.0, 1800, "dep"), InvalidPersonException);
    EXPECT_THROW(Employee("e", "Name", 30, 100.0, 2020, ""), InvalidPersonException);
}

TEST(EmployeeTest, SeniorityBoundary) {
    Employee employee = MakeEmployee();
    EXPECT_FALSE(employee.IsSenior(2029));
    EXPECT_TRUE(employee.IsSenior(2030));
}

TEST(CuratorTest, AssignsExhibitsWithoutDuplicates) {
    Curator curator("c1", "Curator", 40, 1200.0, 2015, "dep", "Ancient Egypt");
    EXPECT_TRUE(curator.AssignExhibit("ex1"));
    EXPECT_FALSE(curator.AssignExhibit("ex1"));
    EXPECT_TRUE(curator.AssignExhibit("ex2"));
    EXPECT_EQ(curator.ManagedExhibitCount(), 2U);
    EXPECT_EQ(curator.Specialty(), "Ancient Egypt");
    EXPECT_EQ(curator.Role(), "curator");
}

TEST(CuratorTest, RejectsEmptyExhibitId) {
    Curator curator("c1", "Curator", 40, 1200.0, 2015, "dep", "Ancient Egypt");
    EXPECT_THROW(curator.AssignExhibit(""), InvalidExhibitException);
}

TEST(RestorerTest, IdentifiesRestorableCondition) {
    Restorer restorer("r1", "Restorer", 35, 900.0, 2018, "dep", "textiles");
    EXPECT_TRUE(restorer.CanRestore(39));
    EXPECT_FALSE(restorer.CanRestore(40));
    EXPECT_THROW(restorer.CanRestore(-1), InvalidConditionException);
    EXPECT_THROW(restorer.CanRestore(101), InvalidConditionException);
    EXPECT_EQ(restorer.Specialization(), "textiles");
    EXPECT_EQ(restorer.Role(), "restorer");
}

TEST(RestorerTest, CountsCompletedTasks) {
    Restorer restorer("r1", "Restorer", 35, 900.0, 2018, "dep", "textiles");
    restorer.CompleteTask();
    restorer.CompleteTask();
    EXPECT_EQ(restorer.CompletedTasks(), 2);
}

TEST(GuideTest, ManagesLanguages) {
    Guide guide("g1", "Guide", 30, 800.0, 2019, "dep", {"English", "Russian"}, 4.5);
    EXPECT_TRUE(guide.Speaks("English"));
    EXPECT_FALSE(guide.Speaks("French"));
    EXPECT_FALSE(guide.AddLanguage("English"));
    EXPECT_TRUE(guide.AddLanguage("French"));
    EXPECT_EQ(guide.LanguageCount(), 3U);
    EXPECT_THROW(guide.AddLanguage(""), InvalidPersonException);
    EXPECT_EQ(guide.Role(), "guide");
}

TEST(GuideTest, UpdatesRatingWithinRange) {
    Guide guide("g1", "Guide", 30, 800.0, 2019, "dep", {"English"}, 4.0);
    guide.UpdateRating(5.0);
    EXPECT_DOUBLE_EQ(guide.Rating(), 5.0);
    EXPECT_THROW(guide.UpdateRating(0.9), InvalidRatingException);
    EXPECT_THROW(guide.UpdateRating(5.1), InvalidRatingException);
}

TEST(GuideTest, RejectsInvalidInitialRating) {
    EXPECT_THROW(Guide("g1", "Guide", 30, 800.0, 2019, "dep", {"English"}, 6.0),
                 InvalidRatingException);
}

TEST(ResearcherTest, PublishesAndCountsWorks) {
    Researcher researcher("r1", "Researcher", 45, 1100.0, 2010, "dep", "paleontology");
    researcher.Publish("Triassic flora");
    EXPECT_EQ(researcher.PublicationCount(), 1U);
    EXPECT_THROW(researcher.Publish(""), InvalidPersonException);
    EXPECT_EQ(researcher.Field(), "paleontology");
    EXPECT_EQ(researcher.Role(), "researcher");
}

TEST(ResearcherTest, TenureBoundary) {
    Researcher researcher("r1", "Researcher", 45, 1100.0, 2010, "dep", "paleontology");
    EXPECT_FALSE(researcher.HasTenure(2016));
    EXPECT_TRUE(researcher.HasTenure(2017));
}

TEST(VisitorTest, AffordabilityAndTopUp) {
    Visitor visitor("v1", "Visitor", 25, "full", 100.0);
    EXPECT_TRUE(visitor.CanAfford(100.0));
    EXPECT_FALSE(visitor.CanAfford(100.1));
    EXPECT_THROW(visitor.CanAfford(-1.0), InvalidPersonException);
    visitor.TopUp(50.0);
    EXPECT_DOUBLE_EQ(visitor.Balance(), 150.0);
    EXPECT_THROW(visitor.TopUp(0.0), InvalidPersonException);
    EXPECT_EQ(visitor.Role(), "visitor");
}

TEST(VisitorTest, RejectsInvalidRecord) {
    EXPECT_THROW(Visitor("v1", "Visitor", 25, "", 100.0), InvalidPersonException);
    EXPECT_THROW(Visitor("v1", "Visitor", 25, "full", -1.0), InvalidPersonException);
}

TEST(VisitorTest, AppliesTicketTypeDiscounts) {
    Visitor full("v1", "Visitor", 25, "full", 0.0);
    Visitor child("v2", "Visitor", 10, "child", 0.0);
    Visitor student("v3", "Visitor", 20, "student", 0.0);
    Visitor senior("v4", "Visitor", 70, "senior", 0.0);
    EXPECT_DOUBLE_EQ(full.ApplyDiscount(200.0), 200.0);
    EXPECT_DOUBLE_EQ(child.ApplyDiscount(200.0), 100.0);
    EXPECT_DOUBLE_EQ(student.ApplyDiscount(200.0), 140.0);
    EXPECT_DOUBLE_EQ(senior.ApplyDiscount(200.0), 160.0);
    EXPECT_THROW(full.ApplyDiscount(-1.0), InvalidTicketException);
}

TEST(MembershipTest, ActivePeriodAndLateDay) {
    Membership membership("v1", 1, 30);
    EXPECT_TRUE(membership.IsActive(30));
    EXPECT_FALSE(membership.IsActive(31));
    EXPECT_EQ(membership.VisitorId(), "v1");
    Visitor owner("v1", "Owner", 25, "full", 100.0);
    Visitor stranger("v2", "Stranger", 25, "full", 100.0);
    EXPECT_TRUE(membership.BelongsTo(owner));
    EXPECT_FALSE(membership.BelongsTo(stranger));
}

TEST(MembershipTest, DiscountDependsOnLevel) {
    Membership basic("v1", 1, 30);
    Membership gold("v2", 2, 30);
    Membership platinum("v3", 3, 30);
    EXPECT_DOUBLE_EQ(basic.DiscountPercent(), 5.0);
    EXPECT_DOUBLE_EQ(gold.DiscountPercent(), 10.0);
    EXPECT_DOUBLE_EQ(platinum.DiscountPercent(), 15.0);
}

TEST(MembershipTest, ExtendMovesValidity) {
    Membership membership("v1", 2, 30);
    membership.Extend(10);
    EXPECT_EQ(membership.ValidUntilDay(), 40);
    EXPECT_THROW(membership.Extend(0), InvalidMembershipException);
}

TEST(MembershipTest, RejectsInvalidRecords) {
    EXPECT_THROW(Membership("", 1, 30), InvalidMembershipException);
    EXPECT_THROW(Membership("v1", 0, 30), InvalidMembershipException);
    EXPECT_THROW(Membership("v1", 4, 30), InvalidMembershipException);
    EXPECT_THROW(Membership("v1", 1, -1), InvalidMembershipException);
}

TEST(PolymorphismTest, PersonPointerDispatchesRole) {
    const Curator curator("c1", "Curator", 40, 1200.0, 2015, "dep", "Ancient Egypt");
    const Person& person = curator;
    EXPECT_EQ(person.Role(), "curator");
    EXPECT_TRUE(person.IsAdult());
}

}  // namespace museum
