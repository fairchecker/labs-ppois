/**
 * @file test_tracking.cpp
 * @brief Unit tests for tracking points and history queries.
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "logistics/exceptions.hpp"
#include "logistics/tracking.hpp"

namespace logistics {
namespace {

TrackPoint MakePoint(int day, double speed, double latitude = 53.9, double longitude = 27.56) {
    return TrackPoint("S1", GeoPoint(latitude, longitude), day, 10, speed);
}

}  // namespace

TEST(TrackPointTest, ValidatesRecord) {
    const GeoPoint point(53.9, 27.56);
    EXPECT_THROW(TrackPoint("", point, 1, 10, 60.0), TrackingException);
    EXPECT_THROW(TrackPoint("S1", point, -1, 10, 60.0), TrackingException);
    EXPECT_THROW(TrackPoint("S1", point, 1, -1, 60.0), TrackingException);
    EXPECT_THROW(TrackPoint("S1", point, 1, 24, 60.0), TrackingException);
    EXPECT_THROW(TrackPoint("S1", point, 1, 10, -1.0), TrackingException);
}

TEST(TrackPointTest, ChecksSpeedAndDay) {
    const TrackPoint point = MakePoint(3, 80.0);
    EXPECT_FALSE(point.IsSpeeding(80.0));
    EXPECT_TRUE(point.IsSpeeding(79.9));
    EXPECT_THROW(point.IsSpeeding(-1.0), TrackingException);
    EXPECT_TRUE(point.IsSameDay(3));
    EXPECT_FALSE(point.IsSameDay(4));
    EXPECT_EQ(point.Describe(), "S1 at 53.900000, 27.560000, 80.000000 km/h");
    EXPECT_EQ(point.ShipmentId(), "S1");
    EXPECT_EQ(point.Day(), 3);
    EXPECT_EQ(point.Hour(), 10);
    EXPECT_DOUBLE_EQ(point.SpeedKmh(), 80.0);
}

TEST(TrackingServiceTest, RecordsAndReadsHistory) {
    TrackingService service;
    EXPECT_EQ(service.TrackedShipments(), 0U);
    EXPECT_THROW(service.History("S1"), TrackingException);
    EXPECT_THROW(service.Latest("S1"), TrackingException);
    EXPECT_THROW(service.DistanceTravelled("S1"), TrackingException);
    EXPECT_THROW(service.AverageSpeedKmh("S1"), TrackingException);
    service.Record(MakePoint(1, 60.0));
    service.Record(MakePoint(2, 80.0, 52.09, 23.73));
    EXPECT_EQ(service.TrackedShipments(), 1U);
    EXPECT_EQ(service.History("S1").size(), 2U);
    EXPECT_EQ(service.Latest("S1").Day(), 2);
    EXPECT_NEAR(service.DistanceTravelled("S1"), 326.7, 5.0);
    EXPECT_DOUBLE_EQ(service.AverageSpeedKmh("S1"), 70.0);
}

TEST(TrackingServiceTest, DetectsStaleTracking) {
    TrackingService service;
    EXPECT_THROW(service.IsStale("S1", 10, 2), TrackingException);
    service.Record(MakePoint(2, 60.0));
    EXPECT_FALSE(service.IsStale("S1", 4, 2));
    EXPECT_TRUE(service.IsStale("S1", 5, 2));
    EXPECT_THROW(service.IsStale("S1", 5, -1), TrackingException);
    service.Record(MakePoint(3, 60.0));
    EXPECT_TRUE(service.IsStale("S1", 5, 1));
}

}  // namespace logistics
