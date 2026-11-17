#include "museum/department.hpp"

#include <utility>

#include "museum/constants.hpp"
#include "museum/curator.hpp"
#include "museum/exceptions.hpp"
#include "museum/exhibit.hpp"
#include "museum/showcase.hpp"
#include "museum/storage_room.hpp"

namespace museum {

namespace {

/// Finds an identifier in a vector of identifiers.
bool ContainsId(const std::vector<std::string>& ids, const std::string& id) {
    for (const std::string& storedId : ids) {
        if (storedId == id) {
            return true;
        }
    }
    return false;
}

/// Removes an identifier from a vector of identifiers.
bool EraseId(std::vector<std::string>& ids, const std::string& id) {
    for (auto iterator = ids.begin(); iterator != ids.end(); ++iterator) {
        if (*iterator == id) {
            ids.erase(iterator);
            return true;
        }
    }
    return false;
}

}  // namespace

Department::Department(std::string name, int floor, std::string curatorId, int capacity)
    : name_(std::move(name)),
      floor_(floor),
      curatorId_(std::move(curatorId)),
      capacity_(capacity) {
    if (name_.empty() || curatorId_.empty()) {
        throw InvalidDepartmentException("department name and curator id must not be empty");
    }
    if (floor_ < 0) {
        throw InvalidDepartmentException("department floor must not be negative");
    }
    if (capacity_ <= 0) {
        throw InvalidDepartmentException("department capacity must be positive");
    }
}

bool Department::AddExhibit(const std::string& exhibitId) {
    if (exhibitId.empty()) {
        throw InvalidExhibitException("exhibit id must not be empty");
    }
    if (ContainsId(exhibitIds_, exhibitId) || IsFull()) {
        return false;
    }
    exhibitIds_.push_back(exhibitId);
    return true;
}

bool Department::RemoveExhibit(const std::string& exhibitId) {
    return EraseId(exhibitIds_, exhibitId);
}

void Department::RequireExhibit(const std::string& exhibitId) const {
    if (!ContainsId(exhibitIds_, exhibitId)) {
        throw ExhibitNotFoundException("exhibit not assigned to department: " + exhibitId);
    }
}

bool Department::AssignCurator(const Curator& curator) {
    if (curatorId_ == curator.Id()) {
        return false;
    }
    curatorId_ = curator.Id();
    return true;
}

bool Department::AcceptExhibit(const Exhibit& exhibit) { return AddExhibit(exhibit.Id()); }

bool Department::IsFull() const {
    return exhibitIds_.size() >= static_cast<std::size_t>(capacity_);
}

StorageRoom::StorageRoom(std::string id, int capacity, double temperature, double humidity)
    : id_(std::move(id)), capacity_(capacity), temperature_(temperature), humidity_(humidity) {
    if (id_.empty() || capacity_ <= 0) {
        throw InvalidExhibitException("storage room id must not be empty and capacity must be positive");
    }
}

bool StorageRoom::Store(const std::string& exhibitId) {
    if (exhibitId.empty()) {
        throw InvalidExhibitException("exhibit id must not be empty");
    }
    if (ContainsId(exhibitIds_, exhibitId) || IsFull()) {
        return false;
    }
    exhibitIds_.push_back(exhibitId);
    return true;
}

bool StorageRoom::Remove(const std::string& exhibitId) {
    return EraseId(exhibitIds_, exhibitId);
}

bool StorageRoom::IsFull() const {
    return exhibitIds_.size() >= static_cast<std::size_t>(capacity_);
}

bool StorageRoom::ClimateOk() const {
    const bool temperatureOk =
        temperature_ >= kMinStorageTemperature && temperature_ <= kMaxStorageTemperature;
    const bool humidityOk = humidity_ >= kMinStorageHumidity && humidity_ <= kMaxStorageHumidity;
    return temperatureOk && humidityOk;
}

Showcase::Showcase(std::string id, std::string roomId, int capacity)
    : id_(std::move(id)), roomId_(std::move(roomId)), capacity_(capacity) {
    if (id_.empty() || roomId_.empty()) {
        throw InvalidExhibitException("showcase id and room id must not be empty");
    }
    if (capacity_ <= 0) {
        throw InvalidExhibitException("showcase capacity must be positive");
    }
}

bool Showcase::Place(const std::string& exhibitId) {
    if (exhibitId.empty()) {
        throw InvalidExhibitException("exhibit id must not be empty");
    }
    if (ContainsId(exhibitIds_, exhibitId) || IsFull()) {
        return false;
    }
    exhibitIds_.push_back(exhibitId);
    return true;
}

bool Showcase::Remove(const std::string& exhibitId) {
    return EraseId(exhibitIds_, exhibitId);
}

void Showcase::TurnOn() { lit_ = true; }

void Showcase::TurnOff() { lit_ = false; }

bool Showcase::IsFull() const {
    return exhibitIds_.size() >= static_cast<std::size_t>(capacity_);
}

}  // namespace museum
