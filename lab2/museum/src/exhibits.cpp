#include "museum/artifact.hpp"

#include <utility>

#include "museum/constants.hpp"
#include "museum/exceptions.hpp"
#include "museum/fossil.hpp"
#include "museum/jewelry.hpp"
#include "museum/manuscript.hpp"
#include "museum/painting.hpp"
#include "museum/sculpture.hpp"
#include "museum/textile.hpp"
#include "museum/weapon.hpp"

namespace museum {

Exhibit::Exhibit(std::string id, std::string title, std::string epoch, int conditionScore,
                 double appraisedValue)
    : id_(std::move(id)),
      title_(std::move(title)),
      epoch_(std::move(epoch)),
      conditionScore_(conditionScore),
      appraisedValue_(appraisedValue) {
    if (id_.empty() || title_.empty()) {
        throw InvalidExhibitException("exhibit id and title must not be empty");
    }
    if (conditionScore_ < kMinConditionScore || conditionScore_ > kMaxConditionScore) {
        throw InvalidConditionException("condition score must be in [0, 100]");
    }
    if (appraisedValue_ < 0.0) {
        throw InvalidExhibitException("appraised value must not be negative");
    }
}

void Exhibit::AssessCondition(int score) {
    if (score < kMinConditionScore || score > kMaxConditionScore) {
        throw InvalidConditionException("condition score must be in [0, 100]");
    }
    conditionScore_ = score;
}

void Exhibit::Depreciate(double percent) {
    if (percent < kPercentMin || percent > kPercentMax) {
        throw InvalidExhibitException("depreciation percent must be in [0, 100]");
    }
    appraisedValue_ -= appraisedValue_ * percent / kPercentMax;
}

bool Exhibit::IsLoanable() const { return conditionScore_ >= kLoanableCondition; }

std::string Exhibit::Summary() const {
    return Category() + ": " + title_ + ", condition " + std::to_string(conditionScore_) +
           ", value " + std::to_string(appraisedValue_);
}

Artifact::Artifact(std::string id, std::string title, std::string epoch, int conditionScore,
                   double appraisedValue, std::string material, double weightKg,
                   std::string origin)
    : Exhibit(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue),
      material_(std::move(material)),
      weightKg_(weightKg),
      origin_(std::move(origin)) {
    if (material_.empty() || origin_.empty()) {
        throw InvalidExhibitException("artifact material and origin must not be empty");
    }
    if (weightKg_ < 0.0) {
        throw InvalidExhibitException("artifact weight must not be negative");
    }
}

std::string Artifact::Category() const { return "artifact"; }

void Artifact::MoveTo(const std::string& roomId) {
    if (roomId.empty()) {
        throw InvalidExhibitException("target room id must not be empty");
    }
    locationRoomId_ = roomId;
}

bool Artifact::IsFragile() const {
    return material_ == "glass" || material_ == "ceramic" || weightKg_ < 1.0;
}

Painting::Painting(std::string id, std::string title, std::string epoch, int conditionScore,
                   double appraisedValue, double weightKg, std::string origin,
                   std::string artist, std::string technique, double widthCm, double heightCm)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               "canvas", weightKg, std::move(origin)),
      artist_(std::move(artist)),
      technique_(std::move(technique)),
      widthCm_(widthCm),
      heightCm_(heightCm) {
    if (artist_.empty()) {
        throw InvalidExhibitException("painting artist must not be empty");
    }
    if (widthCm_ <= 0.0 || heightCm_ <= 0.0) {
        throw InvalidExhibitException("painting dimensions must be positive");
    }
}

std::string Painting::Category() const { return "painting"; }

double Painting::Area() const { return widthCm_ * heightCm_; }

bool Painting::IsLarge() const { return Area() > kLargePaintingAreaCm2; }

Sculpture::Sculpture(std::string id, std::string title, std::string epoch, int conditionScore,
                     double appraisedValue, double weightKg, std::string origin,
                     std::string material, double heightCm, bool hasPedestal,
                     bool outdoorSuitable)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               std::move(material), weightKg, std::move(origin)),
      heightCm_(heightCm),
      hasPedestal_(hasPedestal),
      outdoorSuitable_(outdoorSuitable) {
    if (heightCm_ <= 0.0) {
        throw InvalidExhibitException("sculpture height must be positive");
    }
}

std::string Sculpture::Category() const { return "sculpture"; }

double Sculpture::VolumeEstimate() const {
    constexpr double kFootprintSquareMetres = 0.25;
    return heightCm_ / kCentimetresPerMetre * kFootprintSquareMetres;
}

