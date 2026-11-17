#pragma once

#include <vector>

#include "museum/visitor_feedback.hpp"

/**
 * @file feedback_board.hpp
 * @brief Aggregates visitor feedback and computes rating statistics.
 */

namespace museum {

/**
 * @class FeedbackBoard
 * @brief Bounded collection of visitor reviews.
 */
class FeedbackBoard {
    public:
    /**
     * @brief Creates a feedback board.
     * @throws FeedbackException when the capacity is not positive.
     */
    explicit FeedbackBoard(int maxSize);

    /**
     * @brief Adds feedback to the board.
     * @param feedback review to add.
     * @return true when added.
     * @throws FeedbackException when the board is full.
     */
    bool Add(const VisitorFeedback& feedback);

    /**
     * @brief Computes the average rating.
     * @return arithmetic mean of all ratings.
     * @throws FeedbackException when the board is empty.
     */
    double AverageRating() const;

    /**
     * @brief Counts complaints.
     * @return number of ratings at or below two.
     */
    std::size_t ComplaintCount() const;

    /**
     * @brief Removes every review.
     */
    void Clear();

    /// @brief Returns the number of reviews.
    std::size_t Size() const { return feedbacks_.size(); }

    private:
    /// Collected reviews.
    std::vector<VisitorFeedback> feedbacks_;
    /// Maximum number of reviews.
    int maxSize_;
};

}  // namespace museum
