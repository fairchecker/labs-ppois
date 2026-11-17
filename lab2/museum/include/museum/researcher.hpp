#pragma once

#include <string>
#include <vector>

#include "museum/employee.hpp"

/**
 * @file researcher.hpp
 * @brief Researcher role with publications and tenure rules.
 */

namespace museum {

/**
 * @class Researcher
 * @brief Employee who produces academic work for the museum.
 */
class Researcher : public Employee {
    public:
    /**
     * @brief Creates a researcher.
     * @param field research field, for example "paleontology".
     */
    Researcher(std::string id, std::string fullName, int age, double monthlySalary,
               int hiredYear, std::string departmentId, std::string field);

    /// @brief Returns the researcher role.
    std::string Role() const override;

    /**
     * @brief Adds a publication to the researcher profile.
     * @param title non-empty publication title.
     * @throws InvalidPersonException when the title is empty.
     */
    void Publish(const std::string& title);

    /**
     * @brief Checks whether the researcher has tenure.
     * @param currentYear current calendar year.
     * @return true after kTenureYears of service.
     */
    bool HasTenure(int currentYear) const;

    /// @brief Returns the number of publications.
    std::size_t PublicationCount() const { return publications_.size(); }

    /// @brief Returns the research field.
    const std::string& Field() const { return field_; }

    private:
    /// Research field.
    std::string field_;
    /// Titles of published works.
    std::vector<std::string> publications_;
};

}  // namespace museum
