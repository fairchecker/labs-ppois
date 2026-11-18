#include "logistics/people.hpp"

#include <algorithm>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr int kMinHiringYear = 1900;
constexpr int kMaxHiringYear = 2100;
constexpr int kMonthsPerYear = 12;
constexpr double kMaxPromotionPercent = 50.0;
constexpr int kMaxDrivingExperienceYears = 60;
constexpr double kMaxRating = 5.0;
constexpr double kMinBudget = 0.0;
constexpr double kMaxTaxRatePercent = 100.0;
constexpr double kDailyVolumeLimitM3 = 500.0;
constexpr int kMinCertificationLevel = 1;
constexpr int kMaxCertificationLevel = 3;
constexpr int kUniversalMechanicLevel = 3;

bool ContainsEmailSign(const std::string& email) {
    return email.find('@') != std::string::npos;
}

bool IsValidYear(int year) { return year >= kMinHiringYear && year <= kMaxHiringYear; }

}  // namespace

Person::Person(std::string id, std::string fullName, std::string phone, std::string email)
    : id_(std::move(id)),
      fullName_(std::move(fullName)),
      phone_(std::move(phone)),
      email_(std::move(email)) {
    if (id_.empty() || fullName_.empty()) {
        throw InvalidClientException("Person requires an identifier and a full name");
    }
    if (!email_.empty() && !ContainsEmailSign(email_)) {
        throw InvalidClientException("Person email must contain an at sign");
    }
}

std::string Person::Describe() const {
    return fullName_ + " (" + id_ + "), " + phone_ + ", " + email_;
}

bool Person::HasContactInfo() const { return !phone_.empty() && !email_.empty(); }

void Person::UpdateContact(std::string phone, std::string email) {
    if (email.empty() || !ContainsEmailSign(email)) {
        throw InvalidClientException("Updated email must contain an at sign");
    }
    phone_ = std::move(phone);
    email_ = std::move(email);
}

Employee::Employee(std::string id, std::string fullName, std::string phone, std::string email,
                   std::string employeeId, int hiredYear, double monthlySalary,
                   std::string department)
    : Person(std::move(id), std::move(fullName), std::move(phone), std::move(email)),
      employeeId_(std::move(employeeId)),
      hiredYear_(hiredYear),
      monthlySalary_(monthlySalary),
      department_(std::move(department)) {
    if (employeeId_.empty() || department_.empty() || !IsValidYear(hiredYear_) ||
        monthlySalary_ < 0.0) {
        throw InvalidClientException("Employee has an invalid payroll record");
    }
}

double Employee::AnnualSalary() const { return monthlySalary_ * kMonthsPerYear; }

void Employee::Promote(double percent) {
    if (percent <= 0.0 || percent > kMaxPromotionPercent) {
        throw InvalidClientException("Promotion percentage is out of range");
    }
    monthlySalary_ += monthlySalary_ * (percent / kPercentScale);
}

bool Employee::IsSenior(int currentYear, int minYears) const {
    if (currentYear < hiredYear_ || minYears < 0) {
        return false;
    }
    return currentYear - hiredYear_ >= minYears;
}

Driver::Driver(std::string id, std::string fullName, std::string phone, std::string email,
               std::string employeeId, int hiredYear, double monthlySalary, DrivingLicense license,
               int experienceYears, double rating)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "transport"),
      license_(std::move(license)),
      experienceYears_(experienceYears),
      rating_(rating),
      status_(DriverStatus::Available) {
    if (experienceYears_ < 0 || experienceYears_ > kMaxDrivingExperienceYears ||
        rating_ < 0.0 || rating_ > kMaxRating) {
        throw InvalidClientException("Driver experience or rating is out of range");
    }
}

bool Driver::CanDrive(const std::string& category) const {
    return status_ == DriverStatus::Available && license_.AllowsCategory(category);
}

void Driver::BeginTrip() {
    if (status_ != DriverStatus::Available) {
        throw DriverNotAvailableException("Driver is not available for a trip");
    }
    status_ = DriverStatus::OnTrip;
}

void Driver::EndTrip() {
    if (status_ != DriverStatus::OnTrip) {
        throw DriverNotAvailableException("Driver is not currently on a trip");
    }
    status_ = DriverStatus::Available;
}

void Driver::UpdateRating(double rating) {
    if (rating < 0.0 || rating > kMaxRating) {
        throw InvalidClientException("Driver rating is out of range");
    }
    rating_ = rating;
}

Dispatcher::Dispatcher(std::string id, std::string fullName, std::string phone, std::string email,
                       std::string employeeId, int hiredYear, double monthlySalary,
                       std::string shift)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "dispatch"),
      shift_(std::move(shift)) {
    if (shift_.empty()) {
        throw InvalidClientException("Dispatcher requires a working shift");
    }
}

void Dispatcher::AssignShipment(const std::string& shipmentId) {
    const auto found = std::find(assignedShipmentIds_.begin(), assignedShipmentIds_.end(), shipmentId);
    if (found != assignedShipmentIds_.end()) {
        throw InvalidShipmentException("Shipment is already assigned to the dispatcher");
    }
    assignedShipmentIds_.push_back(shipmentId);
}

