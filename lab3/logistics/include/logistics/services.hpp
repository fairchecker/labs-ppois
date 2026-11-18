#pragma once

#include <string>
#include <vector>

#include "logistics/clients.hpp"
#include "logistics/geo.hpp"
#include "logistics/warehouse.hpp"

/**
 * @file services.hpp
 * @brief Operational services: route planning, notifications, quality and incidents.
 */

namespace logistics {

/// @brief Quality grade boundary for an excellent service score.
inline constexpr double kExcellentQualityScore = 4.5;
/// @brief Quality grade boundary for an acceptable service score.
inline constexpr double kAcceptableQualityScore = 3.5;
/// @brief Percentage scale shared by service metrics.
inline constexpr double kServicePercentScale = 100.0;

/**
 * @class RoutePlanner
 * @brief Builds and orders a multi-stop route.
 */
class RoutePlanner {
    public:
    /**
     * @brief Creates a planner with an average speed.
     * @throws InvalidRouteException when the speed is not positive.
     */
    explicit RoutePlanner(double averageSpeedKmh);

    /// @brief Returns the planned waypoints.
    const std::vector<Address>& Waypoints() const { return waypoints_; }

    /// @brief Returns the assumed average speed.
    double AverageSpeedKmh() const { return averageSpeedKmh_; }

    /**
     * @brief Adds a waypoint to the route.
     * @param address waypoint address.
     */
    void AddWaypoint(const Address& address);

    /// @brief Returns the distance along the waypoints.
    double TotalDistanceKm() const;

    /// @brief Returns the estimated travel time in hours.
    double EstimatedHours() const;

    /**
     * @brief Orders waypoints greedily by nearest neighbour.
     * @return reordered copy starting with the first waypoint.
     */
    std::vector<Address> OptimizeOrder() const;

    private:
    std::size_t NearestUnvisited(std::size_t current, const std::vector<bool>& visited) const;

    std::vector<Address> waypoints_;
    double averageSpeedKmh_;
};

/**
 * @class Notification
 * @brief Message delivered to a client over a channel.
 */
class Notification {
    public:
    /**
     * @brief Creates an unsent notification.
     * @throws InvalidClientException for an empty recipient or channel.
     */
    Notification(std::string recipientId, std::string message, std::string channel);

    /// @brief Returns the recipient identifier.
    const std::string& RecipientId() const { return recipientId_; }

    /// @brief Returns the message text.
    const std::string& Message() const { return message_; }

    /// @brief Returns the delivery channel.
    const std::string& Channel() const { return channel_; }

    /// @brief Checks whether the message was sent.
    bool IsSent() const { return sent_; }

    /**
     * @brief Sends the notification.
     * @throws InvalidClientException when the message is empty.
     */
    void Send();

    /**
     * @brief Shortens a copy of the message to a character limit.
     * @param maxChars maximum length, including the ellipsis.
     * @throws InvalidClientException when the limit is too small.
     */
    std::string Shorten(int maxChars) const;

    private:
    std::string recipientId_;
    std::string message_;
    std::string channel_;
    bool sent_;
};

/**
 * @class ServiceQualityMonitor
 * @brief Aggregates feedback and delivery punctuality.
 */
class ServiceQualityMonitor {
    public:
    /// @brief Creates an empty monitor.
    ServiceQualityMonitor() = default;

    /// @brief Returns the collected feedback.
    const std::vector<CustomerFeedback>& Feedbacks() const { return feedbacks_; }

    /// @brief Returns the number of late deliveries.
    int LateDeliveries() const { return lateDeliveries_; }

    /// @brief Returns the number of recorded deliveries.
    int TotalDeliveries() const { return totalDeliveries_; }

    /**
     * @brief Records feedback.
     * @param feedback feedback entry.
     */
    void RecordFeedback(const CustomerFeedback& feedback);

    /**
     * @brief Records a delivery and its punctuality.
     * @param late true when the delivery missed its slot.
     */
    void RecordDelivery(bool late);

    /// @brief Returns the average feedback rating, zero without feedback.
    double AverageRating() const;

    /// @brief Returns the on-time delivery percentage, zero without deliveries.
    double OnTimeRate() const;

    /// @brief Returns the quality grade A, B or C.
    std::string QualityGrade() const;

    private:
    std::vector<CustomerFeedback> feedbacks_;
    int lateDeliveries_ = 0;
    int totalDeliveries_ = 0;
};

/// @brief Severity of a transport incident.
enum class IncidentSeverity { Low, Medium, High, Critical };

/**
 * @class IncidentReport
 * @brief Operational incident linked to a shipment.
 */
class IncidentReport {
    public:
    /**
     * @brief Creates an unresolved incident.
     * @throws InvalidShipmentException for empty identifiers, invalid day or severity.
     */
    IncidentReport(std::string id, std::string shipmentId, std::string kind, IncidentSeverity severity,
                   int day);

    /// @brief Returns the incident identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the shipment identifier.
    const std::string& ShipmentId() const { return shipmentId_; }

    /// @brief Returns the incident kind.
    const std::string& Kind() const { return kind_; }

    /// @brief Returns the severity.
    IncidentSeverity Severity() const { return severity_; }

    /// @brief Returns the report day.
    int Day() const { return day_; }

    /// @brief Checks whether the incident is resolved.
    bool IsResolved() const { return resolved_; }

    /**
     * @brief Raises the severity by one level.
     * @throws InvalidShipmentException when the incident is already critical.
     */
    void Escalate();

    /**
     * @brief Marks the incident as resolved.
     * @throws InvalidShipmentException when it is already resolved.
     */
    void Resolve();

    /// @brief Checks whether the incident is critical.
    bool IsCritical() const { return severity_ == IncidentSeverity::Critical; }

    /// @brief Builds a one-line incident summary.
    std::string Describe() const;

    private:
    std::string id_;
    std::string shipmentId_;
    std::string kind_;
    IncidentSeverity severity_;
    int day_;
    bool resolved_;
};

}  // namespace logistics
