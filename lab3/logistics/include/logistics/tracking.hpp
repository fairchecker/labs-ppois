#pragma once

#include <map>
#include <string>
#include <vector>

#include "logistics/geo.hpp"

/**
 * @file tracking.hpp
 * @brief Shipment tracking points and history queries.
 */

namespace logistics {

/// @brief Highest accepted hour of the day.
inline constexpr int kHoursPerDay = 24;

/**
 * @class TrackPoint
 * @brief One position report emitted by a vehicle.
 */
class TrackPoint {
    public:
    /**
     * @brief Creates a tracking point.
     * @throws TrackingException for an empty shipment identifier or invalid day, hour or speed.
     */
    TrackPoint(std::string shipmentId, GeoPoint position, int day, int hour, double speedKmh);

    /// @brief Returns the shipment identifier.
    const std::string& ShipmentId() const { return shipmentId_; }

    /// @brief Returns the geographic position.
    const GeoPoint& Position() const { return position_; }

    /// @brief Returns the day of the report.
    int Day() const { return day_; }

    /// @brief Returns the hour of the report.
    int Hour() const { return hour_; }

    /// @brief Returns the reported speed.
    double SpeedKmh() const { return speedKmh_; }

    /**
     * @brief Checks whether the point exceeds a speed limit.
     * @param limitKmh speed limit in km/h.
     */
    bool IsSpeeding(double limitKmh) const;

    /**
     * @brief Checks whether the point belongs to a day.
     * @param day calendar day.
     */
    bool IsSameDay(int day) const;

    /// @brief Builds a one-line tracking description.
    std::string Describe() const;

    private:
    std::string shipmentId_;
    GeoPoint position_;
    int day_;
    int hour_;
    double speedKmh_;
};

/**
 * @class TrackingService
 * @brief Stores tracking history per shipment.
 */
class TrackingService {
    public:
    /// @brief Creates an empty tracking service.
    TrackingService() = default;

    /**
     * @brief Appends a tracking point.
     * @param point point to store.
     */
    void Record(const TrackPoint& point);

    /**
     * @brief Returns the latest point of a shipment.
     * @throws TrackingException when the shipment has no points.
     */
    const TrackPoint& Latest(const std::string& shipmentId) const;

    /**
     * @brief Returns the full history of a shipment.
     * @throws TrackingException when the shipment has no points.
     */
    const std::vector<TrackPoint>& History(const std::string& shipmentId) const;

    /**
     * @brief Total distance travelled by a shipment.
     * @throws TrackingException when the shipment has no points.
     */
    double DistanceTravelled(const std::string& shipmentId) const;

    /**
     * @brief Average reported speed of a shipment.
     * @throws TrackingException when the shipment has no points.
     */
    double AverageSpeedKmh(const std::string& shipmentId) const;

    /**
     * @brief Checks whether tracking data has not been updated for too long.
     * @param shipmentId shipment to inspect.
     * @param currentDay current calendar day.
     * @param maxAgeDays maximum accepted age in days.
     * @throws TrackingException when the shipment has no points.
     */
    bool IsStale(const std::string& shipmentId, int currentDay, int maxAgeDays) const;

    /// @brief Returns the number of tracked shipments.
    std::size_t TrackedShipments() const { return tracks_.size(); }

    private:
    std::map<std::string, std::vector<TrackPoint>> tracks_;
};

}  // namespace logistics