bool Dispatcher::ReleaseShipment(const std::string& shipmentId) {
    const auto found = std::find(assignedShipmentIds_.begin(), assignedShipmentIds_.end(), shipmentId);
    if (found == assignedShipmentIds_.end()) {
        return false;
    }
    assignedShipmentIds_.erase(found);
    return true;
}

bool Dispatcher::IsOverloaded(int maxAssignments) const {
    return maxAssignments >= 0 && static_cast<int>(assignedShipmentIds_.size()) > maxAssignments;
}

LogisticsManager::LogisticsManager(std::string id, std::string fullName, std::string phone,
                                   std::string email, std::string employeeId, int hiredYear,
                                   double monthlySalary, std::string region, double budgetLimit)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "logistics"),
      region_(std::move(region)),
      budgetLimit_(budgetLimit) {
    if (region_.empty() || budgetLimit_ < kMinBudget) {
        throw InvalidClientException("Logistics manager requires a region and non-negative budget");
    }
}

void LogisticsManager::ApproveRoute(const std::string& routeId, double cost) {
    if (routeId.empty() || cost <= 0.0) {
        throw InvalidRouteException("Route approval requires an identifier and positive cost");
    }
    if (!IsWithinBudget(cost)) {
        throw InvalidRouteException("Route cost exceeds the regional budget");
    }
    const auto found = std::find(approvedRouteIds_.begin(), approvedRouteIds_.end(), routeId);
    if (found != approvedRouteIds_.end()) {
        throw InvalidRouteException("Route is already approved");
    }
    approvedRouteIds_.push_back(routeId);
}

bool LogisticsManager::IsWithinBudget(double cost) const {
    return cost > 0.0 && cost <= budgetLimit_;
}

WarehouseManager::WarehouseManager(std::string id, std::string fullName, std::string phone,
                                   std::string email, std::string employeeId, int hiredYear,
                                   double monthlySalary, std::string warehouseId, std::string shift)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "warehouse"),
      warehouseId_(std::move(warehouseId)),
      shift_(std::move(shift)),
      processedOperations_(0) {
    if (warehouseId_.empty() || shift_.empty()) {
        throw InvalidClientException("Warehouse manager requires a warehouse and shift");
    }
}

void WarehouseManager::RegisterOperation() { ++processedOperations_; }

bool WarehouseManager::CanHandle(double volumeM3) const {
    return volumeM3 > 0.0 && volumeM3 <= kDailyVolumeLimitM3;
}

Mechanic::Mechanic(std::string id, std::string fullName, std::string phone, std::string email,
                   std::string employeeId, int hiredYear, double monthlySalary,
                   std::string specialization, int certificationLevel)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "workshop"),
      specialization_(std::move(specialization)),
      completedRepairs_(0),
      certificationLevel_(certificationLevel) {
    const bool levelValid =
        certificationLevel_ >= kMinCertificationLevel && certificationLevel_ <= kMaxCertificationLevel;
    if (specialization_.empty() || !levelValid) {
        throw InvalidClientException("Mechanic requires a specialization and valid level");
    }
}

bool Mechanic::CanRepair(const std::string& vehicleType) const {
    return certificationLevel_ >= kUniversalMechanicLevel || specialization_ == vehicleType;
}

void Mechanic::CompleteRepair() { ++completedRepairs_; }

HRManager::HRManager(std::string id, std::string fullName, std::string phone, std::string email,
                     std::string employeeId, int hiredYear, double monthlySalary,
                     double trainingBudget, double minRating)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "human resources"),
      trainingBudget_(trainingBudget),
      minRating_(minRating) {
    if (trainingBudget_ < kMinBudget || minRating_ < 0.0 || minRating_ > kMaxRating) {
        throw InvalidClientException("HR manager requires a valid budget and rating threshold");
    }
}

bool HRManager::CanHire(const Driver& driver) const {
    return driver.Rating() >= minRating_ &&
           driver.ExperienceYears() >= kMinDrivingExperienceYears;
}

void HRManager::AllocateTraining(double cost) {
    if (cost <= 0.0) {
        throw InvalidClientException("Training cost must be positive");
    }
    if (cost > trainingBudget_) {
        throw InsufficientFundsException("Training budget is insufficient");
    }
    trainingBudget_ -= cost;
}

Accountant::Accountant(std::string id, std::string fullName, std::string phone, std::string email,
                       std::string employeeId, int hiredYear, double monthlySalary,
                       double taxRatePercent)
    : Employee(std::move(id), std::move(fullName), std::move(phone), std::move(email),
               std::move(employeeId), hiredYear, monthlySalary, "finance"),
      taxRatePercent_(taxRatePercent),
      processedInvoices_(0) {
    if (taxRatePercent_ < kMinBudget || taxRatePercent_ > kMaxTaxRatePercent) {
        throw InvalidClientException("Accountant tax rate is out of range");
    }
}

double Accountant::NetAmount(double grossAmount) const {
    if (grossAmount < 0.0) {
        throw InvalidClientException("Gross amount cannot be negative");
    }
    return grossAmount * (1.0 - taxRatePercent_ / kPercentScale);
}

void Accountant::RegisterInvoice() { ++processedInvoices_; }

}  // namespace logistics
