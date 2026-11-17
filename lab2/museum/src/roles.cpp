#include "museum/curator.hpp"

#include <utility>

#include "museum/exceptions.hpp"
#include "museum/guide.hpp"
#include "museum/researcher.hpp"
#include "museum/restorer.hpp"

namespace museum {

namespace {

/// Validates a guide rating against the supported range.
void ValidateRating(double rating) {
    if (rating < kMinRating || rating > kMaxRating) {
        throw InvalidRatingException("rating must be in [1, 5]");
    }
}

}  // namespace

Curator::Curator(std::string id, std::string fullName, int age, double monthlySalary,
                 int hiredYear, std::string departmentId, std::string specialty)
    : Employee(std::move(id), std::move(fullName), age, monthlySalary, hiredYear,
               std::move(departmentId)),
      specialty_(std::move(specialty)) {}

std::string Curator::Role() const { return "curator"; }

bool Curator::AssignExhibit(const std::string& exhibitId) {
    if (exhibitId.empty()) {
        throw InvalidExhibitException("exhibit id must not be empty");
    }
    for (const std::string& assignedId : managedExhibitIds_) {
        if (assignedId == exhibitId) {
            return false;
        }
    }
    managedExhibitIds_.push_back(exhibitId);
    return true;
}

std::size_t Curator::ManagedExhibitCount() const { return managedExhibitIds_.size(); }

Restorer::Restorer(std::string id, std::string fullName, int age, double monthlySalary,
                   int hiredYear, std::string departmentId, std::string specialization)
    : Employee(std::move(id), std::move(fullName), age, monthlySalary, hiredYear,
               std::move(departmentId)),
      specialization_(std::move(specialization)) {}

std::string Restorer::Role() const { return "restorer"; }

bool Restorer::CanRestore(int conditionScore) const {
    if (conditionScore < kMinConditionScore || conditionScore > kMaxConditionScore) {
        throw InvalidConditionException("condition score must be in [0, 100]");
    }
    return conditionScore < kRestorableCondition;
}

void Restorer::CompleteTask() { ++completedTasks_; }

Guide::Guide(std::string id, std::string fullName, int age, double monthlySalary,
             int hiredYear, std::string departmentId, std::vector<std::string> languages,
             double rating)
    : Employee(std::move(id), std::move(fullName), age, monthlySalary, hiredYear,
               std::move(departmentId)),
      languages_(std::move(languages)),
      rating_(rating) {
    ValidateRating(rating_);
}

std::string Guide::Role() const { return "guide"; }

bool Guide::Speaks(const std::string& language) const {
    for (const std::string& spokenLanguage : languages_) {
        if (spokenLanguage == language) {
            return true;
        }
    }
    return false;
}

bool Guide::AddLanguage(const std::string& language) {
    if (language.empty()) {
        throw InvalidPersonException("language must not be empty");
    }
    if (Speaks(language)) {
        return false;
    }
    languages_.push_back(language);
    return true;
}

void Guide::UpdateRating(double rating) {
    ValidateRating(rating);
    rating_ = rating;
}

Researcher::Researcher(std::string id, std::string fullName, int age, double monthlySalary,
                       int hiredYear, std::string departmentId, std::string field)
    : Employee(std::move(id), std::move(fullName), age, monthlySalary, hiredYear,
               std::move(departmentId)),
      field_(std::move(field)) {}

std::string Researcher::Role() const { return "researcher"; }

void Researcher::Publish(const std::string& title) {
    if (title.empty()) {
        throw InvalidPersonException("publication title must not be empty");
    }
    publications_.push_back(title);
}

bool Researcher::HasTenure(int currentYear) const {
    return currentYear - HiredYear() >= kTenureYears;
}

}  // namespace museum
