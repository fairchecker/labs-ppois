#pragma once

#include <string>

#include "museum/artifact.hpp"

/**
 * @file manuscript.hpp
 * @brief Written artifact with page count and language.
 */

namespace museum {

/**
 * @class Manuscript
 * @brief Handwritten or early printed document.
 */
class Manuscript : public Artifact {
    public:
    /**
     * @brief Creates a manuscript.
     * @throws InvalidExhibitException when page count is not positive or language is empty.
     */
    Manuscript(std::string id, std::string title, std::string epoch, int conditionScore,
               double appraisedValue, double weightKg, std::string origin, int pageCount,
               std::string language, int publicationYear);

    /// @brief Returns the manuscript category.
    std::string Category() const override;

    /**
     * @brief Checks whether the manuscript is rare.
     * @return true before kRareManuscriptYear or above kRareManuscriptPages pages.
     */
    bool IsRare() const;

    /**
     * @brief Checks whether a reader can read the manuscript.
     * @param language language of the reader.
     * @return true when the manuscript language matches.
     */
    bool ReadableBy(const std::string& language) const;

    /// @brief Returns the page count.
    int PageCount() const { return pageCount_; }

    /// @brief Returns the manuscript language.
    const std::string& Language() const { return language_; }

    /// @brief Returns the publication year.
    int PublicationYear() const { return publicationYear_; }

    private:
    /// Number of pages.
    int pageCount_;
    /// Language of the text.
    std::string language_;
    /// Year of creation or printing.
    int publicationYear_;
};

}  // namespace museum
