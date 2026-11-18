#pragma once

#include <string>
#include <vector>

#include "logistics/vehicles.hpp"

/**
 * @file people.hpp
 * @brief Personnel hierarchy and driver assignments.
 */

namespace logistics {

/// @brief Employee availability during a trip.
enum class DriverStatus { Available, OnTrip, OffDuty };

/**
 * @class Person
 * @brief Contact record shared by every employee.
 */
class Person {
    public:
    /**
     * @brief Creates a person.
     * @throws InvalidClientException when an identifier or name is empty.
     */
    Person(std::string id, std::string fullName, std::string phone, std::string email);

    /// @brief Virtual destructor for polymorphic personnel collections.
    virtual ~Person() = default;

    /// @brief Returns the person identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the full name.
    const std::string& FullName() const { return fullName_; }

    /// @brief Returns the phone number.
    const std::string& Phone() const { return phone_; }

    /// @brief Returns the email address.
    const std::string& Email() const { return email_; }

    /**
     * @brief Builds a contact card.
     * @return "full name (id), phone, email".
     */
    std::string Describe() const;

    /// @brief Checks whether both phone and email are present.
    bool HasContactInfo() const;

    /**
     * @brief Replaces contact details.
     * @param phone new phone number.
     * @param email new email address.
     * @throws InvalidClientException when the email has no at sign.
     */
    void UpdateContact(std::string phone, std::string email);

    protected:
    std::string id_;
    std::string fullName_;
    std::string phone_;
    std::string email_;
};

/**
 * @class Employee
 * @brief Salaried person with seniority and role reporting.
 */
class Employee : public Person {
    public:
    /**
     * @brief Creates an employee.
     * @throws InvalidClientException when the departmental record is invalid.
     */
    Employee(std::string id, std::string fullName, std::string phone, std::string email,
             std::string employeeId, int hiredYear, double monthlySalary, std::string department);

    /// @brief Returns the payroll identifier.
    const std::string& EmployeeId() const { return employeeId_; }

    /// @brief Returns the hiring year.
    int HiredYear() const { return hiredYear_; }

    /// @brief Returns the monthly salary.
    double MonthlySalary() const { return monthlySalary_; }

    /// @brief Returns the department name.
    const std::string& Department() const { return department_; }

    /// @brief Returns twelve monthly salaries.
    double AnnualSalary() const;

    /**
     * @brief Raises the salary by a percentage.
     * @param percent raise in (0, 50].
     * @throws InvalidClientException when the percentage is out of range.
     */
    void Promote(double percent);

    /**
     * @brief Checks whether the employee has accumulated enough seniority.
     * @param currentYear current calendar year.
     * @param minYears minimum years of service.
     */
    bool IsSenior(int currentYear, int minYears) const;

    /// @brief Returns the role name implemented by derived classes.
    virtual std::string Role() const { return "employee"; }

    protected:
    std::string employeeId_;
    int hiredYear_;
    double monthlySalary_;
    std::string department_;
};

/**
 * @class Driver
 * @brief Employee who operates vehicles and holds a driving license.
 */
class Driver : public Employee {
    public:
    /**
     * @brief Creates a driver.
     * @throws InvalidClientException for invalid experience or rating.
     */
    Driver(std::string id, std::string fullName, std::string phone, std::string email,
           std::string employeeId, int hiredYear, double monthlySalary, DrivingLicense license,
           int experienceYears, double rating);

    /// @brief Returns the driving license.
    const DrivingLicense& License() const { return license_; }

    /// @brief Returns the driving experience in years.
    int ExperienceYears() const { return experienceYears_; }

    /// @brief Returns the customer rating in [0, 5].
    double Rating() const { return rating_; }

    /// @brief Returns the current assignment status.
    DriverStatus Status() const { return status_; }

    /**
     * @brief Checks whether the driver may operate a vehicle category now.
     * @param category required driving license category.
     */
    bool CanDrive(const std::string& category) const;

