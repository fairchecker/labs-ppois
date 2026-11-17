#pragma once

#include <string>

/**
 * @file security_camera.hpp
 * @brief Museum security camera with recording state.
 */

namespace museum {

/**
 * @class SecurityCamera
 * @brief Camera assigned to a zone, with resolution metadata.
 */
class SecurityCamera {
    public:
    /**
     * @brief Creates a camera.
     * @throws SecurityException when identifiers are empty or resolution is not positive.
     */
    SecurityCamera(std::string id, std::string zone, int resolutionP);

    /**
     * @brief Starts recording.
     * @return true when recording started, false when already recording.
     */
    bool StartRecording();

    /**
     * @brief Stops recording.
     * @return true when recording stopped, false when it was not recording.
     */
    bool StopRecording();

    /**
     * @brief Captures a frame while recording.
     * @return frame label.
     * @throws SecurityException when the camera is not recording.
     */
    std::string CaptureFrame() const;

    /**
     * @brief Checks whether the camera is high definition.
     * @return true at or above kHighResolutionP.
     */
    bool IsHighResolution() const;

    /// @brief Returns the camera identifier.
    const std::string& Id() const { return id_; }

    /// @brief Returns the monitored zone.
    const std::string& Zone() const { return zone_; }

    /// @brief Checks whether the camera is recording.
    bool IsRecording() const { return recording_; }

    /// @brief Returns the resolution in pixels.
    int ResolutionP() const { return resolutionP_; }

    private:
    /// Camera identifier.
    std::string id_;
    /// Monitored zone.
    std::string zone_;
    /// Whether the camera is recording.
    bool recording_ = false;
    /// Vertical resolution in pixels.
    int resolutionP_;
};

}  // namespace museum
