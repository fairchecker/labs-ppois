#pragma once

#include <string>

/**
 * @file incident.hpp
 * @brief Security incident with severity and resolution state.
 */

namespace museum {

/**
 * @class Incident
 * @brief Recorded security event that can be resolved and escalated.
 */
class Incident {
    public:
    /**
     * @brief Creates an incident.
     * @throws SecurityException when fields are invalid or severity is out of range.
     */
    Incident(std::string id, std::string type, int severity, std::string description);

    /**
     * @brief Resolves the incident.
     * @return true when the state changed.
     */
    bool Resolve();

    /**
     * @brief Checks whether the incident is critical.
     * @return true when severity is at least four.
     */
    bool IsCritical() const;

    /**
     * @brief Raises the severity by one.
     * @throws SecurityException when the severity is already maximal.
     */
    void Escalate();

    /// @brief Returns the incident identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the incident type.
    const std::string& Type() const { return type_; }

    /// @brief Returns the severity.
    int Severity() const { return severity_; }

    /// @brief Returns the description.
    const std::string& Description() const { return description_; }

    /// @brief Checks whether the incident is resolved.
    bool IsResolved() const { return resolved_; }

    private:
    /// Incident identifier.
    std::string id_;
    /// Incident type, for example "theft".
    std::string type_;
    /// Severity in [1, 5].
    int severity_;
    /// Free-form description.
    std::string description_;
    /// Whether the incident is resolved.
    bool resolved_ = false;
};

}  // namespace museum
