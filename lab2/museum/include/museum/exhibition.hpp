#pragma once

#include <string>
#include <vector>

/**
 * @file exhibition.hpp
 * @brief Temporary exhibition with a date range and ticket price.
 */

namespace museum {

class Exhibit;

/**
 * @class Exhibition
 * @brief Curated display of exhibits available for a limited period.
 */
class Exhibition {
    public:
    /**
     * @brief Creates an exhibition.
     * @throws InvalidExhibitionException when the record is malformed or the dates are inverted.
     */
    Exhibition(std::string id, std::string title, int startDay, int endDay,
               double ticketPrice);

    /**
     * @brief Adds an exhibit to the exhibition.
     * @param exhibitId non-empty identifier.
     * @return true when added, false when already present.
     * @throws InvalidExhibitException when the identifier is empty.
     */
    bool AddExhibit(const std::string& exhibitId);

    /**
     * @brief Adds an exhibit object to the exhibition.
     * @param exhibit exhibit to add.
     * @return true when added, false when already present.
     */
    bool AddExhibit(const Exhibit& exhibit);

    /**
     * @brief Checks whether the exhibition is open on a day.
     * @param day day number.
     * @return true inside the inclusive date range.
     */
    bool IsActiveOn(int day) const;

    /**
     * @brief Returns the inclusive duration in days.
     * @return end day minus start day plus one.
     */
    int DurationDays() const;

    /**
     * @brief Estimates ticket revenue for a visitor count.
     * @param visitors non-negative number of visitors.
     * @return visitors multiplied by the ticket price.
     * @throws InvalidExhibitionException when the visitor count is negative.
     */
    double ExpectedRevenue(int visitors) const;

    /// @brief Returns the number of exhibits.
    std::size_t ExhibitCount() const { return exhibitIds_.size(); }

    /// @brief Returns the exhibition identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the exhibition title.
    const std::string& Title() const { return title_; }

    /// @brief Returns the first day.
    int StartDay() const { return startDay_; }

    /// @brief Returns the last day.
    int EndDay() const { return endDay_; }

    /// @brief Returns the ticket price.
    double TicketPrice() const { return ticketPrice_; }

    private:
    /// Exhibition identifier.
    std::string id_;
    /// Exhibition title.
    std::string title_;
    /// First day of the exhibition.
    int startDay_;
    /// Last day of the exhibition.
    int endDay_;
    /// Base ticket price.
    double ticketPrice_;
    /// Identifiers of exhibited items.
    std::vector<std::string> exhibitIds_;
};

}  // namespace museum
