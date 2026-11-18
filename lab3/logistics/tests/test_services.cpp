/**
 * @file test_services.cpp
 * @brief Unit tests for route planning, notifications, quality and incidents.
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "logistics/exceptions.hpp"
#include "logistics/services.hpp"

namespace logistics {
namespace {

Address Stop(const std::string& city, double latitude, double longitude) {
    return Address("Stop", city, "000000", GeoPoint(latitude, longitude));
}

CustomerFeedback Feedback(double rating) {
    return CustomerFeedback("F1", "C1", "O1", rating, "comment");
}

}  // namespace

TEST(RoutePlannerTest, ValidatesSpeed) {
    EXPECT_THROW(RoutePlanner(0.0), InvalidRouteException);
    EXPECT_THROW(RoutePlanner(-1.0), InvalidRouteException);
    const RoutePlanner planner(60.0);
    EXPECT_DOUBLE_EQ(planner.AverageSpeedKmh(), 60.0);
    EXPECT_TRUE(planner.Waypoints().empty());
}

TEST(RoutePlannerTest, MeasuresWaypoints) {
    RoutePlanner planner(60.0);
    planner.AddWaypoint(Stop("Minsk", 53.9, 27.56));
    planner.AddWaypoint(Stop("Brest", 52.09, 23.73));
    planner.AddWaypoint(Stop("Warsaw", 52.23, 21.01));
    EXPECT_EQ(planner.Waypoints().size(), 3U);
    EXPECT_GT(planner.TotalDistanceKm(), 300.0);
    EXPECT_DOUBLE_EQ(planner.EstimatedHours(), planner.TotalDistanceKm() / 60.0);
}

TEST(RoutePlannerTest, OptimizesWaypointOrder) {
    RoutePlanner empty(60.0);
    EXPECT_TRUE(empty.OptimizeOrder().empty());
    RoutePlanner single(60.0);
    single.AddWaypoint(Stop("A", 0.0, 0.0));
    EXPECT_EQ(single.OptimizeOrder().size(), 1U);
    RoutePlanner planner(60.0);
    planner.AddWaypoint(Stop("A", 0.0, 0.0));
    planner.AddWaypoint(Stop("C", 0.0, 0.2));
    planner.AddWaypoint(Stop("B", 0.0, 0.1));
    const std::vector<Address> ordered = planner.OptimizeOrder();
    ASSERT_EQ(ordered.size(), 3U);
    EXPECT_EQ(ordered[0].City(), "A");
    EXPECT_EQ(ordered[1].City(), "B");
    EXPECT_EQ(ordered[2].City(), "C");
}

TEST(NotificationTest, ValidatesAndSends) {
    EXPECT_THROW(Notification("", "hello", "sms"), InvalidClientException);
    EXPECT_THROW(Notification("C1", "hello", ""), InvalidClientException);
    Notification notification("C1", "hello", "sms");
    EXPECT_FALSE(notification.IsSent());
    notification.Send();
    EXPECT_TRUE(notification.IsSent());
    EXPECT_EQ(notification.RecipientId(), "C1");
    EXPECT_EQ(notification.Message(), "hello");
    EXPECT_EQ(notification.Channel(), "sms");
    Notification empty("C1", "", "sms");
    EXPECT_THROW(empty.Send(), InvalidClientException);
}

TEST(NotificationTest, ShortensMessages) {
    const Notification notification("C1", "hello world", "sms");
    EXPECT_EQ(notification.Shorten(20), "hello world");
    EXPECT_EQ(notification.Shorten(8), "hello...");
    EXPECT_THROW(notification.Shorten(3), InvalidClientException);
}

TEST(ServiceQualityMonitorTest, HandlesEmptyState) {
    const ServiceQualityMonitor monitor;
    EXPECT_DOUBLE_EQ(monitor.AverageRating(), 0.0);
    EXPECT_DOUBLE_EQ(monitor.OnTimeRate(), 0.0);
    EXPECT_EQ(monitor.QualityGrade(), "C");
    EXPECT_EQ(monitor.TotalDeliveries(), 0);
    EXPECT_EQ(monitor.LateDeliveries(), 0);
}

TEST(ServiceQualityMonitorTest, GradesService) {
    ServiceQualityMonitor excellent;
    excellent.RecordFeedback(Feedback(5.0));
    excellent.RecordFeedback(Feedback(5.0));
    for (int index = 0; index < 9; ++index) {
        excellent.RecordDelivery(false);
    }
    excellent.RecordDelivery(true);
    EXPECT_DOUBLE_EQ(excellent.AverageRating(), 5.0);
    EXPECT_DOUBLE_EQ(excellent.OnTimeRate(), 90.0);
    EXPECT_EQ(excellent.QualityGrade(), "A");
    EXPECT_EQ(excellent.Feedbacks().size(), 2U);
    ServiceQualityMonitor acceptable;
    acceptable.RecordFeedback(Feedback(4.0));
    acceptable.RecordDelivery(true);
    EXPECT_EQ(acceptable.QualityGrade(), "B");
    ServiceQualityMonitor poor;
    poor.RecordFeedback(Feedback(2.0));
    EXPECT_EQ(poor.QualityGrade(), "C");
}

TEST(IncidentReportTest, ValidatesRecord) {
    EXPECT_THROW(IncidentReport("", "S1", "delay", IncidentSeverity::Low, 0), InvalidShipmentException);
    EXPECT_THROW(IncidentReport("I1", "", "delay", IncidentSeverity::Low, 0), InvalidShipmentException);
    EXPECT_THROW(IncidentReport("I1", "S1", "", IncidentSeverity::Low, 0), InvalidShipmentException);
    EXPECT_THROW(IncidentReport("I1", "S1", "delay", IncidentSeverity::Low, -1), InvalidShipmentException);
    EXPECT_THROW(IncidentReport("I1", "S1", "delay", static_cast<IncidentSeverity>(9), 0),
                 InvalidShipmentException);
}

TEST(IncidentReportTest, EscalatesAndResolves) {
    IncidentReport incident("I1", "S1", "delay", IncidentSeverity::Low, 1);
    EXPECT_FALSE(incident.IsResolved());
    EXPECT_FALSE(incident.IsCritical());
    incident.Escalate();
    EXPECT_EQ(incident.Severity(), IncidentSeverity::Medium);
    incident.Escalate();
    EXPECT_EQ(incident.Severity(), IncidentSeverity::High);
    incident.Escalate();
    EXPECT_TRUE(incident.IsCritical());
    EXPECT_THROW(incident.Escalate(), InvalidShipmentException);
    incident.Resolve();
    EXPECT_TRUE(incident.IsResolved());
    EXPECT_THROW(incident.Resolve(), InvalidShipmentException);
    EXPECT_THROW(incident.Escalate(), InvalidShipmentException);
    EXPECT_EQ(incident.Describe(), "delay (critical) for S1, resolved");
    EXPECT_EQ(incident.Id(), "I1");
    EXPECT_EQ(incident.ShipmentId(), "S1");
    EXPECT_EQ(incident.Kind(), "delay");
    EXPECT_EQ(incident.Day(), 1);
}

}  // namespace logistics
