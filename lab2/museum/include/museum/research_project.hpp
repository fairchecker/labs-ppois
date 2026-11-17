#pragma once

#include <string>
#include <vector>

#include "museum/publication.hpp"

/**
 * @file research_project.hpp
 * @brief Funded research project with findings and publications.
 */

namespace museum {

class Researcher;

/**
 * @class ResearchProject
 * @brief Investigation led by a researcher and closed when complete.
 */
class ResearchProject {
    public:
    /**
     * @brief Creates a research project.
     * @throws ResearchException when the title or lead id is empty or the budget is negative.
     */
    ResearchProject(std::string title, std::string leadResearcherId, double budget);

    /**
     * @brief Records a project finding.
     * @param finding non-empty finding text.
     * @throws ResearchException when the project is complete or the finding is empty.
     */
    void AddFinding(const std::string& finding);

    /**
     * @brief Adds a publication to the project output.
     * @param publication publication to add.
     * @throws ResearchException when the project is complete.
     */
    void AddPublication(const Publication& publication);

    /**
     * @brief Marks the project as complete.
     * @throws ResearchException when the project is already complete.
     */
    void Complete();

    /// @brief Checks whether the project has funding.
    bool IsFunded() const { return budget_ > 0.0; }

    /**
     * @brief Checks whether the project is led by a researcher.
     * @param researcher researcher to compare.
     * @return true when the lead identifier matches.
     */
    bool LedBy(const Researcher& researcher) const;

    /// @brief Checks whether the project is complete.
    bool IsComplete() const { return completed_; }

    /// @brief Returns the project title.
    const std::string& Title() const { return title_; }

    /// @brief Returns the lead researcher identifier.
    const std::string& LeadResearcherId() const { return leadResearcherId_; }

    /// @brief Returns the project budget.
    double Budget() const { return budget_; }

    /// @brief Returns the number of findings.
    std::size_t FindingCount() const { return findings_.size(); }

    /// @brief Returns the number of publications.
    std::size_t PublicationCount() const { return publications_.size(); }

    private:
    /// Project title.
    std::string title_;
    /// Identifier of the lead researcher.
    std::string leadResearcherId_;
    /// Allocated budget.
    double budget_;
    /// Recorded findings.
    std::vector<std::string> findings_;
    /// Publications produced by the project.
    std::vector<Publication> publications_;
    /// Whether the project is closed.
    bool completed_ = false;
};

}  // namespace museum
