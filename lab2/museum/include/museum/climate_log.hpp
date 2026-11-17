#pragma once

#include <vector>

/**
 * @file climate_log.hpp
 * @brief Temperature history used to verify storage stability.
 */

namespace museum {

/**
 * @class ClimateLog
 * @brief Recorded temperatures with stability and average calculations.
 */
class ClimateLog {
    public:
    /**
     * @brief Creates a climate log with a configured normal range.
     * @throws ClimateException when the range is inverted or outside plausible limits.
     */
    ClimateLog(double minTemperature, double maxTemperature);

    /**
     * @brief Records a temperature reading.
     * @param temperature reading in Celsius.
     * @throws ClimateException when the reading is implausible.
     */
    void Record(double temperature);

    /**
     * @brief Checks whether recorded temperatures are stable.
     * @return true when the swing is at most kStableTemperatureSwing.
     * @throws ClimateException when no readings were recorded.
     */
    bool IsStable() const;

    /**
     * @brief Computes the average recorded temperature.
     * @return arithmetic mean.
     * @throws ClimateException when no readings were recorded.
     */
    double Average() const;

    /// @brief Returns the number of readings.
    std::size_t ReadingCount() const { return readings_.size(); }

    /// @brief Returns the configured minimum.
    double MinTemperature() const { return minTemperature_; }

    /// @brief Returns the configured maximum.
    double MaxTemperature() const { return maxTemperature_; }

    private:
    /// Recorded temperatures.
    std::vector<double> readings_;
    /// Lower bound of the normal range.
    double minTemperature_;
    /// Upper bound of the normal range.
    double maxTemperature_;
};

}  // namespace museum
