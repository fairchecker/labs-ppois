#include <algorithm>
#include <utility>

#include "museum/book.hpp"
#include "museum/condition_report.hpp"
#include "museum/constants.hpp"
#include "museum/exceptions.hpp"
#include "museum/exhibit.hpp"
#include "museum/library.hpp"
#include "museum/publication.hpp"
#include "museum/research_project.hpp"
#include "museum/researcher.hpp"
#include "museum/restoration_lab.hpp"

namespace museum {

Book::Book(std::string title, std::string author, std::string isbn)
    : title_(std::move(title)), author_(std::move(author)), isbn_(std::move(isbn)) {
    if (title_.empty() || author_.empty() || isbn_.empty()) {
        throw LibraryException("book title, author and isbn must not be empty");
    }
}

bool Book::Borrow() {
    if (!available_) {
        return false;
    }
    available_ = false;
    return true;
}

bool Book::Return() {
    if (available_) {
        return false;
    }
    available_ = true;
    return true;
}

Library::Library(int capacity) : capacity_(capacity) {
    if (capacity_ <= 0) {
        throw LibraryException("library capacity must be positive");
    }
}

bool Library::AddBook(const Book& book) {
    if (books_.size() >= static_cast<std::size_t>(capacity_)) {
        return false;
    }
    const bool duplicate = std::any_of(books_.begin(), books_.end(), [&book](const Book& stored) {
        return stored.Isbn() == book.Isbn();
    });
    if (duplicate) {
        return false;
    }
    books_.push_back(book);
    return true;
}

bool Library::LendBook(const std::string& isbn) {
    for (Book& book : books_) {
        if (book.Isbn() == isbn) {
            return book.Borrow();
        }
    }
    throw LibraryException("unknown book isbn: " + isbn);
}

bool Library::ReturnBook(const std::string& isbn) {
    for (Book& book : books_) {
        if (book.Isbn() == isbn) {
            return book.Return();
        }
    }
    throw LibraryException("unknown book isbn: " + isbn);
}

std::size_t Library::AvailableCount() const {
    return static_cast<std::size_t>(std::count_if(
        books_.begin(), books_.end(), [](const Book& book) { return book.IsAvailable(); }));
}

Publication::Publication(std::string title, std::string authorId, int year)
    : title_(std::move(title)), authorId_(std::move(authorId)), year_(year) {
    if (title_.empty() || authorId_.empty()) {
        throw ResearchException("publication title and author id must not be empty");
    }
    if (year_ < 0) {
        throw ResearchException("publication year must not be negative");
    }
}

void Publication::Cite() { ++citations_; }

bool Publication::IsRecent(int currentYear) const {
    return currentYear - year_ <= kRecentPublicationYears;
}

double Publication::ImpactScore(int currentYear) const {
    const int age = std::max(kMinimumPublicationAge, currentYear - year_);
    return static_cast<double>(citations_) / static_cast<double>(age);
}

bool Publication::AuthoredBy(const Researcher& researcher) const {
    return authorId_ == researcher.Id();
}

ConditionReport::ConditionReport(std::string exhibitId, int score, std::string notes, int day)
    : exhibitId_(std::move(exhibitId)), score_(score), notes_(std::move(notes)), day_(day) {
    if (exhibitId_.empty()) {
        throw InvalidExhibitException("condition report exhibit id must not be empty");
    }
    if (score_ < kMinConditionScore || score_ > kMaxConditionScore) {
        throw InvalidConditionException("condition score must be in [0, 100]");
    }
    if (day_ < 0) {
        throw InvalidExhibitException("condition report day must not be negative");
    }
}

bool ConditionReport::IsCritical() const { return score_ < kRestorableCondition; }

std::string ConditionReport::Recommendation() const {
    if (IsCritical()) {
        return "immediate restoration";
    }
    if (score_ < kLoanableCondition) {
        return "monitor condition";
    }
    return "stable";
}

bool ConditionReport::Concerns(const Exhibit& exhibit) const {
    return exhibitId_ == exhibit.Id();
}

ResearchProject::ResearchProject(std::string title, std::string leadResearcherId, double budget)
    : title_(std::move(title)), leadResearcherId_(std::move(leadResearcherId)), budget_(budget) {
    if (title_.empty() || leadResearcherId_.empty()) {
        throw ResearchException("project title and lead researcher id must not be empty");
    }
    if (budget_ < 0.0) {
        throw ResearchException("project budget must not be negative");
    }
}

void ResearchProject::AddFinding(const std::string& finding) {
    if (completed_) {
        throw ResearchException("cannot add findings to a completed project");
    }
    if (finding.empty()) {
        throw ResearchException("finding must not be empty");
    }
    findings_.push_back(finding);
}

void ResearchProject::AddPublication(const Publication& publication) {
    if (completed_) {
        throw ResearchException("cannot add publications to a completed project");
    }
    publications_.push_back(publication);
}

void ResearchProject::Complete() {
    if (completed_) {
        throw ResearchException("project is already complete");
    }
    completed_ = true;
}

bool ResearchProject::LedBy(const Researcher& researcher) const {
    return leadResearcherId_ == researcher.Id();
}

RestorationTask::RestorationTask(std::string id, std::string exhibitId, std::string restorerId,
                                 double hourlyRate)
    : id_(std::move(id)),
      exhibitId_(std::move(exhibitId)),
      restorerId_(std::move(restorerId)),
      status_("pending"),
      hourlyRate_(hourlyRate) {
    if (id_.empty() || exhibitId_.empty() || restorerId_.empty()) {
        throw RestorationException("restoration task ids must not be empty");
    }
    if (hourlyRate_ < 0.0) {
        throw RestorationException("restoration hourly rate must not be negative");
    }
}

void RestorationTask::Start() {
    if (status_ != "pending") {
        throw RestorationException("only pending tasks can be started");
    }
    status_ = "in progress";
}

void RestorationTask::AddHours(double hours) {
    if (status_ != "in progress") {
        throw RestorationException("hours can only be added to a task in progress");
    }
    if (hours <= 0.0) {
        throw RestorationException("restoration hours must be positive");
    }
    hoursSpent_ += hours;
}

void RestorationTask::Complete() {
    if (status_ != "in progress") {
        throw RestorationException("only tasks in progress can be completed");
    }
    status_ = "completed";
}

double RestorationTask::Cost() const { return hoursSpent_ * hourlyRate_; }

RestorationMaterial::RestorationMaterial(std::string name, double quantity, double unitCost)
    : name_(std::move(name)), quantity_(quantity), unitCost_(unitCost) {
    if (name_.empty()) {
        throw RestorationException("material name must not be empty");
    }
    if (quantity_ < 0.0 || unitCost_ < 0.0) {
        throw RestorationException("material quantity and cost must not be negative");
    }
}

void RestorationMaterial::Consume(double amount) {
    if (amount <= 0.0 || amount > quantity_) {
        throw RestorationException("material consumption is invalid");
    }
    quantity_ -= amount;
}

void RestorationMaterial::Restock(double amount) {
    if (amount <= 0.0) {
        throw RestorationException("material restock must be positive");
    }
    quantity_ += amount;
}

double RestorationMaterial::TotalValue() const { return quantity_ * unitCost_; }

RestorationLab::RestorationLab(int capacity) : capacity_(capacity) {
    if (capacity_ <= 0) {
        throw RestorationException("restoration lab capacity must be positive");
    }
}

bool RestorationLab::AssignTask(const RestorationTask& task) {
    if (tasks_.size() >= static_cast<std::size_t>(capacity_)) {
        throw RestorationException("restoration lab is full");
    }
    const bool duplicate = std::any_of(tasks_.begin(), tasks_.end(),
                                       [&task](const RestorationTask& stored) {
                                           return stored.Id() == task.Id();
                                       });
    if (duplicate) {
        throw RestorationException("duplicate restoration task id: " + task.Id());
    }
    tasks_.push_back(task);
    return true;
}

void RestorationLab::CompleteTask(const std::string& taskId) {
    for (RestorationTask& task : tasks_) {
        if (task.Id() == taskId) {
            task.Complete();
            return;
        }
    }
    throw RestorationException("unknown restoration task: " + taskId);
}

bool RestorationLab::AddMaterial(const RestorationMaterial& material) {
    const bool duplicate = std::any_of(
        materials_.begin(), materials_.end(), [&material](const RestorationMaterial& stored) {
            return stored.Name() == material.Name();
        });
    if (duplicate) {
        return false;
    }
    materials_.push_back(material);
    return true;
}

void RestorationLab::ConsumeMaterial(const std::string& name, double amount) {
    for (RestorationMaterial& material : materials_) {
        if (material.Name() == name) {
            material.Consume(amount);
            return;
        }
    }
    throw RestorationException("unknown restoration material: " + name);
}

std::size_t RestorationLab::PendingCount() const {
    return static_cast<std::size_t>(std::count_if(
        tasks_.begin(), tasks_.end(), [](const RestorationTask& task) {
            return task.Status() != "completed";
        }));
}

double RestorationLab::TotalCost() const {
    double total = 0.0;
    for (const RestorationTask& task : tasks_) {
        total += task.Cost();
    }
    return total;
}

}  // namespace museum
