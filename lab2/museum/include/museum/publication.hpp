#pragma once

#include <string>

/**
 * @file publication.hpp
 * @brief Research publication with citation tracking.
 */

namespace museum {

/**
 * @class Publication
 * @brief Academic work authored by a museum researcher.
 */
class Publication {
    public:
    /**
     * @brief Creates a publication.
     * @throws ResearchException when the title or author is empty or the year is negative.
     */
    Publication(std::string title, std::string authorId, int year);

    /**
     * @brief Adds one citation to the publication.
     */
    void Cite();

    /**
     * @brief Checks whether the publication is recent.
     * @param currentYear current calendar year.
     * @return true when published within kRecentPublicationYears.
     */
    bool IsRecent(int currentYear) const;

    /**
     * @brief Computes a citation impact score.
     * @param currentYear current calendar year used for age normalization.
     * @return citations divided by the publication age in years.
     */
    double ImpactScore(int currentYear) const;

    /// @brief Returns the publication title.
    const std::string& Title() const { return title_; }

    /// @brief Returns the author identifier.
    const std::string& AuthorId() const { return authorId_; }

    /// @brief Returns the publication year.
    int Year() const { return year_; }

    /// @brief Returns the citation count.
    int Citations() const { return citations_; }

    private:
    /// Publication title.
    std::string title_;
    /// Identifier of the authoring researcher.
    std::string authorId_;
    /// Publication year.
    int year_;
    /// Number of citations.
    int citations_ = 0;
};

}  // namespace museum
