#pragma once

#include <string>
#include <vector>

#include "museum/employee.hpp"

/**
 * @file curator.hpp
 * @brief Curator role responsible for a set of exhibits.
 */

namespace museum {

/**
 * @class Curator
 * @brief Employee who manages exhibits assigned to a department.
 */
class Curator : public Employee {
    public:
    /**
     * @brief Creates a curator.
     * @param specialty curatorial specialization, for example "Ancient Egypt".
     */
    Curator(std::string id, std::string fullName, int age, double monthlySalary,
            int hiredYear, std::string departmentId, std::string specialty);

    /// @brief Returns the curator role.
    std::string Role() const override;

    /**
     * @brief Assigns an exhibit to this curator.
     * @param exhibitId non-empty exhibit identifier.
     * @return true when the exhibit was added, false when it was already assigned.
     * @throws InvalidExhibitException when the identifier is empty.
     */
    bool AssignExhibit(const std::string& exhibitId);

    /**
     * @brief Returns how many exhibits the curator manages.
     * @return number of assigned exhibits.
     */
    std::size_t ManagedExhibitCount() const;

    /// @brief Returns the curatorial specialty.
    const std::string& Specialty() const { return specialty_; }

    private:
    /// Curatorial specialization.
    std::string specialty_;
    /// Identifiers of exhibits managed by this curator.
    std::vector<std::string> managedExhibitIds_;
};

}  // namespace museum
