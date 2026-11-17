#pragma once

#include <string>

#include "museum/constants.hpp"

/**
 * @file person.hpp
 * @brief Base class for every person known to the museum.
 */

namespace museum {

/**
 * @class Person
 * @brief Abstract person with identity data shared by visitors and staff.
 */
class Person {
    public:
    /**
     * @brief Creates a person record.
     * @throws InvalidPersonException when the id or name is empty or age is negative.
     */
    Person(std::string id, std::string fullName, int age);

    /// @brief Virtual destructor for polymorphic use.
    virtual ~Person() = default;

    /// @brief Returns the unique person identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the full name.
    const std::string& FullName() const { return fullName_; }

    /// @brief Returns the age in years.
    int Age() const { return age_; }

    /**
     * @brief Checks whether the person is at least the adult age.
     * @return true for adults.
     */
    bool IsAdult() const;

    /**
     * @brief Returns the domain role of the person.
     * @return role name, for example "visitor" or "guide".
     */
    virtual std::string Role() const = 0;

    /**
     * @brief Builds a human-readable description.
     * @return string in the form "role: name (id), age N".
     */
    std::string Describe() const;

    protected:
    /// Unique identifier of the person.
    std::string id_;
    /// Full name of the person.
    std::string fullName_;
    /// Age in years.
    int age_;
};

}  // namespace museum
