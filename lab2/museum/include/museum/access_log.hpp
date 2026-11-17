#pragma once

#include <string>

/**
 * @file access_log.hpp
 * @brief Access control record for a museum zone.
 */

namespace museum {

/**
 * @class AccessLog
 * @brief Entry that can be granted or denied.
 */
class AccessLog {
    public:
    /**
     * @brief Creates an access record.
     * @throws SecurityException when fields are empty or the day is negative.
     */
    AccessLog(std::string personId, std::string zone, int day);

    /**
     * @brief Grants access.
     * @return true when the decision changed.
     */
    bool Grant();

    /**
     * @brief Denies access.
     * @return true when the decision changed.
     */
    bool Deny();

    /// @brief Checks whether access is granted.
    bool IsAllowed() const { return granted_; }

    /// @brief Returns the person identifier.
    const std::string& PersonId() const { return personId_; }

    /// @brief Returns the zone name.
    const std::string& Zone() const { return zone_; }

    /// @brief Returns the access day.
    int Day() const { return day_; }

    private:
    /// Identifier of the person entering.
    std::string personId_;
    /// Zone being entered.
    std::string zone_;
    /// Day of the access attempt.
    int day_;
    /// Whether access was granted.
    bool granted_ = false;
};

}  // namespace museum
