#pragma once

#include <string>
#include <vector>

/**
 * @file showcase.hpp
 * @brief Display case that holds a limited number of exhibits.
 */

namespace museum {

/**
 * @class Showcase
 * @brief Lit display case inside a room.
 */
class Showcase {
    public:
    /**
     * @brief Creates a showcase.
     * @throws InvalidExhibitException when the id or room id is empty or capacity is not positive.
     */
    Showcase(std::string id, std::string roomId, int capacity);

    /**
     * @brief Places an exhibit in the showcase.
     * @param exhibitId non-empty identifier.
     * @return true when placed, false when already present or the showcase is full.
     * @throws InvalidExhibitException when the identifier is empty.
     */
    bool Place(const std::string& exhibitId);

    /**
     * @brief Removes an exhibit from the showcase.
     * @param exhibitId identifier to remove.
     * @return true when removed.
     */
    bool Remove(const std::string& exhibitId);

    /// @brief Turns the display lighting on.
    void TurnOn();

    /// @brief Turns the display lighting off.
    void TurnOff();

    /// @brief Checks whether the display lighting is on.
    bool IsLit() const { return lit_; }

    /// @brief Checks whether the showcase is full.
    bool IsFull() const;

    /// @brief Returns the number of displayed exhibits.
    std::size_t ExhibitCount() const { return exhibitIds_.size(); }

    /// @brief Returns the room identifier.
    const std::string& RoomId() const { return roomId_; }

    private:
    /// Showcase identifier.
    std::string id_;
    /// Identifier of the room that contains the showcase.
    std::string roomId_;
    /// Maximum number of exhibits.
    int capacity_;
    /// Whether the display lighting is on.
    bool lit_ = false;
    /// Identifiers of displayed exhibits.
    std::vector<std::string> exhibitIds_;
};

}  // namespace museum
