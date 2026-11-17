#pragma once

#include <memory>
#include <string>
#include <vector>

#include "museum/address.hpp"
#include "museum/budget.hpp"
#include "museum/department.hpp"
#include "museum/employee.hpp"
#include "museum/exhibit.hpp"
#include "museum/exhibition.hpp"

/**
 * @file museum.hpp
 * @brief Aggregate root that owns departments, staff, exhibits and exhibitions.
 */

namespace museum {

/**
 * @class Museum
 * @brief Central domain object of the museum management system.
 */
class Museum {
    public:
    /**
     * @brief Creates a museum.
     * @throws MuseumException when the name is empty.
     */
    Museum(std::string name, Address address, Budget budget);

    /**
     * @brief Adds a department.
     * @param department department to add.
     * @return true when added, false when the name already exists.
     * @throws InvalidDepartmentException when the pointer is null.
     */
    bool AddDepartment(const std::shared_ptr<Department>& department);

    /**
     * @brief Finds a department by name.
     * @param name department name.
     * @return department pointer.
     * @throws DepartmentNotFoundException when the department is missing.
     */
    std::shared_ptr<Department> FindDepartment(const std::string& name) const;

    /**
     * @brief Hires an employee.
     * @param employee employee to hire.
     * @return true when hired, false when the id already exists.
     * @throws InvalidPersonException when the pointer is null.
     */
    bool Hire(const std::shared_ptr<Employee>& employee);

    /**
     * @brief Finds an employee by identifier.
     * @param id employee identifier.
     * @return employee pointer or nullptr.
     */
    std::shared_ptr<Employee> FindEmployee(const std::string& id) const;

    /**
     * @brief Adds an exhibit to the collection.
     * @param exhibit exhibit to add.
     * @return true when added, false when the id already exists.
     * @throws InvalidExhibitException when the pointer is null.
     */
    bool AddExhibit(const std::shared_ptr<Exhibit>& exhibit);

    /**
     * @brief Finds an exhibit by identifier.
     * @param id exhibit identifier.
     * @return exhibit pointer or nullptr.
     */
    std::shared_ptr<Exhibit> FindExhibit(const std::string& id) const;

    /**
     * @brief Registers an exhibition.
     * @param exhibition exhibition to open.
     */
    void OpenExhibition(const Exhibition& exhibition);

    /**
     * @brief Sums the annual salaries of all employees.
     * @return total annual payroll.
     */
    double AnnualPayroll() const;

    /**
     * @brief Sums the appraised value of the collection.
     * @return total collection value.
     */
    double TotalCollectionValue() const;

    /**
     * @brief Selects exhibits of a category.
     * @param category category name.
     * @return matching exhibits.
     */
    std::vector<std::shared_ptr<Exhibit>> ExhibitsByCategory(const std::string& category) const;

    /// @brief Returns the museum name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the museum address.
    const Address& Location() const { return address_; }

    /// @brief Returns the mutable budget.
    Budget& BudgetRef() { return budget_; }

    /// @brief Returns the budget.
    const Budget& BudgetRef() const { return budget_; }

    /// @brief Returns the number of employees.
    std::size_t EmployeeCount() const { return employees_.size(); }

    /// @brief Returns the number of departments.
    std::size_t DepartmentCount() const { return departments_.size(); }

    /// @brief Returns the number of exhibits.
    std::size_t ExhibitCount() const { return exhibits_.size(); }

    /// @brief Returns the number of exhibitions.
    std::size_t ExhibitionCount() const { return exhibitions_.size(); }

    /// @brief Returns the collection exhibits.
    const std::vector<std::shared_ptr<Exhibit>>& Exhibits() const { return exhibits_; }

    private:
    /// Museum name.
    std::string name_;
    /// Museum address.
    Address address_;
    /// Museum budget.
    Budget budget_;
    /// Departments.
    std::vector<std::shared_ptr<Department>> departments_;
    /// Employees.
    std::vector<std::shared_ptr<Employee>> employees_;
    /// Collection exhibits.
    std::vector<std::shared_ptr<Exhibit>> exhibits_;
    /// Registered exhibitions.
    std::vector<Exhibition> exhibitions_;
};

}  // namespace museum
