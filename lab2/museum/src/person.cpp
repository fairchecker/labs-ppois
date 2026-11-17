#include "museum/employee.hpp"

#include <utility>

#include "museum/constants.hpp"
#include "museum/exceptions.hpp"

namespace museum {

Person::Person(std::string id, std::string fullName, int age)
    : id_(std::move(id)), fullName_(std::move(fullName)), age_(age) {
    if (id_.empty() || fullName_.empty()) {
        throw InvalidPersonException("person id and name must not be empty");
    }
    if (age_ < 0) {
        throw InvalidPersonException("person age must not be negative");
    }
}

bool Person::IsAdult() const { return age_ >= kAdultAge; }

std::string Person::Describe() const {
    return Role() + ": " + fullName_ + " (" + id_ + "), age " + std::to_string(age_);
}

Employee::Employee(std::string id, std::string fullName, int age, double monthlySalary,
                   int hiredYear, std::string departmentId)
    : Person(std::move(id), std::move(fullName), age),
      monthlySalary_(monthlySalary),
      hiredYear_(hiredYear),
      departmentId_(std::move(departmentId)) {
    if (monthlySalary_ < 0.0) {
        throw InvalidPersonException("employee salary must not be negative");
    }
    if (hiredYear_ < kEarliestHireYear) {
        throw InvalidPersonException("employee hire year is implausible");
    }
    if (departmentId_.empty()) {
        throw InvalidPersonException("employee department id must not be empty");
    }
}

double Employee::AnnualSalary() const { return monthlySalary_ * 12.0; }

void Employee::Promote(double percent) {
    if (percent <= kPercentMin || percent > kMaxPromotionPercent) {
        throw InvalidPersonException("promotion percent must be in (0, 50]");
    }
    monthlySalary_ += monthlySalary_ * percent / kPercentMax;
}

bool Employee::IsSenior(int currentYear) const { return currentYear - hiredYear_ >= kSeniorYears; }

std::string Employee::Role() const { return "employee"; }

}  // namespace museum
