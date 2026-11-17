#pragma once

#include <string>
#include <vector>

/**
 * @file storage_room.hpp
 * @brief Climate-controlled storage room for exhibits.
 */

namespace museum {

/**
 * @class StorageRoom
 * @brief Room with capacity, temperature and humidity limits.
 */
class StorageRoom {
    public:
    /**
     * @brief Creates a storage room.
     * @throws InvalidExhibitException when the id is empty or the capacity is not positive.
     */
    StorageRoom(std::string id, int capacity, double temperature, double humidity);

    /**
     * @brief Stores an exhibit in the room.
     * @param exhibitId non-empty identifier.
     * @return true when stored, false when already present or the room is full.
     * @throws InvalidExhibitException when the identifier is empty.
     */
    bool Store(const std::string& exhibitId);

    /**
     * @brief Removes an exhibit from the room.
     * @param exhibitId identifier to remove.
     * @return true when removed.
     */
    bool Remove(const std::string& exhibitId);

    /// @brief Checks whether the room reached its capacity.
    bool IsFull() const;

    /**
     * @brief Checks whether temperature and humidity are within storage limits.
     * @return true when both values are in range.
     */
    bool ClimateOk() const;

    /// @brief Returns the room identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the number of stored exhibits.
    std::size_t ExhibitCount() const { return exhibitIds_.size(); }

    /// @brief Returns the stored temperature.
    double Temperature() const { return temperature_; }

    /// @brief Returns the stored humidity.
    double Humidity() const { return humidity_; }

    private:
    /// Room identifier.
    std::string id_;
    /// Maximum number of stored exhibits.
    int capacity_;
    /// Current temperature in Celsius.
    double temperature_;
    /// Current humidity in percent.
    double humidity_;
    /// Identifiers of stored exhibits.
    std::vector<std::string> exhibitIds_;
};

}  // namespace museum
