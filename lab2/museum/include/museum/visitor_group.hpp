#pragma once

#include <string>
#include <vector>

#include "museum/visitor.hpp"

/**
 * @file visitor_group.hpp
 * @brief Organized group of visitors, usually from a school.
 */

namespace museum {

/**
 * @class VisitorGroup
 * @brief Group booking that may require a guide.
 */
class VisitorGroup {
    public:
    /**
     * @brief Creates a visitor group.
     * @throws BookingException when identifiers are empty or size is not positive.
     */
    VisitorGroup(std::string id, std::string schoolName, int size, std::string guideId);

    /**
     * @brief Adds a visitor to the group.
     * @param visitor visitor to add.
     * @return true when added, false when the group is full or the visitor is already a member.
     */
    bool AddMember(Visitor& visitor);

    /**
     * @brief Checks whether the group reached its size.
     * @return true when no places remain.
     */
    bool IsFull() const;

    /**
     * @brief Checks whether the group has no assigned guide.
     * @return true when the guide identifier is empty.
     */
    bool NeedsGuide() const;

    /// @brief Returns the number of added members.
    std::size_t MemberCount() const { return memberIds_.size(); }

    /// @brief Returns the group identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the school name.
    const std::string& SchoolName() const { return schoolName_; }

    /// @brief Returns the declared group size.
    int Size() const { return size_; }

    /// @brief Returns the assigned guide identifier.
    const std::string& GuideId() const { return guideId_; }

    private:
    /// Group identifier.
    std::string id_;
    /// School or organization name.
    std::string schoolName_;
    /// Maximum number of members.
    int size_;
    /// Identifier of the assigned guide, empty when none.
    std::string guideId_;
    /// Identifiers of added visitors.
    std::vector<std::string> memberIds_;
};

}  // namespace museum
