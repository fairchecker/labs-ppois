#pragma once

#include <string>

#include "museum/person.hpp"

/**
 * @file employee.hpp
 * @brief Employee record with salary and seniority logic.
 */

namespace museum {

/**
 * @class Employee
 * @brief Museum employee with monthly salary and department assignment.
 */
class Employee : public Person {
    public:
    /**
     * @brief Creates an employee record.
     * @throws InvalidPersonException when salary is negative or the hire year is implausible.
     */
    Employee(std::string id, std::string fullName, int age, double monthlySalary,
             int hiredYear, std::string departmentId);

    /// @brief Virtual destructor.
    ~Employee() override = default;

    /**
     * @brief Computes the annual salary.
     * @return twelve monthly salaries.
     */
    double AnnualSalary() const;

    /**
     * @brief Raises the monthly salary by a percentage.
     * @param percent raise percentage in the range (0, 50].
     * @throws InvalidPersonException when the percentage is outside the range.
     */
    void Promote(double percent);

    /**
     * @brief Checks whether the employee has worked long enough to be senior.
     * @param currentYear current calendar year.
     * @return true when service is at least kSeniorYears.
     */
    bool IsSenior(int currentYear) const;

    /**
     * @brief Returns the employee role.
     * @return "employee" for the base class.
     */
    std::string Role() const override;

    /// @brief Returns the monthly salary.
    double MonthlySalary() const { return monthlySalary_; }

    /// @brief Returns the hire year.
    int HiredYear() const { return hiredYear_; }

    /// @brief Returns the department identifier.
    const std::string& DepartmentId() const { return departmentId_; }

    protected:
    /// Monthly salary in the museum currency.
    double monthlySalary_;
    /// Calendar year when the employee was hired.
    int hiredYear_;
    /// Identifier of the department the employee belongs to.
    std::string departmentId_;
};

}  // namespace museum
