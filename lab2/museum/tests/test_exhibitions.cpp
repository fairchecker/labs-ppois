/**
 * @file test_exhibitions.cpp
 * @brief Unit tests for exhibitions, tickets, tours, groups and events.
 */

#include <gtest/gtest.h>

#include <string>

#include "museum/artifact.hpp"
#include "museum/audio_guide.hpp"
#include "museum/event.hpp"
#include "museum/exceptions.hpp"
#include "museum/exhibition.hpp"
#include "museum/guide.hpp"
#include "museum/ticket.hpp"
#include "museum/ticket_office.hpp"
#include "museum/tour.hpp"
#include "museum/tour_booking.hpp"
#include "museum/visitor.hpp"
#include "museum/visitor_group.hpp"

namespace museum {
namespace {

Visitor MakeVisitor(const std::string& id = "v1", double balance = 500.0) {
    return Visitor(id, "Visitor", 25, "full", balance);
}

}  // namespace

TEST(ExhibitionTest, ManagesDatesAndExhibits) {
    Exhibition exhibition("ex1", "Ancient Worlds", 10, 20, 50.0);
    EXPECT_TRUE(exhibition.IsActiveOn(10));
    EXPECT_TRUE(exhibition.IsActiveOn(15));
    EXPECT_TRUE(exhibition.IsActiveOn(20));
    EXPECT_FALSE(exhibition.IsActiveOn(9));
    EXPECT_FALSE(exhibition.IsActiveOn(21));
    EXPECT_EQ(exhibition.DurationDays(), 11);
    EXPECT_DOUBLE_EQ(exhibition.ExpectedRevenue(10), 500.0);
    EXPECT_TRUE(exhibition.AddExhibit("a1"));
    EXPECT_FALSE(exhibition.AddExhibit("a1"));
    Artifact artifact("a2", "Bowl", "Ancient", 70, 100.0, "bronze", 1.0, "Greece");
    EXPECT_TRUE(exhibition.AddExhibit(artifact));
    EXPECT_FALSE(exhibition.AddExhibit(artifact));
    EXPECT_EQ(exhibition.ExhibitCount(), 2U);
    EXPECT_EQ(exhibition.Id(), "ex1");
    EXPECT_EQ(exhibition.Title(), "Ancient Worlds");
    EXPECT_EQ(exhibition.StartDay(), 10);
    EXPECT_EQ(exhibition.EndDay(), 20);
    EXPECT_DOUBLE_EQ(exhibition.TicketPrice(), 50.0);
}

TEST(ExhibitionTest, RejectsInvalidData) {
    EXPECT_THROW(Exhibition("", "Title", 1, 2, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Exhibition("id", "", 1, 2, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Exhibition("id", "Title", -1, 2, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Exhibition("id", "Title", 5, 2, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Exhibition("id", "Title", 1, 2, -1.0), InvalidExhibitionException);
    Exhibition exhibition("id", "Title", 1, 2, 10.0);
    EXPECT_THROW(exhibition.AddExhibit(""), InvalidExhibitException);
    EXPECT_THROW(exhibition.ExpectedRevenue(-1), InvalidExhibitionException);
}

TEST(TicketTest, DiscountsAndValidity) {
    Ticket ticket("t1", "v1", "full", 100.0, 5);
    EXPECT_DOUBLE_EQ(ticket.ApplyDiscount(25.0), 75.0);
    EXPECT_DOUBLE_EQ(ticket.Price(), 75.0);
    EXPECT_TRUE(ticket.IsValidOn(5));
    EXPECT_FALSE(ticket.IsValidOn(6));
    EXPECT_TRUE(ticket.IsRefundable());
    EXPECT_EQ(ticket.Id(), "t1");
    EXPECT_EQ(ticket.VisitorId(), "v1");
    EXPECT_EQ(ticket.Type(), "full");
    EXPECT_EQ(ticket.Day(), 5);
    Visitor owner("v1", "Owner", 25, "full", 100.0);
    Visitor stranger("v2", "Stranger", 25, "full", 100.0);
    EXPECT_TRUE(ticket.BelongsTo(owner));
    EXPECT_FALSE(ticket.BelongsTo(stranger));
    EXPECT_THROW(ticket.ApplyDiscount(-1.0), InvalidTicketException);
    EXPECT_THROW(ticket.ApplyDiscount(101.0), InvalidTicketException);
}

TEST(TicketTest, VipTicketsAreNotRefundable) {
    Ticket ticket("t1", "v1", "vip", 100.0, 5);
    EXPECT_FALSE(ticket.IsRefundable());
}

TEST(TicketTest, RejectsInvalidData) {
    EXPECT_THROW(Ticket("", "v1", "full", 10.0, 1), InvalidTicketException);
    EXPECT_THROW(Ticket("t1", "", "full", 10.0, 1), InvalidTicketException);
    EXPECT_THROW(Ticket("t1", "v1", "", 10.0, 1), InvalidTicketException);
    EXPECT_THROW(Ticket("t1", "v1", "full", -1.0, 1), InvalidTicketException);
    EXPECT_THROW(Ticket("t1", "v1", "full", 10.0, -1), InvalidTicketException);
}

TEST(TicketOfficeTest, SellsDiscountedTicketsAndRefunds) {
    Visitor student("v1", "Student", 20, "student", 500.0);
    Visitor full("v2", "Adult", 30, "full", 500.0);
    TicketOffice office(2);
    const Ticket first = office.SellTicket(student, "student", 200.0, 5);
    EXPECT_EQ(first.Id(), "T1");
    EXPECT_DOUBLE_EQ(first.Price(), 140.0);
    const Ticket second = office.SellTicket(full, "vip", 100.0, 5);
    EXPECT_EQ(second.Id(), "T2");
    EXPECT_DOUBLE_EQ(second.Price(), 100.0);
    EXPECT_EQ(office.TicketsSold(), 2U);
    EXPECT_DOUBLE_EQ(office.TotalRevenue(), 240.0);
    EXPECT_DOUBLE_EQ(office.RevenueForDay(5), 240.0);
    EXPECT_DOUBLE_EQ(office.RevenueForDay(6), 0.0);
    EXPECT_THROW(office.SellTicket(full, "full", 100.0, 5), TicketSoldOutException);
    EXPECT_TRUE(office.Refund("T1"));
    EXPECT_DOUBLE_EQ(office.TotalRevenue(), 100.0);
    EXPECT_FALSE(office.Refund("T1"));
    EXPECT_FALSE(office.Refund("T2"));
}

TEST(TicketOfficeTest, RejectsInvalidCapacity) {
    EXPECT_THROW(TicketOffice(0), InvalidTicketException);
    EXPECT_THROW(TicketOffice(-1), InvalidTicketException);
}

TEST(EventTest, SellsTicketsUntilCapacity) {
    Event event("e1", "Night at the Museum", 10, 100, 20.0);
    EXPECT_EQ(event.SellTickets(50), 50);
    EXPECT_FALSE(event.IsSoldOut());
    EXPECT_EQ(event.SellTickets(50), 100);
    EXPECT_TRUE(event.IsSoldOut());
    EXPECT_DOUBLE_EQ(event.Revenue(), 2000.0);
    EXPECT_THROW(event.SellTickets(1), TicketSoldOutException);
    EXPECT_EQ(event.Id(), "e1");
    EXPECT_EQ(event.Name(), "Night at the Museum");
    EXPECT_EQ(event.Day(), 10);
    EXPECT_EQ(event.Capacity(), 100);
    EXPECT_EQ(event.TicketsSold(), 100);
}

TEST(EventTest, RejectsInvalidData) {
    EXPECT_THROW(Event("", "Name", 1, 10, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Event("e", "", 1, 10, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Event("e", "Name", -1, 10, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Event("e", "Name", 1, 0, 10.0), InvalidExhibitionException);
    EXPECT_THROW(Event("e", "Name", 1, 10, -1.0), InvalidExhibitionException);
    Event event("e", "Name", 1, 10, 10.0);
    EXPECT_THROW(event.SellTickets(0), InvalidTicketException);
}

TEST(TourTest, BooksAndCancelsPlaces) {
    Visitor first = MakeVisitor("v1", 500.0);
    Visitor second = MakeVisitor("v2", 500.0);
    Visitor third = MakeVisitor("v3", 500.0);
    Tour tour("t1", "g1", "Highlights", 2, 100.0);
    EXPECT_TRUE(tour.Book(first));
    EXPECT_TRUE(tour.Book(second));
    EXPECT_TRUE(tour.IsFull());
    EXPECT_THROW(tour.Book(third), BookingException);
    EXPECT_DOUBLE_EQ(tour.Revenue(), 200.0);
    EXPECT_TRUE(tour.Cancel());
    EXPECT_FALSE(tour.IsFull());
    EXPECT_TRUE(tour.Cancel());
    EXPECT_FALSE(tour.Cancel());
    EXPECT_DOUBLE_EQ(tour.Revenue(), 0.0);
    EXPECT_EQ(tour.Id(), "t1");
    EXPECT_EQ(tour.GuideId(), "g1");
    EXPECT_EQ(tour.RouteName(), "Highlights");
    EXPECT_EQ(tour.MaxVisitors(), 2);
    Guide replacement("g2", "New Guide", 30, 900.0, 2020, "Education", {"English"}, 4.0);
    EXPECT_TRUE(tour.AssignGuide(replacement));
    EXPECT_FALSE(tour.AssignGuide(replacement));
    EXPECT_EQ(tour.GuideId(), "g2");
}

TEST(TourTest, RejectsUnaffordableVisitorAndInvalidRecord) {
    Visitor poor = MakeVisitor("v1", 10.0);
    Tour tour("t1", "g1", "Highlights", 2, 100.0);
    EXPECT_THROW(tour.Book(poor), BookingException);
    EXPECT_THROW(Tour("", "g1", "R", 2, 10.0), BookingException);
    EXPECT_THROW(Tour("t", "", "R", 2, 10.0), BookingException);
    EXPECT_THROW(Tour("t", "g", "", 2, 10.0), BookingException);
    EXPECT_THROW(Tour("t", "g", "R", 0, 10.0), BookingException);
    EXPECT_THROW(Tour("t", "g", "R", 2, -1.0), BookingException);
}

TEST(TourBookingTest, TracksPaymentAndCancellation) {
    TourBooking booking("t1", "v1", 5, 100.0);
    EXPECT_FALSE(booking.IsPaid());
    EXPECT_TRUE(booking.Pay());
    EXPECT_FALSE(booking.Pay());
    EXPECT_TRUE(booking.IsPaid());
    EXPECT_THROW(booking.Cancel(), BookingException);
    EXPECT_EQ(booking.Amount(), 100.0);
    EXPECT_EQ(booking.TourId(), "t1");
    EXPECT_EQ(booking.VisitorId(), "v1");
    EXPECT_EQ(booking.Day(), 5);

    TourBooking cancelled("t2", "v2", 6, 50.0);
    EXPECT_TRUE(cancelled.Cancel());
    EXPECT_FALSE(cancelled.Cancel());
    EXPECT_TRUE(cancelled.IsCancelled());
    EXPECT_THROW(cancelled.Pay(), BookingException);
}

TEST(TourBookingTest, RejectsInvalidData) {
    EXPECT_THROW(TourBooking("", "v1", 1, 10.0), BookingException);
    EXPECT_THROW(TourBooking("t1", "", 1, 10.0), BookingException);
    EXPECT_THROW(TourBooking("t1", "v1", -1, 10.0), BookingException);
    EXPECT_THROW(TourBooking("t1", "v1", 1, -1.0), BookingException);
}

TEST(VisitorGroupTest, AddsMembersUpToSize) {
    Visitor first = MakeVisitor("v1");
    Visitor second = MakeVisitor("v2");
    Visitor third = MakeVisitor("v3");
    VisitorGroup group("g1", "School 5", 2, "");
    EXPECT_TRUE(group.NeedsGuide());
    EXPECT_TRUE(group.AddMember(first));
    EXPECT_FALSE(group.AddMember(first));
    EXPECT_TRUE(group.AddMember(second));
    EXPECT_TRUE(group.IsFull());
    EXPECT_FALSE(group.AddMember(third));
    EXPECT_EQ(group.MemberCount(), 2U);
    EXPECT_EQ(group.Id(), "g1");
    EXPECT_EQ(group.SchoolName(), "School 5");
    EXPECT_EQ(group.Size(), 2);
    EXPECT_EQ(group.GuideId(), "");

    VisitorGroup guided("g2", "School 6", 3, "guide-1");
    EXPECT_FALSE(guided.NeedsGuide());
    Guide guide("g3", "Guide", 30, 800.0, 2020, "Education", {"English"}, 4.0);
    VisitorGroup unguided("g3", "School 7", 3, "");
    EXPECT_TRUE(unguided.AssignGuide(guide));
    EXPECT_FALSE(unguided.AssignGuide(guide));
    EXPECT_FALSE(unguided.NeedsGuide());
}

TEST(VisitorGroupTest, RejectsInvalidData) {
    EXPECT_THROW(VisitorGroup("", "School", 2, ""), BookingException);
    EXPECT_THROW(VisitorGroup("g", "", 2, ""), BookingException);
    EXPECT_THROW(VisitorGroup("g", "School", 0, ""), BookingException);
}

TEST(AudioGuideTest, RentsReturnsAndCharges) {
    Visitor first = MakeVisitor("v1");
    Visitor second = MakeVisitor("v2");
    AudioGuide guide("ag1", "English", 20);
    EXPECT_TRUE(guide.IsAvailable());
    EXPECT_TRUE(guide.Rent(first));
    EXPECT_FALSE(guide.IsAvailable());
    EXPECT_FALSE(guide.Rent(second));
    EXPECT_EQ(guide.RentedToVisitorId(), "v1");
    EXPECT_EQ(guide.BatteryPercent(), 10);
    EXPECT_TRUE(guide.Return());
    EXPECT_FALSE(guide.Return());
    EXPECT_TRUE(guide.IsAvailable());
    guide.Charge(20);
    EXPECT_EQ(guide.BatteryPercent(), 20);
    guide.Charge(1000);
    EXPECT_EQ(guide.BatteryPercent(), 100);
    EXPECT_THROW(guide.Charge(0), BookingException);
    EXPECT_EQ(guide.Id(), "ag1");
    EXPECT_EQ(guide.Language(), "English");
}

TEST(AudioGuideTest, RejectsLowBatteryAndInvalidData) {
    Visitor visitor = MakeVisitor("v1");
    AudioGuide low("ag1", "English", 5);
    EXPECT_THROW(low.Rent(visitor), BookingException);
    EXPECT_THROW(AudioGuide("", "English", 50), BookingException);
    EXPECT_THROW(AudioGuide("ag", "", 50), BookingException);
    EXPECT_THROW(AudioGuide("ag", "English", -1), BookingException);
    EXPECT_THROW(AudioGuide("ag", "English", 101), BookingException);
    AudioGuide empty("ag", "English", 0);
    EXPECT_FALSE(empty.IsAvailable());
}

}  // namespace museum
