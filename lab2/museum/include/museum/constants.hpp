#pragma once

/**
 * @file constants.hpp
 * @brief Domain-wide numeric constants shared by the museum model.
 */

namespace museum {

/// Minimum age at which a person is considered an adult.
inline constexpr int kAdultAge = 18;
/// Years of service after which an employee is considered senior.
inline constexpr int kSeniorYears = 10;
/// Years of service required for tenure.
inline constexpr int kTenureYears = 7;
/// Maximum promotion percentage accepted in a single review.
inline constexpr double kMaxPromotionPercent = 50.0;
/// Lowest allowed condition score for an exhibit.
inline constexpr int kMinConditionScore = 0;
/// Highest allowed condition score for an exhibit.
inline constexpr int kMaxConditionScore = 100;
/// Condition score below which an exhibit needs restoration.
inline constexpr int kRestorableCondition = 40;
/// Condition score above which an exhibit may be lent out.
inline constexpr int kLoanableCondition = 70;
/// Lowest allowed visitor rating.
inline constexpr int kMinRating = 1;
/// Highest allowed visitor rating.
inline constexpr int kMaxRating = 5;
/// Donations at or above this amount are considered large.
inline constexpr double kLargeDonation = 10000.0;
/// Stock level at or below which a shop item is considered low.
inline constexpr int kLowStockLevel = 5;
/// Default discount for members of the first level.
inline constexpr double kBasicMembershipDiscount = 5.0;
/// Default discount for members of the second level.
inline constexpr double kGoldMembershipDiscount = 10.0;
/// Default discount for members of the third level.
inline constexpr double kPlatinumMembershipDiscount = 15.0;
/// Minimum allowed storage temperature in Celsius.
inline constexpr double kMinStorageTemperature = 16.0;
/// Maximum allowed storage temperature in Celsius.
inline constexpr double kMaxStorageTemperature = 24.0;
/// Minimum allowed storage humidity in percent.
inline constexpr double kMinStorageHumidity = 40.0;
/// Maximum allowed storage humidity in percent.
inline constexpr double kMaxStorageHumidity = 60.0;
/// Value threshold above which an item is insured separately.
inline constexpr double kHighValueThreshold = 50000.0;
/// Maximum temperature swing considered stable in a climate log.
inline constexpr double kStableTemperatureSwing = 5.0;
/// Lowest plausible ambient temperature recorded in a climate log.
inline constexpr double kMinClimateTemperature = -10.0;
/// Highest plausible ambient temperature recorded in a climate log.
inline constexpr double kMaxClimateTemperature = 50.0;
/// Discount applied to child tickets.
inline constexpr double kChildDiscount = 0.50;
/// Discount applied to student tickets.
inline constexpr double kStudentDiscount = 0.30;
/// Discount applied to senior tickets.
inline constexpr double kSeniorDiscount = 0.20;
/// Earliest plausible hire year for an employee record.
inline constexpr int kEarliestHireYear = 1900;
/// Area in square centimetres above which a painting is considered large.
inline constexpr double kLargePaintingAreaCm2 = 20000.0;
/// Age in years after which an object is considered antique.
inline constexpr int kAntiqueYears = 100;
/// Value premium added per gemstone carat during appraisal.
inline constexpr double kCaratPremium = 1000.0;
/// Publication year before which a manuscript is considered rare.
inline constexpr int kRareManuscriptYear = 1600;
/// Page count above which a manuscript is considered rare.
inline constexpr int kRareManuscriptPages = 500;
/// Hours in a standard day.
inline constexpr int kHoursPerDay = 24;
/// Days in a standard year.
inline constexpr int kDaysPerYear = 365;
/// Default capacity of the museum ticket office per day.
inline constexpr int kDefaultTicketCapacity = 500;
/// Default auditorium capacity for events.
inline constexpr int kDefaultEventCapacity = 120;

}  // namespace museum
