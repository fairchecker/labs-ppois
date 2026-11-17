#pragma once

#include <string>
#include <vector>

#include "museum/employee.hpp"

/**
 * @file guide.hpp
 * @brief Guide role with language skills and visitor rating.
 */

namespace museum {

/**
 * @class Guide
 * @brief Employee who leads guided tours.
 */
class Guide : public Employee {
    public:
    /**
     * @brief Creates a guide with a list of spoken languages.
     * @throws InvalidRatingException when the initial rating is outside [1, 5].
     */
    Guide(std::string id, std::string fullName, int age, double monthlySalary,
          int hiredYear, std::string departmentId, std::vector<std::string> languages,
          double rating);

    /// @brief Returns the guide role.
    std::string Role() const override;

    /**
     * @brief Checks whether the guide speaks a language.
     * @param language language name to look up.
     * @return true when the language is present.
     */
    bool Speaks(const std::string& language) const;

    /**
     * @brief Adds a new language to the guide profile.
     * @param language non-empty language name.
     * @return true when added, false when already present.
     * @throws InvalidPersonException when the language name is empty.
     */
    bool AddLanguage(const std::string& language);

    /**
     * @brief Updates the visitor rating.
     * @param rating new rating in [1, 5].
     * @throws InvalidRatingException when the rating is out of range.
     */
    void UpdateRating(double rating);

    /// @brief Returns the current rating.
    double Rating() const { return rating_; }

    /// @brief Returns the number of spoken languages.
    std::size_t LanguageCount() const { return languages_.size(); }

    private:
    /// Languages spoken by the guide.
    std::vector<std::string> languages_;
    /// Average visitor rating.
    double rating_;
};

}  // namespace museum