    /**
     * @brief Marks the driver as assigned to a trip.
     * @throws DriverNotAvailableException when the driver is not available.
     */
    void BeginTrip();

    /// @brief Marks the driver as available again.
    void EndTrip();

    /**
     * @brief Updates the customer rating with a new score.
     * @param rating new rating in [0, 5].
     * @throws InvalidClientException when the rating is out of range.
     */
    void UpdateRating(double rating);

    /// @brief Returns the role name.
    std::string Role() const override { return "driver"; }

    private:
    DrivingLicense license_;
    int experienceYears_;
    double rating_;
    DriverStatus status_;
};

/**
 * @class Dispatcher
 * @brief Assigns shipments to drivers and tracks their workload.
 */
class Dispatcher : public Employee {
    public:
    /**
     * @brief Creates a dispatcher.
     * @throws InvalidClientException when the shift is empty.
     */
    Dispatcher(std::string id, std::string fullName, std::string phone, std::string email,
               std::string employeeId, int hiredYear, double monthlySalary, std::string shift);

    /// @brief Returns the working shift.
    const std::string& Shift() const { return shift_; }

    /// @brief Returns the identifiers of assigned shipments.
    const std::vector<std::string>& AssignedShipmentIds() const { return assignedShipmentIds_; }

    /**
     * @brief Assigns a shipment to the dispatcher.
     * @throws InvalidShipmentException when the shipment is already assigned.
     */
    void AssignShipment(const std::string& shipmentId);

    /**
     * @brief Releases a shipment from the dispatcher.
     * @return true when the shipment was assigned.
     */
    bool ReleaseShipment(const std::string& shipmentId);

    /**
     * @brief Checks whether the workload exceeds a limit.
     * @param maxAssignments maximum concurrent shipments.
     */
    bool IsOverloaded(int maxAssignments) const;

    /// @brief Returns the current workload.
    std::size_t Workload() const { return assignedShipmentIds_.size(); }

    /// @brief Returns the role name.
    std::string Role() const override { return "dispatcher"; }

    private:
    std::string shift_;
    std::vector<std::string> assignedShipmentIds_;
};

/**
 * @class LogisticsManager
 * @brief Approves routes within a regional budget.
 */
class LogisticsManager : public Employee {
    public:
    /**
     * @brief Creates a logistics manager.
     * @throws InvalidClientException for an empty region or negative budget.
     */
    LogisticsManager(std::string id, std::string fullName, std::string phone, std::string email,
                     std::string employeeId, int hiredYear, double monthlySalary,
                     std::string region, double budgetLimit);

    /// @brief Returns the managed region.
    const std::string& Region() const { return region_; }

    /// @brief Returns the route budget limit.
    double BudgetLimit() const { return budgetLimit_; }

    /// @brief Returns the identifiers of approved routes.
    const std::vector<std::string>& ApprovedRouteIds() const { return approvedRouteIds_; }

    /**
     * @brief Approves a route when it fits the remaining budget.
     * @throws InvalidRouteException when the cost is invalid or exceeds the budget.
     */
    void ApproveRoute(const std::string& routeId, double cost);

    /// @brief Checks whether a cost fits the budget.
    bool IsWithinBudget(double cost) const;

    /// @brief Returns the number of approved routes.
    std::size_t ApprovedCount() const { return approvedRouteIds_.size(); }

    /// @brief Returns the role name.
    std::string Role() const override { return "logistics manager"; }

    private:
    std::string region_;
    double budgetLimit_;
    std::vector<std::string> approvedRouteIds_;
};

/**
 * @class WarehouseManager
 * @brief Supervises warehouse operations on a shift.
 */
class WarehouseManager : public Employee {
    public:
    /**
     * @brief Creates a warehouse manager.
     * @throws InvalidClientException when the warehouse identifier or shift is empty.
     */
    WarehouseManager(std::string id, std::string fullName, std::string phone, std::string email,
                     std::string employeeId, int hiredYear, double monthlySalary,
                     std::string warehouseId, std::string shift);

    /// @brief Returns the managed warehouse identifier.
    const std::string& WarehouseId() const { return warehouseId_; }

