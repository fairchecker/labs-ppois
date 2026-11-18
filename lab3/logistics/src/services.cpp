#include "logistics/services.hpp"

#include <algorithm>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kEllipsisLength = 3;
constexpr int kMinimumShortenedLength = 4;
constexpr double kExcellentOnTimePercent = 90.0;

std::string SeverityName(IncidentSeverity severity) {
    switch (severity) {
        case IncidentSeverity::Low:
            return "low";
        case IncidentSeverity::Medium:
            return "medium";
        case IncidentSeverity::High:
            return "high";
        case IncidentSeverity::Critical:
            return "critical";
    }
    return "unknown";
}

bool IsValidSeverity(IncidentSeverity severity) {
    return severity == IncidentSeverity::Low || severity == IncidentSeverity::Medium ||
           severity == IncidentSeverity::High || severity == IncidentSeverity::Critical;
}

}  // namespace

RoutePlanner::RoutePlanner(double averageSpeedKmh) : averageSpeedKmh_(averageSpeedKmh) {
    if (averageSpeedKmh_ <= 0.0) {
        throw InvalidRouteException("Route planner speed must be positive");
    }
}

void RoutePlanner::AddWaypoint(const Address& address) { waypoints_.push_back(address); }

double RoutePlanner::TotalDistanceKm() const {
    double total = 0.0;
    for (std::size_t index = 1; index < waypoints_.size(); ++index) {
        total += waypoints_[index - 1].DistanceTo(waypoints_[index]);
    }
    return total;
}

double RoutePlanner::EstimatedHours() const { return TotalDistanceKm() / averageSpeedKmh_; }

std::vector<Address> RoutePlanner::OptimizeOrder() const {
    if (waypoints_.size() < 2) {
        return waypoints_;
    }
    std::vector<Address> ordered;
    std::vector<bool> visited(waypoints_.size(), false);
    std::size_t current = 0;
    visited[current] = true;
    ordered.push_back(waypoints_[current]);
    while (ordered.size() < waypoints_.size()) {
        current = NearestUnvisited(current, visited);
        visited[current] = true;
        ordered.push_back(waypoints_[current]);
    }
    return ordered;
}

std::size_t RoutePlanner::NearestUnvisited(std::size_t current,
                                           const std::vector<bool>& visited) const {
    std::size_t best = current;
    double bestDistance = 0.0;
    for (std::size_t index = 0; index < waypoints_.size(); ++index) {
        if (visited[index]) {
            continue;
        }
        const double distance = waypoints_[current].DistanceTo(waypoints_[index]);
        if (best == current || distance < bestDistance) {
            best = index;
            bestDistance = distance;
        }
    }
    return best;
}

Notification::Notification(std::string recipientId, std::string message, std::string channel)
    : recipientId_(std::move(recipientId)),
      message_(std::move(message)),
      channel_(std::move(channel)),
      sent_(false) {
    if (recipientId_.empty() || channel_.empty()) {
        throw InvalidClientException("Notification requires a recipient and a channel");
    }
}

void Notification::Send() {
    if (message_.empty()) {
        throw InvalidClientException("Cannot send an empty notification");
    }
    sent_ = true;
}

std::string Notification::Shorten(int maxChars) const {
    if (maxChars < kMinimumShortenedLength) {
        throw InvalidClientException("Notification limit is too small");
    }
    if (static_cast<int>(message_.size()) <= maxChars) {
        return message_;
    }
    const std::size_t kept = static_cast<std::size_t>(maxChars - kEllipsisLength);
    return message_.substr(0, kept) + "...";
}

void ServiceQualityMonitor::RecordFeedback(const CustomerFeedback& feedback) {
    feedbacks_.push_back(feedback);
}

void ServiceQualityMonitor::RecordDelivery(bool late) {
    ++totalDeliveries_;
    if (late) {
        ++lateDeliveries_;
    }
}

double ServiceQualityMonitor::AverageRating() const {
    if (feedbacks_.empty()) {
        return 0.0;
    }
    double total = 0.0;
    for (const CustomerFeedback& feedback : feedbacks_) {
        total += feedback.Rating();
    }
    return total / static_cast<double>(feedbacks_.size());
}

double ServiceQualityMonitor::OnTimeRate() const {
    if (totalDeliveries_ == 0) {
        return 0.0;
    }
    const int onTime = totalDeliveries_ - lateDeliveries_;
    return static_cast<double>(onTime) / static_cast<double>(totalDeliveries_) * kServicePercentScale;
}

std::string ServiceQualityMonitor::QualityGrade() const {
    const double rating = AverageRating();
    const double onTimeRate = OnTimeRate();
    if (rating >= kExcellentQualityScore && onTimeRate >= kExcellentOnTimePercent) {
        return "A";
    }
    if (rating >= kAcceptableQualityScore) {
        return "B";
    }
    return "C";
}

IncidentReport::IncidentReport(std::string id, std::string shipmentId, std::string kind,
                               IncidentSeverity severity, int day)
    : id_(std::move(id)),
      shipmentId_(std::move(shipmentId)),
      kind_(std::move(kind)),
      severity_(severity),
      day_(day),
      resolved_(false) {
    if (id_.empty() || shipmentId_.empty() || kind_.empty() || day_ < 0 ||
        !IsValidSeverity(severity_)) {
        throw InvalidShipmentException("Incident report has an invalid identifier, kind or severity");
    }
}

void IncidentReport::Escalate() {
    if (resolved_) {
        throw InvalidShipmentException("A resolved incident cannot be escalated");
    }
    switch (severity_) {
        case IncidentSeverity::Low:
            severity_ = IncidentSeverity::Medium;
            return;
        case IncidentSeverity::Medium:
            severity_ = IncidentSeverity::High;
            return;
        case IncidentSeverity::High:
            severity_ = IncidentSeverity::Critical;
            return;
        case IncidentSeverity::Critical:
            throw InvalidShipmentException("Incident is already critical");
    }
}

void IncidentReport::Resolve() {
    if (resolved_) {
        throw InvalidShipmentException("Incident is already resolved");
    }
    resolved_ = true;
}

std::string IncidentReport::Describe() const {
    const std::string state = resolved_ ? "resolved" : "open";
    return kind_ + " (" + SeverityName(severity_) + ") for " + shipmentId_ + ", " + state;
}

}  // namespace logistics
