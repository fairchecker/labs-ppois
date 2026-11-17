#pragma once

#include <string>
#include <vector>

/**
 * @file department.hpp
 * @brief Museum department that owns a set of exhibits.
 */

namespace museum {

class Curator;
class Exhibit;

/**
 * @class Department
 * @brief Organizational unit led by a curator.
 */
class Department {
    public:
    /**
     * @brief Creates a department.
     * @throws InvalidDepartmentException when the name is empty, the floor is negative
     *         or the capacity is not positive.
     */
    Department(std::string name, int floor, std::string curatorId, int capacity);

    /**
     * @brief Adds an exhibit identifier to the department.
     * @param exhibitId non-empty identifier.
     * @return true when added, false when already present or the department is full.
     * @throws InvalidExhibitException when the identifier is empty.
     */
    bool AddExhibit(const std::string& exhibitId);

    /**
     * @brief Removes an exhibit identifier from the department.
     * @param exhibitId identifier to remove.
     * @return true when removed.
     */
    bool RemoveExhibit(const std::string& exhibitId);

    /**
     * @brief Requires an exhibit to be present in the department.
     * @param exhibitId identifier to look up.
     * @throws ExhibitNotFoundException when the exhibit is not assigned to the department.
     */
    void RequireExhibit(const std::string& exhibitId) const;

    /**
     * @brief Assigns a curator to the department.
     * @param curator curator to assign.
     * @return true when the curator changed.
     */
    bool AssignCurator(const Curator& curator);

    /**
     * @brief Adds an exhibit object to the department.
     * @param exhibit exhibit to add.
     * @return true when added.
     */
    bool AcceptExhibit(const Exhibit& exhibit);

    /// @brief Returns the number of assigned exhibits.
    std::size_t ExhibitCount() const { return exhibitIds_.size(); }

    /// @brief Checks whether the department reached its capacity.
    bool IsFull() const;

    /// @brief Returns the department name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the floor number.
    int Floor() const { return floor_; }

    /// @brief Returns the curator identifier.
    const std::string& CuratorId() const { return curatorId_; }

    private:
    /// Department name.
    std::string name_;
    /// Floor where the department is located.
    int floor_;
    /// Identifier of the curator responsible for the department.
    std::string curatorId_;
    /// Maximum number of exhibits.
    int capacity_;
    /// Identifiers of assigned exhibits.
    std::vector<std::string> exhibitIds_;
};

}  // namespace museum
