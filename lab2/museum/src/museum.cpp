#include "museum/museum.hpp"

#include <cctype>
#include <utility>

#include "museum/exceptions.hpp"

namespace museum {

Address::Address(std::string street, std::string city, std::string postalCode,
                 std::string buildingNumber)
    : street_(std::move(street)),
      city_(std::move(city)),
      postalCode_(std::move(postalCode)),
      buildingNumber_(std::move(buildingNumber)) {
    if (street_.empty() || city_.empty() || postalCode_.empty() || buildingNumber_.empty()) {
        throw InvalidAddressException("address fields must not be empty");
    }
    if (!Validate()) {
        throw InvalidAddressException("postal code must consist of six digits");
    }
}

std::string Address::Format() const {
    return street_ + " " + buildingNumber_ + ", " + postalCode_ + " " + city_;
}

bool Address::IsSameCity(const Address& other) const { return city_ == other.city_; }

bool Address::Validate() const {
    if (postalCode_.size() != 6) {
        return false;
    }
    for (const char character : postalCode_) {
        if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
            return false;
        }
    }
    return true;
}

Museum::Museum(std::string name, Address address, Budget budget)
    : name_(std::move(name)), address_(std::move(address)), budget_(std::move(budget)) {
    if (name_.empty()) {
        throw MuseumException("museum name must not be empty");
    }
}

bool Museum::AddDepartment(const std::shared_ptr<Department>& department) {
    if (department == nullptr) {
        throw InvalidDepartmentException("department must not be null");
    }
    for (const std::shared_ptr<Department>& stored : departments_) {
        if (stored->Name() == department->Name()) {
            return false;
        }
    }
    departments_.push_back(department);
    return true;
}

std::shared_ptr<Department> Museum::FindDepartment(const std::string& name) const {
    for (const std::shared_ptr<Department>& department : departments_) {
        if (department->Name() == name) {
            return department;
        }
    }
    throw DepartmentNotFoundException("unknown department: " + name);
}

bool Museum::Hire(const std::shared_ptr<Employee>& employee) {
    if (employee == nullptr) {
        throw InvalidPersonException("employee must not be null");
    }
    for (const std::shared_ptr<Employee>& stored : employees_) {
        if (stored->Id() == employee->Id()) {
            return false;
        }
    }
    employees_.push_back(employee);
    return true;
}

std::shared_ptr<Employee> Museum::FindEmployee(const std::string& id) const {
    for (const std::shared_ptr<Employee>& employee : employees_) {
        if (employee->Id() == id) {
            return employee;
        }
    }
    return nullptr;
}

bool Museum::AddExhibit(const std::shared_ptr<Exhibit>& exhibit) {
    if (exhibit == nullptr) {
        throw InvalidExhibitException("exhibit must not be null");
    }
    for (const std::shared_ptr<Exhibit>& stored : exhibits_) {
        if (stored->Id() == exhibit->Id()) {
            return false;
        }
    }
    exhibits_.push_back(exhibit);
    return true;
}

std::shared_ptr<Exhibit> Museum::FindExhibit(const std::string& id) const {
    for (const std::shared_ptr<Exhibit>& exhibit : exhibits_) {
        if (exhibit->Id() == id) {
            return exhibit;
        }
    }
    return nullptr;
}

void Museum::OpenExhibition(const Exhibition& exhibition) { exhibitions_.push_back(exhibition); }

double Museum::AnnualPayroll() const {
    double total = 0.0;
    for (const std::shared_ptr<Employee>& employee : employees_) {
        total += employee->AnnualSalary();
    }
    return total;
}

double Museum::TotalCollectionValue() const {
    double total = 0.0;
    for (const std::shared_ptr<Exhibit>& exhibit : exhibits_) {
        total += exhibit->AppraisedValue();
    }
    return total;
}

std::vector<std::shared_ptr<Exhibit>> Museum::ExhibitsByCategory(
    const std::string& category) const {
    std::vector<std::shared_ptr<Exhibit>> matches;
    for (const std::shared_ptr<Exhibit>& exhibit : exhibits_) {
        if (exhibit->Category() == category) {
            matches.push_back(exhibit);
        }
    }
    return matches;
}

}  // namespace museum
