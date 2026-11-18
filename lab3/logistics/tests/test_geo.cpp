/**
 * @file test_geo.cpp
 * @brief Unit tests for coordinates, addresses, segments and routes.
 */

#include <gtest/gtest.h>

#include <string>

#include "logistics/exceptions.hpp"
#include "logistics/geo.hpp"

namespace logistics {
namespace {

Address MinskDepot() {
    return Address("Depot 1", "Minsk", "220000", GeoPoint(53.9023, 27.5619));
}

Address BrestDepot() {
    return Address("Port 2", "Brest", "224000", GeoPoint(52.0976, 23.7341));
}

}  // namespace

TEST(GeoPointTest, RejectsOutOfRangeCoordinates) {
    EXPECT_THROW(GeoPoint(91.0, 0.0), InvalidRouteException);
    EXPECT_THROW(GeoPoint(-91.0, 0.0), InvalidRouteException);
    EXPECT_THROW(GeoPoint(0.0, 181.0), InvalidRouteException);
    EXPECT_THROW(GeoPoint(0.0, -181.0), InvalidRouteException);
}

TEST(GeoPointTest, ComputesDistanceBetweenCities) {
    const GeoPoint minsk(53.9023, 27.5619);
    const GeoPoint brest(52.0976, 23.7341);
    EXPECT_DOUBLE_EQ(minsk.DistanceTo(minsk), 0.0);
    EXPECT_NEAR(minsk.DistanceTo(brest), 326.7, 5.0);
    EXPECT_NEAR(brest.DistanceTo(minsk), minsk.DistanceTo(brest), 1e-9);
}

TEST(GeoPointTest, DescribesCoordinates) {
    const GeoPoint point(53.9, 27.56);
    EXPECT_EQ(point.Describe(), "53.900000, 27.560000");
    EXPECT_DOUBLE_EQ(point.Latitude(), 53.9);
    EXPECT_DOUBLE_EQ(point.Longitude(), 27.56);
}

TEST(AddressTest, RejectsIncompleteAddress) {
    const GeoPoint point(0.0, 0.0);
    EXPECT_THROW(Address("", "Minsk", "220000", point), InvalidRouteException);
    EXPECT_THROW(Address("Street", "", "220000", point), InvalidRouteException);
    EXPECT_THROW(Address("Street", "Minsk", "", point), InvalidRouteException);
}

TEST(AddressTest, FormatsAndComparesAddresses) {
    const Address minsk = MinskDepot();
    const Address brest = BrestDepot();
    EXPECT_EQ(minsk.Format(), "Depot 1, Minsk 220000");
    EXPECT_EQ(minsk.Street(), "Depot 1");
    EXPECT_EQ(minsk.City(), "Minsk");
    EXPECT_EQ(minsk.PostalCode(), "220000");
    EXPECT_TRUE(minsk.IsSameCity(MinskDepot()));
    EXPECT_FALSE(minsk.IsSameCity(brest));
    EXPECT_NEAR(minsk.DistanceTo(brest), 326.7, 5.0);
}

TEST(RouteSegmentTest, RejectsInvalidSegment) {
    const Address minsk = MinskDepot();
    const Address brest = BrestDepot();
    EXPECT_THROW(RouteSegment(minsk, brest, 0.0, "highway", 0.0), InvalidRouteException);
    EXPECT_THROW(RouteSegment(minsk, brest, 100.0, "", 0.0), InvalidRouteException);
    EXPECT_THROW(RouteSegment(minsk, brest, 100.0, "highway", -1.0), InvalidRouteException);
}

TEST(RouteSegmentTest, ComputesTravelTime) {
    const RouteSegment segment(MinskDepot(), BrestDepot(), 120.0, "highway", 25.0);
    EXPECT_DOUBLE_EQ(segment.TravelMinutes(60.0), 120.0);
    EXPECT_DOUBLE_EQ(segment.TravelMinutes(120.0), 60.0);
    EXPECT_THROW(segment.TravelMinutes(0.0), InvalidRouteException);
    EXPECT_TRUE(segment.IsTollRoad());
    EXPECT_EQ(segment.RoadType(), "highway");
    EXPECT_DOUBLE_EQ(segment.DistanceKm(), 120.0);
    EXPECT_DOUBLE_EQ(segment.TollCost(), 25.0);
    EXPECT_EQ(segment.Describe(), "Minsk -> Brest, 120.0 km");
}

TEST(RouteSegmentTest, ReportsFreeRoad) {
    const RouteSegment segment(MinskDepot(), BrestDepot(), 10.0, "local", 0.0);
    EXPECT_FALSE(segment.IsTollRoad());
}

TEST(RouteTest, RejectsInvalidRoute) {
    EXPECT_THROW(Route("", 1.0), InvalidRouteException);
    EXPECT_THROW(Route("R1", -1.0), InvalidRouteException);
}

TEST(RouteTest, AggregatesSegments) {
    Route route("R1", 1.5);
    route.AddSegment(RouteSegment(MinskDepot(), BrestDepot(), 300.0, "highway", 20.0));
    route.AddSegment(RouteSegment(BrestDepot(), MinskDepot(), 300.0, "local", 10.0));
    EXPECT_EQ(route.Id(), "R1");
    EXPECT_DOUBLE_EQ(route.BaseRatePerKm(), 1.5);
    EXPECT_EQ(route.SegmentCount(), 2U);
    EXPECT_DOUBLE_EQ(route.TotalDistanceKm(), 600.0);
    EXPECT_DOUBLE_EQ(route.TotalTollCost(), 30.0);
    EXPECT_DOUBLE_EQ(route.EstimatedMinutes(100.0), 360.0);
    EXPECT_TRUE(route.IsLongHaul());
    EXPECT_EQ(route.Describe(), "R1: 2 segments, 600.0 km");
}

TEST(RouteTest, RejectsDisconnectedSegmentAndInvalidSpeed) {
    Route route("R1", 1.0);
    route.AddSegment(RouteSegment(MinskDepot(), BrestDepot(), 100.0, "highway", 0.0));
    EXPECT_THROW(route.AddSegment(RouteSegment(MinskDepot(), BrestDepot(), 100.0, "highway", 0.0)),
                 InvalidRouteException);
    EXPECT_THROW(route.EstimatedMinutes(0.0), InvalidRouteException);
}

TEST(RouteTest, ShortRouteIsNotLongHaul) {
    Route route("R2", 1.0);
    EXPECT_FALSE(route.IsLongHaul());
    EXPECT_DOUBLE_EQ(route.TotalDistanceKm(), 0.0);
    route.AddSegment(RouteSegment(MinskDepot(), BrestDepot(), 100.0, "highway", 0.0));
    EXPECT_FALSE(route.IsLongHaul());
    EXPECT_EQ(route.Segments().size(), 1U);
}

}  // namespace logistics
