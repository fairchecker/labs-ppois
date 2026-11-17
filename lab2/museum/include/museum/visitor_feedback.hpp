#pragma once

#include <string>

/**
 * @file visitor_feedback.hpp
 * @brief Visitor review with a one-to-five rating.
 */

namespace museum {

class Visitor;

/**
 * @class VisitorFeedback
 * @brief Review left by a visitor after a visit.
 */
class VisitorFeedback {
    public:
    /**
     * @brief Creates visitor feedback.
     * @throws InvalidRatingException when the rating is outside [1, 5].
     * @throws FeedbackException when the visitor id is empty or the day is negative.
     */
    VisitorFeedback(std::string visitorId, int rating, std::string comment, int day);

    /**
     * @brief Checks whether the feedback is positive.
     * @return true for ratings of four or five.
     */
    bool IsPositive() const;

    /**
     * @brief Checks whether the feedback is a complaint.
     * @return true for ratings of one or two.
     */
    bool IsComplaint() const;

    /**
     * @brief Checks whether the feedback was written by a visitor.
     * @param visitor visitor to compare.
     * @return true when the visitor identifier matches.
     */
    bool WrittenBy(const Visitor& visitor) const;

    /// @brief Returns the visitor identifier.
    const std::string& VisitorId() const { return visitorId_; }

    /// @brief Returns the rating.
    int Rating() const { return rating_; }

    /// @brief Returns the comment.
    const std::string& Comment() const { return comment_; }

    /// @brief Returns the visit day.
    int Day() const { return day_; }

    private:
    /// Identifier of the reviewing visitor.
    std::string visitorId_;
    /// Rating in [1, 5].
    int rating_;
    /// Free-form comment.
    std::string comment_;
    /// Day of the visit.
    int day_;
};

}  // namespace museum
