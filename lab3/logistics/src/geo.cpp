#include "logistics/geo.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kMinutesPerHour = 60.0;
constexpr double kHalfCircleDegrees = 180.0;
constexpr int kCoordinatePrecision = 6;
constexpr int kDistancePrecision = 1;
constexpr double kDegreesPerRadian = kHalfCircleDegrees / kPi;

double ToRadians(double degrees) { return degrees / kDegreesPerRadian; }

double SineSquared(double radians) {
    const double sine = std::sin(radians);
    return sine * sine;
}

}  // namespace

GeoPoint::GeoPoint(double latitude, double longitude) : latitude_(latitude), longitude_(longitude) {
    const bool latitudeValid = latitude >= kMinLatitude && latitude <= kMaxLatitude;
    const bool longitudeValid = longitude >= kMinLongitude && longitude <= kMaxLongitude;
    if (!latitudeValid || !longitudeValid) {
        throw InvalidRouteException("GeoPoint coordinates are out of range");
    }
}

double GeoPoint::DistanceTo(const GeoPoint& other) const {
    const double latitudeDelta = ToRadians(other.latitude_ - latitude_);
    const double longitudeDelta = ToRadians(other.longitude_ - longitude_);
    const double originLatitude = ToRadians(latitude_);
    const double targetLatitude = ToRadians(other.latitude_);
    const double haversine = SineSquared(latitudeDelta / 2.0) +
                             std::cos(originLatitude) * std::cos(targetLatitude) *
                                 SineSquared(longitudeDelta / 2.0);
    const double clamped = std::min(1.0, std::max(0.0, haversine));
    const double centralAngle = 2.0 * std::atan2(std::sqrt(clamped), std::sqrt(1.0 - clamped));
    return kEarthRadiusKm * centralAngle;
}

std::string GeoPoint::Describe() const {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(kCoordinatePrecision) << latitude_ << ", "
           << longitude_;
    return stream.str();
}

Address::Address(std::string street, std::string city, std::string postalCode, GeoPoint location)
    : street_(std::move(street)),
      city_(std::move(city)),
      postalCode_(std::move(postalCode)),
      location_(location) {
    if (street_.empty() || city_.empty() || postalCode_.empty()) {
        throw InvalidRouteException("Address requires street, city and postal code");
    }
}

std::string Address::Format() const { return street_ + ", " + city_ + " " + postalCode_; }

bool Address::IsSameCity(const Address& other) const { return city_ == other.city_; }

double Address::DistanceTo(const Address& other) const { return location_.DistanceTo(other.location_); }

RouteSegment::RouteSegment(Address origin, Address destination, double distanceKm,
                           std::string roadType, double tollCost)
    : origin_(std::move(origin)),
      destination_(std::move(destination)),
      distanceKm_(distanceKm),
      roadType_(std::move(roadType)),
      tollCost_(tollCost) {
    if (distanceKm_ <= 0.0 || roadType_.empty() || tollCost_ < 0.0) {
        throw InvalidRouteException("Route segment has invalid distance, road type or toll");
    }
}

double RouteSegment::TravelMinutes(double averageSpeedKmh) const {
    if (averageSpeedKmh <= 0.0) {
        throw InvalidRouteException("Average speed must be positive");
    }
    const double hours = distanceKm_ / averageSpeedKmh;
    return hours * kMinutesPerHour;
}

std::string RouteSegment::Describe() const {
    std::ostringstream stream;
    stream << origin_.City() << " -> " << destination_.City() << ", " << std::fixed
           << std::setprecision(kDistancePrecision) << distanceKm_ << " km";
    return stream.str();
}

Route::Route(std::string id, double baseRatePerKm) : id_(std::move(id)), baseRatePerKm_(baseRatePerKm) {
    if (id_.empty() || baseRatePerKm_ < 0.0) {
        throw InvalidRouteException("Route requires an identifier and a non-negative rate");
    }
}

void Route::AddSegment(const RouteSegment& segment) {
    if (!segments_.empty() && segments_.back().Destination().Format() != segment.Origin().Format()) {
        throw InvalidRouteException("Route segment does not continue the previous segment");
    }
    segments_.push_back(segment);
}

double Route::TotalDistanceKm() const {
    double total = 0.0;
    for (const RouteSegment& segment : segments_) {
        total += segment.DistanceKm();
    }
    return total;
}

double Route::TotalTollCost() const {
    double total = 0.0;
    for (const RouteSegment& segment : segments_) {
        total += segment.TollCost();
    }
    return total;
}

double Route::EstimatedMinutes(double averageSpeedKmh) const {
    if (averageSpeedKmh <= 0.0) {
        throw InvalidRouteException("Average speed must be positive");
    }
    return (TotalDistanceKm() / averageSpeedKmh) * kMinutesPerHour;
}

bool Route::IsLongHaul() const { return TotalDistanceKm() > kLongHaulThresholdKm; }

std::string Route::Describe() const {
    std::ostringstream stream;
    stream << id_ << ": " << segments_.size() << " segments, " << std::fixed
           << std::setprecision(kDistancePrecision) << TotalDistanceKm() << " km";
    return stream.str();
}

}  // namespace logistics
