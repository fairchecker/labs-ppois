#pragma once

#include <string>
#include <vector>

#include "museum/restoration_material.hpp"
#include "museum/restoration_task.hpp"

/**
 * @file restoration_lab.hpp
 * @brief Conservation lab that schedules tasks and tracks materials.
 */

namespace museum {

/**
 * @class RestorationLab
 * @brief Work queue for restorers with a limited number of active tasks.
 */
class RestorationLab {
    public:
    /**
     * @brief Creates a restoration lab.
     * @throws RestorationException when capacity is not positive.
     */
    explicit RestorationLab(int capacity);

    /**
     * @brief Adds a task to the laboratory queue.
     * @param task task to assign.
     * @return true when the task was accepted.
     * @throws RestorationException when the lab is full or the task id already exists.
     */
    bool AssignTask(const RestorationTask& task);

    /**
     * @brief Completes a task by identifier.
     * @param taskId task identifier.
     * @throws RestorationException when the task is unknown or cannot be completed.
     */
    void CompleteTask(const std::string& taskId);

    /**
     * @brief Adds a material stock record.
     * @param material material to add.
     * @return true when added, false when the name already exists.
     */
    bool AddMaterial(const RestorationMaterial& material);

    /**
     * @brief Consumes material by name.
     * @param name material name.
     * @param amount quantity to consume.
     * @throws RestorationException when the material is unknown or the amount is invalid.
     */
    void ConsumeMaterial(const std::string& name, double amount);

    /// @brief Counts tasks that are not completed.
    std::size_t PendingCount() const;

    /**
     * @brief Sums the labour cost of all tasks.
     * @return total labour cost.
     */
    double TotalCost() const;

    /// @brief Returns the number of assigned tasks.
    std::size_t TaskCount() const { return tasks_.size(); }

    private:
    /// Tasks managed by the laboratory.
    std::vector<RestorationTask> tasks_;
    /// Material stock available to restorers.
    std::vector<RestorationMaterial> materials_;
    /// Maximum number of simultaneous tasks.
    int capacity_;
};

}  // namespace museum
