#pragma once

#include <string>
#include <vector>

#include "museum/access_card.hpp"
#include "museum/access_log.hpp"
#include "museum/incident.hpp"
#include "museum/security_camera.hpp"

/**
 * @file security_system.hpp
 * @brief Central security system that aggregates cameras, incidents and access.
 */

namespace museum {

/**
 * @class SecuritySystem
 * @brief Arms the museum, records incidents and enforces access levels.
 */
class SecuritySystem {
    public:
    /**
     * @brief Creates a security system with a disarm code.
     * @throws SecurityException when the code is empty.
     */
    explicit SecuritySystem(std::string disarmCode);

    /**
     * @brief Arms the security system.
     * @return true when the system changed to armed.
     */
    bool Arm();

    /**
     * @brief Disarms the security system with a code.
     * @param code disarm code.
     * @return true when the system changed to disarmed.
     * @throws SecurityException when the code is wrong.
     */
    bool Disarm(const std::string& code);

    /**
     * @brief Registers a security incident.
     * @param incident incident to record.
     */
    void ReportIncident(const Incident& incident);

    /**
     * @brief Counts cameras that are currently recording.
     * @return number of active cameras.
     */
    std::size_t CamerasOnline() const;

    /**
     * @brief Records an access decision.
     * @param log access log entry.
     */
    void LogAccess(const AccessLog& log);

    /**
     * @brief Requires a card to grant a level.
     * @param card card presented by the person.
     * @param requiredLevel level in [1, 3].
     * @throws AccessDeniedException when the card is inactive or insufficient.
     */
    void RequireLevel(const AccessCard& card, int requiredLevel) const;

    /// @brief Adds a camera to the system.
    void AddCamera(const SecurityCamera& camera);

    /// @brief Checks whether the system is armed.
    bool IsArmed() const { return armed_; }

    /// @brief Returns the number of recorded incidents.
    std::size_t IncidentCount() const { return incidents_.size(); }

    /// @brief Returns the number of access log entries.
    std::size_t LogCount() const { return logs_.size(); }

    private:
    /// Whether the system is armed.
    bool armed_ = false;
    /// Secret code required to disarm.
    std::string disarmCode_;
    /// Cameras known to the system.
    std::vector<SecurityCamera> cameras_;
    /// Recorded incidents.
    std::vector<Incident> incidents_;
    /// Access control log.
    std::vector<AccessLog> logs_;
};

}  // namespace museum
