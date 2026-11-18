#include "logistics/tracking.hpp"

#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kFirstHour = 0;

}  // namespace

TrackPoint::TrackPoint(std::string shipmentId, GeoPoint position, int day, int hour,
                       double speedKmh)
    : shipmentId_(std::move(shipmentId)),
      position_(position),
      day_(day),
      hour_(hour),
      speedKmh_(speedKmh) {
    if (shipmentId_.empty() || day_ < 0 || hour_ < kFirstHour || hour_ >= kHoursPerDay ||
        speedKmh_ < 0.0) {
        throw TrackingException("Tracking point has an invalid shipment, day, hour or speed");
    }
}

bool TrackPoint::IsSpeeding(double limitKmh) const {
    if (limitKmh < 0.0) {
        throw TrackingException("Speed limit cannot be negative");
    }
    return speedKmh_ > limitKmh;
}

bool TrackPoint::IsSameDay(int day) const { return day_ == day; }

std::string TrackPoint::Describe() const {
    return shipmentId_ + " at " + position_.Describe() + ", " + std::to_string(speedKmh_) + " km/h";
}

void TrackingService::Record(const TrackPoint& point) {
    tracks_[point.ShipmentId()].push_back(point);
}

const TrackPoint& TrackingService::Latest(const std::string& shipmentId) const {
    return History(shipmentId).back();
}

const std::vector<TrackPoint>& TrackingService::History(const std::string& shipmentId) const {
    const auto found = tracks_.find(shipmentId);
    if (found == tracks_.end() || found->second.empty()) {
        throw TrackingException("No tracking data for shipment " + shipmentId);
    }
    return found->second;
}

double TrackingService::DistanceTravelled(const std::string& shipmentId) const {
    const std::vector<TrackPoint>& points = History(shipmentId);
    double total = 0.0;
    for (std::size_t index = 1; index < points.size(); ++index) {
        total += points[index - 1].Position().DistanceTo(points[index].Position());
    }
    return total;
}

double TrackingService::AverageSpeedKmh(const std::string& shipmentId) const {
    const std::vector<TrackPoint>& points = History(shipmentId);
    double total = 0.0;
    for (const TrackPoint& point : points) {
        total += point.SpeedKmh();
    }
    return total / static_cast<double>(points.size());
}

bool TrackingService::IsStale(const std::string& shipmentId, int currentDay, int maxAgeDays) const {
    if (maxAgeDays < 0) {
        throw TrackingException("Maximum tracking age cannot be negative");
    }
    const int lastDay = Latest(shipmentId).Day();
    return currentDay - lastDay > maxAgeDays;
}

}  // namespace logistics
