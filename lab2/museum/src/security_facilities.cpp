#include <algorithm>
#include <utility>

#include "museum/access_card.hpp"
#include "museum/access_log.hpp"
#include "museum/alarm_system.hpp"
#include "museum/cafe.hpp"
#include "museum/climate_log.hpp"
#include "museum/constants.hpp"
#include "museum/exceptions.hpp"
#include "museum/feedback_board.hpp"
#include "museum/incident.hpp"
#include "museum/parking_lot.hpp"
#include "museum/security_camera.hpp"
#include "museum/security_system.hpp"
#include "museum/visitor_feedback.hpp"

namespace museum {

SecurityCamera::SecurityCamera(std::string id, std::string zone, int resolutionP)
    : id_(std::move(id)), zone_(std::move(zone)), resolutionP_(resolutionP) {
    if (id_.empty() || zone_.empty()) {
        throw SecurityException("camera id and zone must not be empty");
    }
    if (resolutionP_ <= 0) {
        throw SecurityException("camera resolution must be positive");
    }
}

bool SecurityCamera::StartRecording() {
    if (recording_) {
        return false;
    }
    recording_ = true;
    return true;
}

bool SecurityCamera::StopRecording() {
    if (!recording_) {
        return false;
    }
    recording_ = false;
    return true;
}

std::string SecurityCamera::CaptureFrame() const {
    if (!recording_) {
        throw SecurityException("camera is not recording");
    }
    return "frame:" + id_;
}

bool SecurityCamera::IsHighResolution() const { return resolutionP_ >= kHighResolutionP; }

Incident::Incident(std::string id, std::string type, int severity, std::string description)
    : id_(std::move(id)),
      type_(std::move(type)),
      severity_(severity),
      description_(std::move(description)) {
    if (id_.empty() || type_.empty() || description_.empty()) {
        throw SecurityException("incident id, type and description must not be empty");
    }
    if (severity_ < kMinIncidentSeverity || severity_ > kMaxIncidentSeverity) {
        throw SecurityException("incident severity must be in [1, 5]");
    }
}

bool Incident::Resolve() {
    if (resolved_) {
        return false;
    }
    resolved_ = true;
    return true;
}

bool Incident::IsCritical() const { return severity_ >= kCriticalIncidentSeverity; }

void Incident::Escalate() {
    if (severity_ >= kMaxIncidentSeverity) {
        throw SecurityException("incident severity is already maximal");
    }
    ++severity_;
}

AccessLog::AccessLog(std::string personId, std::string zone, int day)
    : personId_(std::move(personId)), zone_(std::move(zone)), day_(day) {
    if (personId_.empty() || zone_.empty()) {
        throw SecurityException("access log person id and zone must not be empty");
    }
    if (day_ < 0) {
        throw SecurityException("access log day must not be negative");
    }
}

bool AccessLog::Grant() {
    if (granted_) {
        return false;
    }
    granted_ = true;
    return true;
}

bool AccessLog::Deny() {
    if (!granted_) {
        return false;
    }
    granted_ = false;
    return true;
}

AccessCard::AccessCard(std::string id, std::string ownerId, int level)
    : id_(std::move(id)), ownerId_(std::move(ownerId)), level_(level) {
    if (id_.empty() || ownerId_.empty()) {
        throw SecurityException("access card id and owner id must not be empty");
    }
    if (level_ < 1 || level_ > kMaxAccessLevel) {
        throw SecurityException("access card level must be in [1, 3]");
    }
}

bool AccessCard::Activate() {
    if (active_) {
        return false;
    }
    active_ = true;
    return true;
}

bool AccessCard::Deactivate() {
    if (!active_) {
        return false;
    }
    active_ = false;
    return true;
}

bool AccessCard::HasLevel(int requiredLevel) const {
    if (requiredLevel < 1 || requiredLevel > kMaxAccessLevel) {
        throw SecurityException("required access level must be in [1, 3]");
    }
    return active_ && level_ >= requiredLevel;
}

SecuritySystem::SecuritySystem(std::string disarmCode) : disarmCode_(std::move(disarmCode)) {
    if (disarmCode_.empty()) {
        throw SecurityException("disarm code must not be empty");
    }
}

bool SecuritySystem::Arm() {
    if (armed_) {
        return false;
    }
    armed_ = true;
    return true;
}

bool SecuritySystem::Disarm(const std::string& code) {
    if (code != disarmCode_) {
        throw SecurityException("wrong disarm code");
    }
    if (!armed_) {
        return false;
    }
    armed_ = false;
    return true;
}

void SecuritySystem::ReportIncident(const Incident& incident) { incidents_.push_back(incident); }

std::size_t SecuritySystem::CamerasOnline() const {
    return static_cast<std::size_t>(std::count_if(
        cameras_.begin(), cameras_.end(),
        [](const SecurityCamera& camera) { return camera.IsRecording(); }));
}

void SecuritySystem::LogAccess(const AccessLog& log) { logs_.push_back(log); }

void SecuritySystem::RequireLevel(const AccessCard& card, int requiredLevel) const {
    if (!card.HasLevel(requiredLevel)) {
        throw AccessDeniedException("access card level is insufficient");
    }
}

void SecuritySystem::AddCamera(const SecurityCamera& camera) { cameras_.push_back(camera); }

AlarmSystem::AlarmSystem(std::string code) : code_(std::move(code)) {
    if (code_.empty()) {
        throw SecurityException("alarm code must not be empty");
    }
}

bool AlarmSystem::Arm() {
    if (triggered_) {
        throw SecurityException("cannot arm a triggered alarm");
    }
    if (armed_) {
        return false;
    }
    armed_ = true;
    return true;
}

void AlarmSystem::Trigger() {
    if (!armed_) {
        throw SecurityException("cannot trigger a disarmed alarm");
    }
    triggered_ = true;
}

bool AlarmSystem::Reset(const std::string& code) {
    if (code != code_) {
        throw SecurityException("wrong alarm code");
    }
    if (!triggered_) {
        return false;
    }
    triggered_ = false;
    armed_ = false;
    return true;
}

bool AlarmSystem::Disarm(const std::string& code) {
    if (code != code_) {
        throw SecurityException("wrong alarm code");
    }
    if (!armed_) {
        return false;
    }
    armed_ = false;
    return true;
}

VisitorFeedback::VisitorFeedback(std::string visitorId, int rating, std::string comment, int day)
    : visitorId_(std::move(visitorId)), rating_(rating), comment_(std::move(comment)), day_(day) {
    if (visitorId_.empty()) {
        throw FeedbackException("feedback visitor id must not be empty");
    }
    if (rating_ < kMinRating || rating_ > kMaxRating) {
        throw InvalidRatingException("rating must be in [1, 5]");
    }
    if (day_ < 0) {
        throw FeedbackException("feedback day must not be negative");
    }
}

bool VisitorFeedback::IsPositive() const { return rating_ >= 4; }

bool VisitorFeedback::IsComplaint() const { return rating_ <= 2; }

FeedbackBoard::FeedbackBoard(int maxSize) : maxSize_(maxSize) {
    if (maxSize_ <= 0) {
        throw FeedbackException("feedback board capacity must be positive");
    }
}

bool FeedbackBoard::Add(const VisitorFeedback& feedback) {
    if (feedbacks_.size() >= static_cast<std::size_t>(maxSize_)) {
        throw FeedbackException("feedback board is full");
    }
    feedbacks_.push_back(feedback);
    return true;
}

double FeedbackBoard::AverageRating() const {
    if (feedbacks_.empty()) {
        throw FeedbackException("feedback board is empty");
    }
    double total = 0.0;
    for (const VisitorFeedback& feedback : feedbacks_) {
        total += static_cast<double>(feedback.Rating());
    }
    return total / static_cast<double>(feedbacks_.size());
}

std::size_t FeedbackBoard::ComplaintCount() const {
    return static_cast<std::size_t>(std::count_if(
        feedbacks_.begin(), feedbacks_.end(),
        [](const VisitorFeedback& feedback) { return feedback.IsComplaint(); }));
}

void FeedbackBoard::Clear() { feedbacks_.clear(); }

Cafe::Cafe(std::string name, int seats) : name_(std::move(name)), seats_(seats) {
    if (name_.empty()) {
        throw FacilityException("cafe name must not be empty");
    }
    if (seats_ <= 0) {
        throw FacilityException("cafe seats must be positive");
    }
}

bool Cafe::AddMenuItem(const ShopItem& item) {
    const bool duplicate = std::any_of(menu_.begin(), menu_.end(), [&item](const ShopItem& stored) {
        return stored.Name() == item.Name();
    });
    if (duplicate) {
        return false;
    }
    menu_.push_back(item);
    return true;
}

double Cafe::Sell(const std::string& name, int count) {
    for (ShopItem& item : menu_) {
        if (item.Name() == name) {
            const double revenue = item.Sell(count);
            revenue_ += revenue;
            return revenue;
        }
    }
    throw FacilityException("unknown menu item: " + name);
}

bool Cafe::SeatGuests(int guests) const {
    if (guests < 0) {
        throw FacilityException("guest count must not be negative");
    }
    return guests <= seats_;
}

ParkingLot::ParkingLot(int capacity, double hourlyFee)
    : capacity_(capacity), hourlyFee_(hourlyFee) {
    if (capacity_ <= 0 || hourlyFee_ < 0.0) {
        throw FacilityException("parking capacity and fee are invalid");
    }
}

void ParkingLot::Park() {
    if (IsFull()) {
        throw FacilityException("parking lot is full");
    }
    ++occupied_;
}

bool ParkingLot::Leave() {
    if (occupied_ == 0) {
        return false;
    }
    --occupied_;
    return true;
}

double ParkingLot::Fee(int hours) const {
    if (hours <= 0) {
        throw FacilityException("parking hours must be positive");
    }
    return static_cast<double>(hours) * hourlyFee_;
}

bool ParkingLot::IsFull() const { return occupied_ >= capacity_; }

ClimateLog::ClimateLog(double minTemperature, double maxTemperature)
    : minTemperature_(minTemperature), maxTemperature_(maxTemperature) {
    if (minTemperature_ < kMinClimateTemperature || maxTemperature_ > kMaxClimateTemperature ||
        minTemperature_ > maxTemperature_) {
        throw ClimateException("climate range is invalid");
    }
}

void ClimateLog::Record(double temperature) {
    if (temperature < kMinClimateTemperature || temperature > kMaxClimateTemperature) {
        throw ClimateException("temperature reading is implausible");
    }
    readings_.push_back(temperature);
}

bool ClimateLog::IsStable() const {
    if (readings_.empty()) {
        throw ClimateException("no climate readings recorded");
    }
    const auto minimum = std::min_element(readings_.begin(), readings_.end());
    const auto maximum = std::max_element(readings_.begin(), readings_.end());
    return (*maximum - *minimum) <= kStableTemperatureSwing;
}

double ClimateLog::Average() const {
    if (readings_.empty()) {
        throw ClimateException("no climate readings recorded");
    }
    double total = 0.0;
    for (const double reading : readings_) {
        total += reading;
    }
    return total / static_cast<double>(readings_.size());
}

}  // namespace museum
