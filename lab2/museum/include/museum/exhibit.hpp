#pragma once

#include <string>

/**
 * @file exhibit.hpp
 * @brief Abstract base class for every museum exhibit.
 */

namespace museum {

/**
 * @class Exhibit
 * @brief Collection item with condition, value and loan eligibility.
 */
class Exhibit {
    public:
    /**
     * @brief Creates an exhibit.
     * @throws InvalidExhibitException when the record is malformed.
     * @throws InvalidConditionException when the condition score is out of range.
     */
    Exhibit(std::string id, std::string title, std::string epoch, int conditionScore,
            double appraisedValue);

    /// @brief Virtual destructor for polymorphic collections.
    virtual ~Exhibit() = default;

    /// @brief Returns the exhibit identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the exhibit title.
    const std::string& Title() const { return title_; }

    /// @brief Returns the historical epoch.
    const std::string& Epoch() const { return epoch_; }

    /// @brief Returns the condition score in [0, 100].
    int ConditionScore() const { return conditionScore_; }

    /// @brief Returns the appraised value.
    double AppraisedValue() const { return appraisedValue_; }

    /**
     * @brief Updates the condition score after an assessment.
     * @param score score in [0, 100].
     * @throws InvalidConditionException when the score is out of range.
     */
    void AssessCondition(int score);

    /**
     * @brief Reduces the appraised value by a percentage.
     * @param percent percentage in [0, 100].
     * @throws InvalidExhibitException when the percentage is out of range.
     */
    void Depreciate(double percent);

    /**
     * @brief Checks whether the exhibit may be lent to another museum.
     * @return true when the condition score is at least kLoanableCondition.
     */
    bool IsLoanable() const;

    /**
     * @brief Returns the exhibit category.
     * @return category name implemented by derived classes.
     */
    virtual std::string Category() const = 0;

    /**
     * @brief Builds a one-line summary.
     * @return "category: title, condition N, value V".
     */
    std::string Summary() const;

    protected:
    /// Unique exhibit identifier.
    std::string id_;
    /// Exhibit title.
    std::string title_;
    /// Historical epoch of the exhibit.
    std::string epoch_;
    /// Conservation condition score in [0, 100].
    int conditionScore_;
    /// Current appraised value.
    double appraisedValue_;
};

}  // namespace museum
