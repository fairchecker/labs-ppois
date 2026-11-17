#pragma once

#include <string>

#include "museum/employee.hpp"

/**
 * @file restorer.hpp
 * @brief Restorer role responsible for conservation work.
 */

namespace museum {

/**
 * @class Restorer
 * @brief Employee who performs restoration tasks.
 */
class Restorer : public Employee {
    public:
    /**
     * @brief Creates a restorer.
     * @param specialization restoration specialization, for example "textiles".
     */
    Restorer(std::string id, std::string fullName, int age, double monthlySalary,
             int hiredYear, std::string departmentId, std::string specialization);

    /// @brief Returns the restorer role.
    std::string Role() const override;

    /**
     * @brief Decides whether an exhibit in the given condition needs this restorer.
     * @param conditionScore condition score in [0, 100].
     * @return true when the score is below kRestorableCondition.
     * @throws InvalidConditionException when the score is out of range.
     */
    bool CanRestore(int conditionScore) const;

    /**
     * @brief Records that one more restoration task was completed.
     */
    void CompleteTask();

    /// @brief Returns the number of completed tasks.
    int CompletedTasks() const { return completedTasks_; }

    /// @brief Returns the restoration specialization.
    const std::string& Specialization() const { return specialization_; }

    private:
    /// Restoration specialization.
    std::string specialization_;
    /// Number of completed restoration tasks.
    int completedTasks_ = 0;
};

}  // namespace museum
