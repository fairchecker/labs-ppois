/**
 * @file test_security.cpp
 * @brief Unit tests for security, feedback and facility services.
 */

#include <gtest/gtest.h>

#include "museum/access_card.hpp"
#include "museum/access_log.hpp"
#include "museum/alarm_system.hpp"
#include "museum/cafe.hpp"
#include "museum/climate_log.hpp"
#include "museum/exceptions.hpp"
#include "museum/feedback_board.hpp"
#include "museum/incident.hpp"
#include "museum/parking_lot.hpp"
#include "museum/security_camera.hpp"
#include "museum/security_system.hpp"
#include "museum/shop_item.hpp"
#include "museum/visitor_feedback.hpp"
#include "museum/visitor.hpp"

namespace museum {

TEST(SecurityCameraTest, RecordsAndCaptures) {
    SecurityCamera camera("cam1", "hall", 1080);
    EXPECT_FALSE(camera.IsRecording());
    EXPECT_THROW(camera.CaptureFrame(), SecurityException);
    EXPECT_TRUE(camera.StartRecording());
    EXPECT_FALSE(camera.StartRecording());
    EXPECT_TRUE(camera.IsRecording());
    EXPECT_EQ(camera.CaptureFrame(), "frame:cam1");
    EXPECT_TRUE(camera.StopRecording());
    EXPECT_FALSE(camera.StopRecording());
    EXPECT_TRUE(camera.IsHighResolution());
    EXPECT_EQ(camera.Id(), "cam1");
    EXPECT_EQ(camera.Zone(), "hall");
    EXPECT_EQ(camera.ResolutionP(), 1080);
    SecurityCamera low("cam2", "shop", 720);
    EXPECT_FALSE(low.IsHighResolution());
}

TEST(SecurityCameraTest, RejectsInvalidData) {
    EXPECT_THROW(SecurityCamera("", "zone", 720), SecurityException);
    EXPECT_THROW(SecurityCamera("cam", "", 720), SecurityException);
    EXPECT_THROW(SecurityCamera("cam", "zone", 0), SecurityException);
}

TEST(IncidentTest, ResolvesAndEscalates) {
    Incident incident("i1", "theft", 3, "Missing statuette");
    EXPECT_FALSE(incident.IsCritical());
    EXPECT_TRUE(incident.Resolve());
    EXPECT_FALSE(incident.Resolve());
    Incident severe("i2", "fire", 4, "Smoke in storage");
    EXPECT_TRUE(severe.IsCritical());
    severe.Escalate();
    EXPECT_EQ(severe.Severity(), 5);
    EXPECT_THROW(severe.Escalate(), SecurityException);
    EXPECT_EQ(severe.Id(), "i2");
    EXPECT_EQ(severe.Type(), "fire");
    EXPECT_EQ(severe.Description(), "Smoke in storage");
    EXPECT_FALSE(severe.IsResolved());
}

TEST(IncidentTest, RejectsInvalidData) {
    EXPECT_THROW(Incident("", "theft", 1, "d"), SecurityException);
    EXPECT_THROW(Incident("i", "", 1, "d"), SecurityException);
    EXPECT_THROW(Incident("i", "theft", 1, ""), SecurityException);
    EXPECT_THROW(Incident("i", "theft", 0, "d"), SecurityException);
    EXPECT_THROW(Incident("i", "theft", 6, "d"), SecurityException);
}

TEST(AccessLogTest, TogglesDecision) {
    AccessLog log("p1", "storage", 5);
    EXPECT_FALSE(log.IsAllowed());
    EXPECT_TRUE(log.Grant());
    EXPECT_FALSE(log.Grant());
    EXPECT_TRUE(log.IsAllowed());
    EXPECT_TRUE(log.Deny());
    EXPECT_FALSE(log.Deny());
    EXPECT_FALSE(log.IsAllowed());
    EXPECT_EQ(log.PersonId(), "p1");
    EXPECT_EQ(log.Zone(), "storage");
    EXPECT_EQ(log.Day(), 5);
}

TEST(AccessLogTest, RejectsInvalidData) {
    EXPECT_THROW(AccessLog("", "zone", 1), SecurityException);
    EXPECT_THROW(AccessLog("p", "", 1), SecurityException);
    EXPECT_THROW(AccessLog("p", "zone", -1), SecurityException);
}

TEST(AccessCardTest, EnforcesLevelsWhenActive) {
    AccessCard card("card1", "e1", 2);
    EXPECT_FALSE(card.IsActive());
    EXPECT_FALSE(card.HasLevel(1));
    EXPECT_TRUE(card.Activate());
    EXPECT_FALSE(card.Activate());
    EXPECT_TRUE(card.HasLevel(2));
    EXPECT_FALSE(card.HasLevel(3));
    EXPECT_THROW(card.HasLevel(4), SecurityException);
    EXPECT_TRUE(card.Deactivate());
    EXPECT_FALSE(card.Deactivate());
    EXPECT_EQ(card.Id(), "card1");
    EXPECT_EQ(card.OwnerId(), "e1");
    EXPECT_EQ(card.Level(), 2);
}

TEST(AccessCardTest, RejectsInvalidData) {
    EXPECT_THROW(AccessCard("", "e1", 1), SecurityException);
    EXPECT_THROW(AccessCard("card", "", 1), SecurityException);
    EXPECT_THROW(AccessCard("card", "e1", 0), SecurityException);
    EXPECT_THROW(AccessCard("card", "e1", 4), SecurityException);
}

TEST(SecuritySystemTest, TracksCamerasIncidentsAndAccess) {
    SecuritySystem system("secret");
    EXPECT_FALSE(system.IsArmed());
    EXPECT_TRUE(system.Arm());
    EXPECT_FALSE(system.Arm());
    EXPECT_THROW(system.Disarm("wrong"), SecurityException);
    EXPECT_TRUE(system.Disarm("secret"));
    EXPECT_FALSE(system.Disarm("secret"));

    SecurityCamera camera("cam1", "hall", 720);
    camera.StartRecording();
    system.AddCamera(camera);
    EXPECT_EQ(system.CamerasOnline(), 1U);

    system.ReportIncident(Incident("i1", "theft", 2, "Missing item"));
    EXPECT_EQ(system.IncidentCount(), 1U);

    AccessLog log("p1", "hall", 1);
    log.Grant();
    system.LogAccess(log);
    EXPECT_EQ(system.LogCount(), 1U);

    AccessCard card("card1", "e1", 3);
    card.Activate();
    EXPECT_NO_THROW(system.RequireLevel(card, 2));
    EXPECT_THROW(system.RequireLevel(card, 4), SecurityException);
    AccessCard inactive("card2", "e2", 3);
    EXPECT_THROW(system.RequireLevel(inactive, 1), AccessDeniedException);
    AccessCard low("card3", "e3", 1);
    low.Activate();
    EXPECT_THROW(system.RequireLevel(low, 2), AccessDeniedException);
}

TEST(SecuritySystemTest, RejectsEmptyCode) {
    EXPECT_THROW(SecuritySystem(""), SecurityException);
}

TEST(AlarmSystemTest, ArmsTriggersAndResets) {
    AlarmSystem alarm("code");
    EXPECT_FALSE(alarm.IsArmed());
    EXPECT_TRUE(alarm.Arm());
    EXPECT_FALSE(alarm.Arm());
    alarm.Trigger();
    EXPECT_TRUE(alarm.IsTriggered());
    EXPECT_THROW(alarm.Arm(), SecurityException);
    EXPECT_THROW(alarm.Reset("wrong"), SecurityException);
    EXPECT_TRUE(alarm.Reset("code"));
    EXPECT_FALSE(alarm.IsTriggered());
    EXPECT_FALSE(alarm.IsArmed());
    EXPECT_FALSE(alarm.Reset("code"));
    EXPECT_THROW(alarm.Trigger(), SecurityException);
    EXPECT_TRUE(alarm.Arm());
    EXPECT_THROW(alarm.Disarm("wrong"), SecurityException);
    EXPECT_TRUE(alarm.Disarm("code"));
    EXPECT_FALSE(alarm.Disarm("code"));
    EXPECT_THROW(AlarmSystem(""), SecurityException);
}

TEST(VisitorFeedbackTest, ClassifiesRatings) {
    VisitorFeedback positive("v1", 5, "Great", 3);
    VisitorFeedback neutral("v2", 3, "Okay", 3);
    VisitorFeedback negative("v3", 1, "Bad", 3);
    EXPECT_TRUE(positive.IsPositive());
    EXPECT_FALSE(positive.IsComplaint());
    EXPECT_FALSE(neutral.IsPositive());
    EXPECT_FALSE(neutral.IsComplaint());
    EXPECT_TRUE(negative.IsComplaint());
    EXPECT_EQ(negative.VisitorId(), "v3");
    EXPECT_EQ(negative.Rating(), 1);
    EXPECT_EQ(negative.Comment(), "Bad");
    EXPECT_EQ(negative.Day(), 3);
    Visitor author("v3", "Author", 25, "full", 100.0);
    Visitor other("v4", "Other", 25, "full", 100.0);
    EXPECT_TRUE(negative.WrittenBy(author));
    EXPECT_FALSE(negative.WrittenBy(other));
}

TEST(VisitorFeedbackTest, RejectsInvalidData) {
    EXPECT_THROW(VisitorFeedback("", 5, "c", 1), FeedbackException);
    EXPECT_THROW(VisitorFeedback("v", 0, "c", 1), InvalidRatingException);
    EXPECT_THROW(VisitorFeedback("v", 6, "c", 1), InvalidRatingException);
    EXPECT_THROW(VisitorFeedback("v", 5, "c", -1), FeedbackException);
}

TEST(FeedbackBoardTest, AggregatesReviews) {
    FeedbackBoard board(3);
    EXPECT_THROW(board.AverageRating(), FeedbackException);
    EXPECT_TRUE(board.Add(VisitorFeedback("v1", 5, "Great", 1)));
    EXPECT_TRUE(board.Add(VisitorFeedback("v2", 3, "Okay", 1)));
    EXPECT_DOUBLE_EQ(board.AverageRating(), 4.0);
    EXPECT_EQ(board.ComplaintCount(), 0U);
    EXPECT_TRUE(board.Add(VisitorFeedback("v3", 1, "Bad", 1)));
    EXPECT_EQ(board.ComplaintCount(), 1U);
    EXPECT_EQ(board.Size(), 3U);
    EXPECT_THROW(board.Add(VisitorFeedback("v4", 4, "Good", 1)), FeedbackException);
    board.Clear();
    EXPECT_EQ(board.Size(), 0U);
}

TEST(FeedbackBoardTest, RejectsInvalidCapacity) {
    EXPECT_THROW(FeedbackBoard(0), FeedbackException);
    EXPECT_THROW(FeedbackBoard(-1), FeedbackException);
}

TEST(CafeTest, SellsMenuItemsAndSeatsGuests) {
    Cafe cafe("Museum Cafe", 20);
    EXPECT_TRUE(cafe.AddMenuItem(ShopItem("Coffee", 3.0, 100, "drink")));
    EXPECT_FALSE(cafe.AddMenuItem(ShopItem("Coffee", 3.0, 100, "drink")));
    EXPECT_EQ(cafe.MenuSize(), 1U);
    EXPECT_DOUBLE_EQ(cafe.Sell("Coffee", 4), 12.0);
    EXPECT_DOUBLE_EQ(cafe.Revenue(), 12.0);
    EXPECT_THROW(cafe.Sell("Tea", 1), FacilityException);
    EXPECT_THROW(cafe.Sell("Coffee", 1000), InvalidShopItemException);
    EXPECT_TRUE(cafe.SeatGuests(20));
    EXPECT_FALSE(cafe.SeatGuests(21));
    EXPECT_THROW(cafe.SeatGuests(-1), FacilityException);
    EXPECT_EQ(cafe.Name(), "Museum Cafe");
    EXPECT_EQ(cafe.Seats(), 20);
}

TEST(CafeTest, RejectsInvalidData) {
    EXPECT_THROW(Cafe("", 10), FacilityException);
    EXPECT_THROW(Cafe("Cafe", 0), FacilityException);
}

TEST(ParkingLotTest, TracksOccupancyAndFees) {
    ParkingLot lot(2, 5.0);
    lot.Park();
    EXPECT_FALSE(lot.IsFull());
    lot.Park();
    EXPECT_TRUE(lot.IsFull());
    EXPECT_THROW(lot.Park(), FacilityException);
    EXPECT_TRUE(lot.Leave());
    EXPECT_TRUE(lot.Leave());
    EXPECT_FALSE(lot.Leave());
    EXPECT_DOUBLE_EQ(lot.Fee(3), 15.0);
    EXPECT_THROW(lot.Fee(0), FacilityException);
    EXPECT_EQ(lot.Occupied(), 0);
    EXPECT_EQ(lot.Capacity(), 2);
    EXPECT_DOUBLE_EQ(lot.HourlyFee(), 5.0);
}

TEST(ParkingLotTest, RejectsInvalidData) {
    EXPECT_THROW(ParkingLot(0, 1.0), FacilityException);
    EXPECT_THROW(ParkingLot(10, -1.0), FacilityException);
}

TEST(ClimateLogTest, RecordsAndChecksStability) {
    ClimateLog log(18.0, 24.0);
    EXPECT_THROW(log.IsStable(), ClimateException);
    EXPECT_THROW(log.Average(), ClimateException);
    log.Record(20.0);
    log.Record(21.0);
    EXPECT_TRUE(log.IsStable());
    EXPECT_DOUBLE_EQ(log.Average(), 20.5);
    log.Record(30.0);
    EXPECT_FALSE(log.IsStable());
    EXPECT_EQ(log.ReadingCount(), 3U);
    EXPECT_DOUBLE_EQ(log.MinTemperature(), 18.0);
    EXPECT_DOUBLE_EQ(log.MaxTemperature(), 24.0);
    EXPECT_THROW(log.Record(60.0), ClimateException);
}

TEST(ClimateLogTest, RejectsInvalidRange) {
    EXPECT_THROW(ClimateLog(-11.0, 20.0), ClimateException);
    EXPECT_THROW(ClimateLog(18.0, 51.0), ClimateException);
    EXPECT_THROW(ClimateLog(24.0, 20.0), ClimateException);
}

}  // namespace museum
