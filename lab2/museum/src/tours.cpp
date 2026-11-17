#include "museum/tour.hpp"

#include <algorithm>
#include <utility>

#include "museum/audio_guide.hpp"
#include "museum/constants.hpp"
#include "museum/exceptions.hpp"
#include "museum/guide.hpp"
#include "museum/tour_booking.hpp"
#include "museum/visitor_group.hpp"

namespace museum {

Tour::Tour(std::string id, std::string guideId, std::string routeName, int maxVisitors,
           double price)
    : id_(std::move(id)),
      guideId_(std::move(guideId)),
      routeName_(std::move(routeName)),
      maxVisitors_(maxVisitors),
      price_(price) {
    if (id_.empty() || guideId_.empty() || routeName_.empty()) {
        throw BookingException("tour id, guide id and route must not be empty");
    }
    if (maxVisitors_ <= 0 || price_ < 0.0) {
        throw BookingException("tour capacity and price are invalid");
    }
}

bool Tour::Book(Visitor& visitor) {
    if (IsFull()) {
        throw BookingException("tour is full");
    }
    if (!visitor.CanAfford(price_)) {
        throw BookingException("visitor cannot afford the tour");
    }
    ++bookedVisitors_;
    return true;
}

bool Tour::Cancel() {
    if (bookedVisitors_ == 0) {
        return false;
    }
    --bookedVisitors_;
    return true;
}

bool Tour::AssignGuide(const Guide& guide) {
    if (guideId_ == guide.Id()) {
        return false;
    }
    guideId_ = guide.Id();
    return true;
}

bool Tour::IsFull() const { return bookedVisitors_ >= maxVisitors_; }

double Tour::Revenue() const { return static_cast<double>(bookedVisitors_) * price_; }

TourBooking::TourBooking(std::string tourId, std::string visitorId, int day, double amount)
    : tourId_(std::move(tourId)),
      visitorId_(std::move(visitorId)),
      day_(day),
      amount_(amount) {
    if (tourId_.empty() || visitorId_.empty()) {
        throw BookingException("booking ids must not be empty");
    }
    if (day_ < 0 || amount_ < 0.0) {
        throw BookingException("booking day and amount must not be negative");
    }
}

bool TourBooking::Pay() {
    if (cancelled_) {
        throw BookingException("cannot pay for a cancelled booking");
    }
    if (paid_) {
        return false;
    }
    paid_ = true;
    return true;
}

bool TourBooking::Cancel() {
    if (paid_) {
        throw BookingException("paid bookings cannot be cancelled");
    }
    if (cancelled_) {
        return false;
    }
    cancelled_ = true;
    return true;
}

VisitorGroup::VisitorGroup(std::string id, std::string schoolName, int size,
                           std::string guideId)
    : id_(std::move(id)),
      schoolName_(std::move(schoolName)),
      size_(size),
      guideId_(std::move(guideId)) {
    if (id_.empty() || schoolName_.empty()) {
        throw BookingException("group id and school name must not be empty");
    }
    if (size_ <= 0) {
        throw BookingException("group size must be positive");
    }
}

bool VisitorGroup::AddMember(Visitor& visitor) {
    if (IsFull()) {
        return false;
    }
    const bool alreadyMember = std::find(memberIds_.begin(), memberIds_.end(), visitor.Id()) !=
                               memberIds_.end();
    if (alreadyMember) {
        return false;
    }
    memberIds_.push_back(visitor.Id());
    return true;
}

bool VisitorGroup::IsFull() const {
    return memberIds_.size() >= static_cast<std::size_t>(size_);
}

bool VisitorGroup::NeedsGuide() const { return guideId_.empty(); }

bool VisitorGroup::AssignGuide(const Guide& guide) {
    if (guideId_ == guide.Id()) {
        return false;
    }
    guideId_ = guide.Id();
    return true;
}

AudioGuide::AudioGuide(std::string id, std::string language, int batteryPercent)
    : id_(std::move(id)), language_(std::move(language)), batteryPercent_(batteryPercent) {
    if (id_.empty() || language_.empty()) {
        throw BookingException("audio guide id and language must not be empty");
    }
    if (batteryPercent_ < kMinBatteryPercent || batteryPercent_ > kMaxBatteryPercent) {
        throw BookingException("audio guide battery must be in [0, 100]");
    }
}

bool AudioGuide::Rent(Visitor& visitor) {
    if (!rentedToVisitorId_.empty()) {
        return false;
    }
    if (batteryPercent_ < kMinAudioGuideBattery) {
        throw BookingException("audio guide battery is too low");
    }
    rentedToVisitorId_ = visitor.Id();
    batteryPercent_ -= kMinAudioGuideBattery;
    return true;
}

bool AudioGuide::Return() {
    if (rentedToVisitorId_.empty()) {
        return false;
    }
    rentedToVisitorId_.clear();
    return true;
}

void AudioGuide::Charge(int minutes) {
    if (minutes <= 0) {
        throw BookingException("charging time must be positive");
    }
    batteryPercent_ += minutes / kMinutesPerBatteryPercent;
    if (batteryPercent_ > 100) {
        batteryPercent_ = 100;
    }
}

bool AudioGuide::IsAvailable() const {
    return rentedToVisitorId_.empty() && batteryPercent_ > 0;
}

}  // namespace museum
