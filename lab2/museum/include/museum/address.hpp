#pragma once

#include <string>

/**
 * @file address.hpp
 * @brief Postal address of a museum building.
 */

namespace museum {

/**
 * @class Address
 * @brief City address with formatting and validation helpers.
 */
class Address {
    public:
    /**
     * @brief Creates an address.
     * @throws InvalidAddressException when a field is empty or the postal code is invalid.
     */
    Address(std::string street, std::string city, std::string postalCode,
            std::string buildingNumber);

    /**
     * @brief Formats the address for display.
     * @return "street building, postalCode city".
     */
    std::string Format() const;

    /**
     * @brief Checks whether two addresses are in the same city.
     * @param other address to compare.
     * @return true when city names match.
     */
    bool IsSameCity(const Address& other) const;

    /**
     * @brief Validates the postal code.
     * @return true when the code consists of six digits.
     */
    bool Validate() const;

    /// @brief Returns the street name.
    const std::string& Street() const { return street_; }

    /// @brief Returns the city name.
    const std::string& City() const { return city_; }

    /// @brief Returns the postal code.
    const std::string& PostalCode() const { return postalCode_; }

    /// @brief Returns the building number.
    const std::string& BuildingNumber() const { return buildingNumber_; }

    private:
    /// Street name.
    std::string street_;
    /// City name.
    std::string city_;
    /// Postal code.
    std::string postalCode_;
    /// Building number.
    std::string buildingNumber_;
};

}  // namespace museum
