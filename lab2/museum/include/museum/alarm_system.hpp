#pragma once

#include <string>

/**
 * @file alarm_system.hpp
 * @brief Emergency alarm controlled by a code.
 */

namespace museum {

/**
 * @class AlarmSystem
 * @brief Armed alarm that can be triggered and reset.
 */
class AlarmSystem {
    public:
    /**
     * @brief Creates an alarm system.
     * @throws SecurityException when the code is empty.
     */
    explicit AlarmSystem(std::string code);

    /**
     * @brief Arms the alarm.
     * @return true when the alarm changed to armed.
     * @throws SecurityException when the alarm is currently triggered.
     */
    bool Arm();

    /**
     * @brief Triggers the alarm.
     * @throws SecurityException when the alarm is not armed.
     */
    void Trigger();

    /**
     * @brief Resets a triggered alarm.
     * @param code reset code.
     * @return true when the alarm was triggered and is now reset.
     * @throws SecurityException when the code is wrong.
     */
    bool Reset(const std::string& code);

    /**
     * @brief Disarms the alarm.
     * @param code disarm code.
     * @return true when the alarm changed to disarmed.
     * @throws SecurityException when the code is wrong.
     */
    bool Disarm(const std::string& code);

    /// @brief Checks whether the alarm is armed.
    bool IsArmed() const { return armed_; }

    /// @brief Checks whether the alarm is triggered.
    bool IsTriggered() const { return triggered_; }

    private:
    /// Whether the alarm is armed.
    bool armed_ = false;
    /// Whether the alarm is currently ringing.
    bool triggered_ = false;
    /// Secret code required for reset and disarm.
    std::string code_;
};

}  // namespace museum
