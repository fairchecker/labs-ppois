#pragma once

/**
 * @file parking_lot.hpp
 * @brief Museum parking lot with occupancy and hourly fees.
 */

namespace museum {

/**
 * @class ParkingLot
 * @brief Capacity-limited parking with a flat hourly fee.
 */
class ParkingLot {
    public:
    /**
     * @brief Creates a parking lot.
     * @throws FacilityException when the capacity is not positive or the fee is negative.
     */
    ParkingLot(int capacity, double hourlyFee);

    /**
     * @brief Parks one car.
     * @throws FacilityException when the lot is full.
     */
    void Park();

    /**
     * @brief Removes one car.
     * @return true when a car left the lot.
     */
    bool Leave();

    /**
     * @brief Computes the fee for a stay.
     * @param hours strictly positive number of hours.
     * @return hours multiplied by the hourly fee.
     * @throws FacilityException when hours is not positive.
     */
    double Fee(int hours) const;

    /// @brief Checks whether the lot is full.
    bool IsFull() const;

    /// @brief Returns the number of parked cars.
    int Occupied() const { return occupied_; }

    /// @brief Returns the lot capacity.
    int Capacity() const { return capacity_; }

    /// @brief Returns the hourly fee.
    double HourlyFee() const { return hourlyFee_; }

    private:
    /// Maximum number of cars.
    int capacity_;
    /// Currently parked cars.
    int occupied_ = 0;
    /// Fee charged per hour.
    double hourlyFee_;
};

}  // namespace museum
