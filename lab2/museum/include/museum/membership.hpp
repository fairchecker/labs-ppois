#pragma once

#include <string>

/**
 * @file membership.hpp
 * @brief Museum membership with level-based discounts.
 */

namespace museum {

class Visitor;

/**
 * @class Membership
 * @brief Paid membership that grants a discount and expires on a given day.
 */
class Membership {
    public:
    /**
     * @brief Creates a membership.
     * @param visitorId owner identifier.
     * @param level membership level in [1, 3].
     * @param validUntilDay last day on which the membership is valid.
     * @throws InvalidMembershipException when the level is unsupported or the id is empty.
     */
    Membership(std::string visitorId, int level, int validUntilDay);

    /**
     * @brief Checks whether the membership is active on a day.
     * @param day day number.
     * @return true when day is not after the validity day.
     */
    bool IsActive(int day) const;

    /**
     * @brief Returns the discount percentage for the membership level.
     * @return 5, 10 or 15 percent.
     */
    double DiscountPercent() const;

    /**
     * @brief Extends the membership by a number of days.
     * @param days strictly positive number of days.
     * @throws InvalidMembershipException when days is not positive.
     */
    void Extend(int days);

    /**
     * @brief Checks whether the membership belongs to a visitor.
     * @param visitor visitor to compare.
     * @return true when the visitor identifier matches.
     */
    bool BelongsTo(const Visitor& visitor) const;

    /// @brief Returns the owner identifier.
    const std::string& VisitorId() const { return visitorId_; }

    /// @brief Returns the membership level.
    int Level() const { return level_; }

    /// @brief Returns the last valid day.
    int ValidUntilDay() const { return validUntilDay_; }

    private:
    /// Owner visitor identifier.
    std::string visitorId_;
    /// Membership level in [1, 3].
    int level_;
    /// Last day on which the membership is valid.
    int validUntilDay_;
};

}  // namespace museum
