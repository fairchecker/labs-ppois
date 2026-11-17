#pragma once

#include <string>

/**
 * @file restoration_task.hpp
 * @brief Conservation task assigned to a restorer.
 */

namespace museum {

/**
 * @class RestorationTask
 * @brief Work item with status, hours and hourly rate.
 */
class RestorationTask {
    public:
    /**
     * @brief Creates a pending restoration task.
     * @throws RestorationException when identifiers are empty or the rate is negative.
     */
    RestorationTask(std::string id, std::string exhibitId, std::string restorerId,
                    double hourlyRate);

    /**
     * @brief Starts work on the task.
     * @throws RestorationException when the task is not pending.
     */
    void Start();

    /**
     * @brief Adds working hours to the task.
     * @param hours strictly positive number of hours.
     * @throws RestorationException when the task is not in progress or hours is not positive.
     */
    void AddHours(double hours);

    /**
     * @brief Completes the task.
     * @throws RestorationException when the task is not in progress.
     */
    void Complete();

    /**
     * @brief Computes the labour cost.
     * @return hours spent multiplied by the hourly rate.
     */
    double Cost() const;

    /// @brief Returns the task identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the exhibit identifier.
    const std::string& ExhibitId() const { return exhibitId_; }

    /// @brief Returns the restorer identifier.
    const std::string& RestorerId() const { return restorerId_; }

    /// @brief Returns the task status.
    const std::string& Status() const { return status_; }

    /// @brief Returns the hours spent.
    double HoursSpent() const { return hoursSpent_; }

    /// @brief Returns the hourly rate.
    double HourlyRate() const { return hourlyRate_; }

    private:
    /// Task identifier.
    std::string id_;
    /// Identifier of the restored exhibit.
    std::string exhibitId_;
    /// Identifier of the assigned restorer.
    std::string restorerId_;
    /// Workflow status: pending, in progress or completed.
    std::string status_;
    /// Hours spent on the task.
    double hoursSpent_ = 0.0;
    /// Labour rate per hour.
    double hourlyRate_;
};

}  // namespace museum
