#pragma once

#include <string>
#include <vector>

/**
 * @file geo.hpp
 * @brief Geography primitives: coordinates, addresses, route segments and routes.
 */

namespace logistics {

/// @brief Earth radius used by the great-circle distance formula, in kilometres.
inline constexpr double kEarthRadiusKm = 6371.0;
/// @brief Minimum latitude accepted by GeoPoint.
inline constexpr double kMinLatitude = -90.0;
/// @brief Maximum latitude accepted by GeoPoint.
inline constexpr double kMaxLatitude = 90.0;
/// @brief Minimum longitude accepted by GeoPoint.
inline constexpr double kMinLongitude = -180.0;
/// @brief Maximum longitude accepted by GeoPoint.
inline constexpr double kMaxLongitude = 180.0;
/// @brief Shortest route still counted as a short haul, in kilometres.
inline constexpr double kLongHaulThresholdKm = 500.0;

/**
 * @class GeoPoint
 * @brief Geographic coordinate with distance calculations.
 */
class GeoPoint {
    public:
    /**
     * @brief Creates a coordinate.
     * @throws InvalidRouteException when a coordinate is out of range.
     */
    GeoPoint(double latitude, double longitude);

    /// @brief Returns the latitude in degrees.
    double Latitude() const { return latitude_; }

    /// @brief Returns the longitude in degrees.
    double Longitude() const { return longitude_; }

    /**
     * @brief Great-circle distance to another coordinate.
     * @param other target coordinate.
     * @return distance in kilometres.
     */
    double DistanceTo(const GeoPoint& other) const;

    /**
     * @brief Describes the coordinate with six decimal places.
     * @return "lat, lon" string.
     */
    std::string Describe() const;

    private:
    double latitude_;
    double longitude_;
};

/**
 * @class Address
 * @brief Postal address with a geographic location.
 */
class Address {
    public:
    /**
     * @brief Creates an address.
     * @throws InvalidRouteException when a required part is empty.
     */
    Address(std::string street, std::string city, std::string postalCode, GeoPoint location);

    /// @brief Returns the street and building number.
    const std::string& Street() const { return street_; }

    /// @brief Returns the city name.
    const std::string& City() const { return city_; }

    /// @brief Returns the postal code.
    const std::string& PostalCode() const { return postalCode_; }

    /// @brief Returns the geographic location.
    const GeoPoint& Location() const { return location_; }

    /**
     * @brief Formats the address for reports.
     * @return "street, city postalCode".
     */
    std::string Format() const;

    /**
     * @brief Checks whether both addresses belong to the same city.
     * @param other address to compare with.
     */
    bool IsSameCity(const Address& other) const;

    /**
     * @brief Distance to another address.
     * @param other target address.
     * @return distance in kilometres.
     */
    double DistanceTo(const Address& other) const;

    private:
    std::string street_;
    std::string city_;
    std::string postalCode_;
    GeoPoint location_;
};

/**
 * @class RouteSegment
 * @brief One leg of a route between two addresses.
 */
class RouteSegment {
    public:
    /**
     * @brief Creates a route segment.
     * @throws InvalidRouteException for an empty address or non-positive distance.
     */
    RouteSegment(Address origin, Address destination, double distanceKm, std::string roadType,
                 double tollCost);

    /// @brief Returns the starting address.
    const Address& Origin() const { return origin_; }

    /// @brief Returns the destination address.
    const Address& Destination() const { return destination_; }

    /// @brief Returns the length in kilometres.
    double DistanceKm() const { return distanceKm_; }

    /// @brief Returns the road class, for example "highway".
    const std::string& RoadType() const { return roadType_; }

    /// @brief Returns the toll fee in currency units.
    double TollCost() const { return tollCost_; }

    /**
     * @brief Travel time at an average speed.
     * @param averageSpeedKmh speed in kilometres per hour, must be positive.
     * @throws InvalidRouteException when the speed is not positive.
     */
    double TravelMinutes(double averageSpeedKmh) const;

    /// @brief Checks whether the segment has a non-zero toll.
    bool IsTollRoad() const { return tollCost_ > 0.0; }

    /**
     * @brief Builds a one-line segment description.
     * @return "origin -> destination, N km".
     */
    std::string Describe() const;

    private:
    Address origin_;
    Address destination_;
    double distanceKm_;
    std::string roadType_;
    double tollCost_;
};

/**
 * @class Route
 * @brief Ordered sequence of segments with aggregate metrics.
 */
class Route {
    public:
    /**
     * @brief Creates an empty route.
     * @throws InvalidRouteException when the identifier is empty or the rate is negative.
     */
    Route(std::string id, double baseRatePerKm);

    /// @brief Returns the route identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the segments in travel order.
    const std::vector<RouteSegment>& Segments() const { return segments_; }

    /// @brief Returns the base price per kilometre.
    double BaseRatePerKm() const { return baseRatePerKm_; }

    /**
     * @brief Appends a segment to the route.
     * @param segment segment to append.
     * @throws InvalidRouteException when the segment does not continue the route.
     */
    void AddSegment(const RouteSegment& segment);

    /// @brief Returns the sum of segment distances.
    double TotalDistanceKm() const;

    /// @brief Returns the sum of toll fees.
    double TotalTollCost() const;

    /**
     * @brief Estimated travel time at an average speed.
     * @param averageSpeedKmh speed in kilometres per hour.
     */
    double EstimatedMinutes(double averageSpeedKmh) const;

    /// @brief Checks whether the route is longer than kLongHaulThresholdKm.
    bool IsLongHaul() const;

    /// @brief Returns the number of segments.
    std::size_t SegmentCount() const { return segments_.size(); }

    /**
     * @brief Builds a one-line route description.
     * @return "id: N segments, D km".
     */
    std::string Describe() const;

    private:
    std::string id_;
    std::vector<RouteSegment> segments_;
    double baseRatePerKm_;
};

}  // namespace logistics