Manuscript::Manuscript(std::string id, std::string title, std::string epoch, int conditionScore,
                       double appraisedValue, double weightKg, std::string origin, int pageCount,
                       std::string language, int publicationYear)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               "paper", weightKg, std::move(origin)),
      pageCount_(pageCount),
      language_(std::move(language)),
      publicationYear_(publicationYear) {
    if (pageCount_ <= 0) {
        throw InvalidExhibitException("manuscript page count must be positive");
    }
    if (language_.empty()) {
        throw InvalidExhibitException("manuscript language must not be empty");
    }
    if (publicationYear_ < 0) {
        throw InvalidExhibitException("manuscript year must not be negative");
    }
}

std::string Manuscript::Category() const { return "manuscript"; }

bool Manuscript::IsRare() const {
    return publicationYear_ < kRareManuscriptYear || pageCount_ > kRareManuscriptPages;
}

bool Manuscript::ReadableBy(const std::string& language) const { return language_ == language; }

Fossil::Fossil(std::string id, std::string title, std::string epoch, int conditionScore,
               double appraisedValue, double weightKg, std::string origin, std::string species,
               std::string era, double ageThousandYears)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               "stone", weightKg, std::move(origin)),
      species_(std::move(species)),
      era_(std::move(era)),
      ageThousandYears_(ageThousandYears) {
    if (species_.empty() || era_.empty()) {
        throw InvalidExhibitException("fossil species and era must not be empty");
    }
    if (ageThousandYears_ < 0.0) {
        throw InvalidExhibitException("fossil age must not be negative");
    }
}

std::string Fossil::Category() const { return "fossil"; }

bool Fossil::IsPrehistoric() const { return ageThousandYears_ > kPrehistoricThousandYears; }

std::string Fossil::EraDescription() const {
    return era_ + " era, " + std::to_string(ageThousandYears_) + " thousand years old";
}

Jewelry::Jewelry(std::string id, std::string title, std::string epoch, int conditionScore,
                 double appraisedValue, double weightKg, std::string origin,
                 std::string gemstone, double carat, bool authenticated)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               "metal", weightKg, std::move(origin)),
      gemstone_(std::move(gemstone)),
      carat_(carat),
      authenticated_(authenticated) {
    if (gemstone_.empty()) {
        throw InvalidExhibitException("jewelry gemstone must not be empty");
    }
    if (carat_ < 0.0) {
        throw InvalidExhibitException("jewelry carat weight must not be negative");
    }
}

std::string Jewelry::Category() const { return "jewelry"; }

double Jewelry::Appraise() const { return AppraisedValue() + carat_ * kCaratPremium; }

bool Jewelry::IsHighValue() const { return Appraise() >= kHighValueThreshold; }

Weapon::Weapon(std::string id, std::string title, std::string epoch, int conditionScore,
               double appraisedValue, double weightKg, std::string origin,
               std::string weaponType, int yearMade, bool licenseRequired)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               "metal", weightKg, std::move(origin)),
      weaponType_(std::move(weaponType)),
      yearMade_(yearMade),
      licenseRequired_(licenseRequired) {
    if (weaponType_.empty()) {
        throw InvalidExhibitException("weapon type must not be empty");
    }
    if (yearMade_ < 0) {
        throw InvalidExhibitException("weapon year must not be negative");
    }
}

std::string Weapon::Category() const { return "weapon"; }

bool Weapon::IsAntique(int currentYear) const {
    if (currentYear < yearMade_) {
        return false;
    }
    return currentYear - yearMade_ >= kAntiqueYears;
}

Textile::Textile(std::string id, std::string title, std::string epoch, int conditionScore,
                 double appraisedValue, double weightKg, std::string origin,
                 std::string fabric, double lengthCm, double widthCm)
    : Artifact(std::move(id), std::move(title), std::move(epoch), conditionScore, appraisedValue,
               fabric, weightKg, std::move(origin)),
      fabric_(std::move(fabric)),
      lengthCm_(lengthCm),
      widthCm_(widthCm) {
    if (fabric_.empty()) {
        throw InvalidExhibitException("textile fabric must not be empty");
    }
    if (lengthCm_ <= 0.0 || widthCm_ <= 0.0) {
        throw InvalidExhibitException("textile dimensions must be positive");
    }
}

std::string Textile::Category() const { return "textile"; }

double Textile::Area() const { return lengthCm_ * widthCm_; }

bool Textile::NeedsClimateControl() const {
    return fabric_ == "silk" || fabric_ == "wool" || fabric_ == "leather";
}

}  // namespace museum
