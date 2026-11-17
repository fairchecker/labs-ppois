#pragma once

#include <string>

/**
 * @file condition_report.hpp
 * @brief Conservation condition report for an exhibit.
 */

namespace museum {

/**
 * @class ConditionReport
 * @brief Expert assessment with a restoration recommendation.
 */
class ConditionReport {
    public:
    /**
     * @brief Creates a condition report.
     * @throws InvalidConditionException when the score is outside [0, 100].
     * @throws InvalidExhibitException when the exhibit id is empty or the day is negative.
     */
    ConditionReport(std::string exhibitId, int score, std::string notes, int day);

    /**
     * @brief Checks whether the exhibit needs urgent restoration.
     * @return true when the score is below kRestorableCondition.
     */
    bool IsCritical() const;

    /**
     * @brief Produces a restoration recommendation.
     * @return "immediate restoration", "monitor condition" or "stable".
     */
    std::string Recommendation() const;

    /// @brief Returns the exhibit identifier.
    const std::string& ExhibitId() const { return exhibitId_; }

    /// @brief Returns the condition score.
    int Score() const { return score_; }

    /// @brief Returns the expert notes.
    const std::string& Notes() const { return notes_; }

    /// @brief Returns the assessment day.
    int Day() const { return day_; }

    private:
    /// Identifier of the assessed exhibit.
    std::string exhibitId_;
    /// Condition score in [0, 100].
    int score_;
    /// Expert notes.
    std::string notes_;
    /// Assessment day.
    int day_;
};

}  // namespace museum