    /// @brief Returns the working shift.
    const std::string& Shift() const { return shift_; }

    /// @brief Returns the number of processed operations.
    int ProcessedOperations() const { return processedOperations_; }

    /// @brief Registers one processed warehouse operation.
    void RegisterOperation();

    /**
     * @brief Checks whether the daily volume limit allows an operation.
     * @param volumeM3 requested volume.
     */
    bool CanHandle(double volumeM3) const;

    /// @brief Returns the role name.
    std::string Role() const override { return "warehouse manager"; }

    private:
    std::string warehouseId_;
    std::string shift_;
    int processedOperations_;
};

/**
 * @class Mechanic
 * @brief Performs vehicle maintenance and repairs.
 */
class Mechanic : public Employee {
    public:
    /**
     * @brief Creates a mechanic.
     * @throws InvalidClientException for an empty specialization or invalid level.
     */
    Mechanic(std::string id, std::string fullName, std::string phone, std::string email,
             std::string employeeId, int hiredYear, double monthlySalary,
             std::string specialization, int certificationLevel);

    /// @brief Returns the repair specialization.
    const std::string& Specialization() const { return specialization_; }

    /// @brief Returns the number of completed repairs.
    int CompletedRepairs() const { return completedRepairs_; }

    /// @brief Returns the certification level in [1, 3].
    int CertificationLevel() const { return certificationLevel_; }

    /**
     * @brief Checks whether the mechanic can repair a given vehicle type.
     * @param vehicleType vehicle type name.
     */
    bool CanRepair(const std::string& vehicleType) const;

    /// @brief Registers one completed repair.
    void CompleteRepair();

    /// @brief Returns the role name.
    std::string Role() const override { return "mechanic"; }

    private:
    std::string specialization_;
    int completedRepairs_;
    int certificationLevel_;
};

/**
 * @class HRManager
 * @brief Hires drivers and allocates the training budget.
 */
class HRManager : public Employee {
    public:
    /**
     * @brief Creates an HR manager.
     * @throws InvalidClientException for a negative budget or invalid rating threshold.
     */
    HRManager(std::string id, std::string fullName, std::string phone, std::string email,
              std::string employeeId, int hiredYear, double monthlySalary, double trainingBudget,
              double minRating);

    /// @brief Returns the remaining training budget.
    double TrainingBudget() const { return trainingBudget_; }

    /// @brief Returns the minimum rating required for hiring.
    double MinRating() const { return minRating_; }

    /**
     * @brief Checks whether a driver passes the hiring policy.
     * @param driver candidate driver.
     */
    bool CanHire(const Driver& driver) const;

    /**
     * @brief Spends money from the training budget.
     * @throws InsufficientFundsException when the budget is too small.
     */
    void AllocateTraining(double cost);

    /// @brief Returns the role name.
    std::string Role() const override { return "hr manager"; }

    private:
    double trainingBudget_;
    double minRating_;
};

/**
 * @class Accountant
 * @brief Applies tax rates and registers processed invoices.
 */
class Accountant : public Employee {
    public:
    /**
     * @brief Creates an accountant.
     * @throws InvalidClientException when the tax rate is out of range.
     */
    Accountant(std::string id, std::string fullName, std::string phone, std::string email,
               std::string employeeId, int hiredYear, double monthlySalary, double taxRatePercent);

    /// @brief Returns the applied tax rate.
    double TaxRatePercent() const { return taxRatePercent_; }

    /// @brief Returns the number of processed invoices.
    int ProcessedInvoices() const { return processedInvoices_; }

    /**
     * @brief Applies the tax rate to a gross amount.
     * @param grossAmount amount before tax.
     */
    double NetAmount(double grossAmount) const;

    /// @brief Registers one processed invoice.
    void RegisterInvoice();

    /// @brief Returns the role name.
    std::string Role() const override { return "accountant"; }

    private:
    double taxRatePercent_;
    int processedInvoices_;
};

}  // namespace logistics
