#pragma once

#include <string>

/**
 * @file access_card.hpp
 * @brief Employee access card with a clearance level.
 */

namespace museum {

/**
 * @class AccessCard
 * @brief Activatable card that grants a numbered access level.
 */
class AccessCard {
    public:
    /**
     * @brief Creates an access card.
     * @throws SecurityException when identifiers are empty or the level is out of range.
     */
    AccessCard(std::string id, std::string ownerId, int level);

    /**
     * @brief Activates the card.
     * @return true when the card was inactive.
     */
    bool Activate();

    /**
     * @brief Deactivates the card.
     * @return true when the card was active.
     */
    bool Deactivate();

    /**
     * @brief Checks whether the card grants a required level.
     * @param requiredLevel level in [1, 3].
     * @return true when the card is active and its level is sufficient.
     * @throws SecurityException when the required level is out of range.
     */
    bool HasLevel(int requiredLevel) const;

    /// @brief Returns the card identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the owner identifier.
    const std::string& OwnerId() const { return ownerId_; }

    /// @brief Returns the granted level.
    int Level() const { return level_; }

    /// @brief Checks whether the card is active.
    bool IsActive() const { return active_; }

    private:
    /// Card identifier.
    std::string id_;
    /// Identifier of the card owner.
    std::string ownerId_;
    /// Clearance level in [1, 3].
    int level_;
    /// Whether the card is active.
    bool active_ = false;
};

}  // namespace museum
