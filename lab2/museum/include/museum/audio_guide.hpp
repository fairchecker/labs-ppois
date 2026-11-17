#pragma once

#include <string>

#include "museum/visitor.hpp"

/**
 * @file audio_guide.hpp
 * @brief Rental audio guide device.
 */

namespace museum {

/**
 * @class AudioGuide
 * @brief Language-specific audio device rented to one visitor at a time.
 */
class AudioGuide {
    public:
    /**
     * @brief Creates an audio guide.
     * @throws BookingException when fields are empty or the battery is out of range.
     */
    AudioGuide(std::string id, std::string language, int batteryPercent);

    /**
     * @brief Rents the device to a visitor.
     * @param visitor visitor receiving the device.
     * @return true when rented.
     * @throws BookingException when the battery is too low.
     */
    bool Rent(Visitor& visitor);

    /**
     * @brief Returns the device from the current visitor.
     * @return true when the device was rented.
     */
    bool Return();

    /**
     * @brief Charges the device for a number of minutes.
     * @param minutes strictly positive charging time.
     * @throws BookingException when minutes is not positive.
     */
    void Charge(int minutes);

    /**
     * @brief Checks whether the device can be rented.
     * @return true when not rented and the battery is not empty.
     */
    bool IsAvailable() const;

    /// @brief Returns the device identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the supported language.
    const std::string& Language() const { return language_; }

    /// @brief Returns the battery percentage.
    int BatteryPercent() const { return batteryPercent_; }

    /// @brief Returns the current renter identifier, empty when free.
    const std::string& RentedToVisitorId() const { return rentedToVisitorId_; }

    private:
    /// Device identifier.
    std::string id_;
    /// Language of the audio content.
    std::string language_;
    /// Battery percentage in [0, 100].
    int batteryPercent_;
    /// Identifier of the current renter, empty when free.
    std::string rentedToVisitorId_;
};

}  // namespace museum
