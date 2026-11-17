#include "museum/cli/console_app.hpp"

#include <sstream>
#include <utility>

#include "museum/condition_report.hpp"
#include "museum/curator.hpp"
#include "museum/donation.hpp"
#include "museum/exceptions.hpp"
#include "museum/fossil.hpp"
#include "museum/guide.hpp"
#include "museum/jewelry.hpp"
#include "museum/manuscript.hpp"
#include "museum/painting.hpp"
#include "museum/restoration_material.hpp"
#include "museum/restoration_task.hpp"
#include "museum/restorer.hpp"
#include "museum/visitor_feedback.hpp"

namespace museum {

namespace {

constexpr double kDemoBudget = 100000.0;
constexpr int kDemoTicketCapacity = 100;
constexpr int kDemoTourCapacity = 2;
constexpr double kDemoTourPrice = 100.0;
constexpr int kDemoRestorationCapacity = 5;
constexpr int kDemoFeedbackCapacity = 100;
constexpr double kDemoMinStorageTemperature = 16.0;
constexpr double kDemoMaxStorageTemperature = 24.0;
constexpr int kDemoDepartmentCapacity = 10;
constexpr int kDemoExhibitionStartDay = 1;
constexpr int kDemoExhibitionEndDay = 30;
constexpr double kDemoExhibitionPrice = 50.0;
constexpr int kDemoDay = 1;
constexpr double kRestorationHourlyRate = 25.0;

/// Splits a line into whitespace-separated tokens.
std::vector<std::string> Tokenize(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

/// Parses a floating-point command argument.
double ParseDouble(const std::string& text) {
    std::size_t parsed = 0;
    double value = 0.0;
    try {
        value = std::stod(text, &parsed);
    } catch (const std::exception&) {
        throw MuseumException("invalid number: " + text);
    }
    if (parsed != text.size()) {
        throw MuseumException("invalid number: " + text);
    }
    return value;
}

/// Parses an integer command argument.
int ParseInt(const std::string& text) {
    std::size_t parsed = 0;
    int value = 0;
    try {
        value = std::stoi(text, &parsed);
    } catch (const std::exception&) {
        throw MuseumException("invalid number: " + text);
    }
    if (parsed != text.size()) {
        throw MuseumException("invalid number: " + text);
    }
    return value;
}

/// Joins tokens starting at the given index with single spaces.
std::string JoinFrom(const std::vector<std::string>& tokens, std::size_t first) {
    std::string result;
    for (std::size_t index = first; index < tokens.size(); ++index) {
        if (!result.empty()) {
            result += ' ';
        }
        result += tokens[index];
    }
    return result;
}

/// Looks up a visitor or reports an error.
Visitor& RequireVisitor(std::map<std::string, Visitor>& visitors, const std::string& id) {
    const auto match = visitors.find(id);
    if (match == visitors.end()) {
        throw MuseumException("unknown visitor: " + id);
    }
    return match->second;
}

}  // namespace

ConsoleApp::ConsoleApp()
    : museum_("City Museum", Address("Main St", "Minsk", "220000", "5"),
              Budget(kDemoBudget, "general")),
      ticketOffice_(kDemoTicketCapacity),
      tour_("tour1", "g1", "Highlights", kDemoTourCapacity, kDemoTourPrice),
      restorationLab_(kDemoRestorationCapacity),
      security_("1234"),
      alarm_("1234"),
      feedbackBoard_(kDemoFeedbackCapacity),
      climateLog_(kDemoMinStorageTemperature, kDemoMaxStorageTemperature) {
    SeedDemoData();
}

void ConsoleApp::SeedDemoData() {
    SeedOrganization();
    SeedEmployees();
    SeedCollection();
    SeedVisitorServices();
}

void ConsoleApp::SeedOrganization() {
    auto archaeology = std::make_shared<Department>("Archaeology", 1, "c1",
                                                    kDemoDepartmentCapacity);
    archaeology->AddExhibit("p1");
    archaeology->AddExhibit("f1");
    museum_.AddDepartment(archaeology);
    museum_.AddDepartment(
        std::make_shared<Department>("Art", 2, "c1", kDemoDepartmentCapacity));
    museum_.OpenExhibition(Exhibition("ex1", "Ancient Worlds", kDemoExhibitionStartDay,
                                     kDemoExhibitionEndDay, kDemoExhibitionPrice));
}

void ConsoleApp::SeedEmployees() {
    museum_.Hire(std::make_shared<Curator>("c1", "Ada Stone", 45, 1200.0, 2012, "Archaeology",
                                          "Prehistory"));
    museum_.Hire(std::make_shared<Guide>("g1", "Ivan Guide", 32, 800.0, 2019, "Education",
                                        std::vector<std::string>{"English", "Russian"}, 4.8));
    museum_.Hire(std::make_shared<Restorer>("r1", "Nina Restorer", 38, 900.0, 2016,
                                           "Restoration", "textiles"));
}

void ConsoleApp::SeedCollection() {
    museum_.AddExhibit(std::make_shared<Painting>("p1", "Morning Light", "Modern", 82, 5000.0,
                                                  3.0, "France", "Monet", "oil", 100.0, 50.0));
    museum_.AddExhibit(std::make_shared<Fossil>("f1", "Dinosaur Bone", "Jurassic", 68, 8000.0,
                                                5.0, "Mongolia", "Sauropod", "Jurassic", 150000.0));
    museum_.AddExhibit(std::make_shared<Manuscript>("m1", "Old Chronicle", "Medieval", 74, 12000.0,
                                                    2.0, "Italy", 320, "Latin", 1420));
    museum_.AddExhibit(std::make_shared<Jewelry>("j1", "Royal Brooch", "Modern", 90, 30000.0, 0.2,
                                                 "France", "diamond", 3.0, true));
}

void ConsoleApp::SeedVisitorServices() {
    visitors_.emplace("v1", Visitor("v1", "Peter Student", 20, "student", 200.0));
    visitors_.emplace("v2", Visitor("v2", "Anna Guest", 35, "full", 500.0));
    restorationLab_.AddMaterial(RestorationMaterial("Linen", 10.0, 3.0));
    feedbackBoard_.Add(VisitorFeedback("v1", kMaxRating, "Wonderful collection", kDemoDay));
}

int ConsoleApp::Run(std::istream& in, std::ostream& out) {
    out << "Museum management console. Type 'help' for commands.\n";
    std::string line;
    while (std::getline(in, line)) {
        if (ProcessLine(line, out)) {
            return 0;
        }
    }
    return 0;
}

bool ConsoleApp::ProcessLine(const std::string& line, std::ostream& out) {
    const std::vector<std::string> tokens = Tokenize(line);
    if (tokens.empty()) {
        return false;
    }
    if (tokens[0] == "exit" || tokens[0] == "quit") {
        out << "Goodbye.\n";
        return true;
    }
    try {
        if (!HandleCommand(tokens, out)) {
            out << "Unknown command. Type 'help'.\n";
        }
    } catch (const MuseumException& error) {
        out << "error: " << error.what() << '\n';
    }
    return false;
}

bool ConsoleApp::HandleCommand(const std::vector<std::string>& tokens, std::ostream& out) {
    return HandleReportCommand(tokens, out) || HandleVisitorCommand(tokens, out) ||
           HandleFacilityCommand(tokens, out);
}

bool ConsoleApp::HandleReportCommand(const std::vector<std::string>& tokens, std::ostream& out) {
    const std::string& command = tokens[0];
    if (command == "help") {
        PrintHelp(out);
    } else if (command == "summary") {
        PrintSummary(out);
    } else if (command == "exhibits") {
        PrintExhibits(out);
    } else if (command == "condition") {
        HandleCondition(tokens, out);
    } else if (command == "depreciate") {
        HandleDepreciate(tokens, out);
    } else {
        return false;
    }
    return true;
}

bool ConsoleApp::HandleVisitorCommand(const std::vector<std::string>& tokens, std::ostream& out) {
    const std::string& command = tokens[0];
    if (command == "ticket") {
        HandleTicket(tokens, out);
    } else if (command == "tour") {
        HandleTour(tokens, out);
    } else if (command == "feedback") {
        HandleFeedback(tokens, out);
    } else {
        return false;
    }
    return true;
}

bool ConsoleApp::HandleFacilityCommand(const std::vector<std::string>& tokens, std::ostream& out) {
    const std::string& command = tokens[0];
    if (command == "restore") {
        HandleRestore(tokens, out);
    } else if (command == "donate") {
        HandleDonate(tokens, out);
    } else if (command == "alarm") {
        HandleAlarm(tokens, out);
    } else if (command == "climate") {
        HandleClimate(tokens, out);
    } else {
        return false;
    }
    return true;
}

void ConsoleApp::PrintHelp(std::ostream& out) const {
    out << "Commands:\n"
        << "  help                              show this help\n"
        << "  summary                           museum overview\n"
        << "  exhibits                          list collection\n"
        << "  ticket <visitorId> <price>        sell a discounted ticket\n"
        << "  tour <visitorId>                  book the featured tour\n"
        << "  restore <exhibitId> <hours>       run a restoration task\n"
        << "  donate <amount>                   add funds to the budget\n"
        << "  alarm <arm|trigger|reset|disarm> [code]\n"
        << "  feedback <rating> <comment...>    leave visitor feedback\n"
        << "  climate <temperature>             record storage climate\n"
        << "  condition <exhibitId> <score>     assess exhibit condition\n"
        << "  depreciate <exhibitId> <percent>  reduce appraised value\n"
        << "  exit, quit                        leave the console\n";
}

void ConsoleApp::PrintSummary(std::ostream& out) const {
    out << museum_.Name() << " at " << museum_.Location().Format() << '\n'
        << "departments: " << museum_.DepartmentCount()
        << ", employees: " << museum_.EmployeeCount()
        << ", exhibits: " << museum_.ExhibitCount()
        << ", exhibitions: " << museum_.ExhibitionCount() << '\n'
        << "collection value: " << museum_.TotalCollectionValue()
        << ", annual payroll: " << museum_.AnnualPayroll()
        << ", budget remaining: " << museum_.BudgetRef().Remaining() << '\n'
        << "tour bookings: " << tour_.BookedVisitors() << ", tour revenue: " << tour_.Revenue()
        << '\n';
}

void ConsoleApp::PrintExhibits(std::ostream& out) const {
    for (const std::shared_ptr<Exhibit>& exhibit : museum_.Exhibits()) {
        out << exhibit->Id() << " [" << exhibit->Category() << "] " << exhibit->Title()
            << " condition " << exhibit->ConditionScore() << " value "
            << exhibit->AppraisedValue() << '\n';
    }
}

void ConsoleApp::HandleTicket(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 3) {
        throw MuseumException("usage: ticket <visitorId> <price>");
    }
    Visitor& visitor = RequireVisitor(visitors_, tokens[1]);
    const double price = ParseDouble(tokens[2]);
    const Ticket ticket = ticketOffice_.SellTicket(visitor, visitor.TicketType(), price, kDemoDay);
    out << "ticket " << ticket.Id() << " for " << visitor.FullName() << " costs "
        << ticket.Price() << '\n';
}

void ConsoleApp::HandleTour(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw MuseumException("usage: tour <visitorId>");
    }
    Visitor& visitor = RequireVisitor(visitors_, tokens[1]);
    tour_.Book(visitor);
    out << "tour booked for " << visitor.FullName() << ", places left "
        << (tour_.MaxVisitors() - tour_.BookedVisitors()) << '\n';
}

void ConsoleApp::HandleRestore(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 3) {
        throw MuseumException("usage: restore <exhibitId> <hours>");
    }
    const std::shared_ptr<Exhibit> exhibit = museum_.FindExhibit(tokens[1]);
    if (exhibit == nullptr) {
        throw ExhibitNotFoundException("unknown exhibit: " + tokens[1]);
    }
    const double hours = ParseDouble(tokens[2]);
    RestorationTask task("task-" + tokens[1], tokens[1], "r1", kRestorationHourlyRate);
    task.Start();
    task.AddHours(hours);
    task.Complete();
    restorationLab_.AssignTask(task);
    out << "restoration of " << exhibit->Title() << " costs " << task.Cost() << '\n';
}

void ConsoleApp::HandleDonate(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw MuseumException("usage: donate <amount>");
    }
    const double amount = ParseDouble(tokens[1]);
    const Donation donation("Guest", amount, "general", true);
    donation.ApplyToBudget(museum_.BudgetRef());
    out << "donation accepted, budget total " << museum_.BudgetRef().Total() << '\n';
}

void ConsoleApp::HandleAlarm(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw MuseumException("usage: alarm <arm|trigger|reset|disarm> [code]");
    }
    const std::string& action = tokens[1];
    if (action == "arm") {
        alarm_.Arm();
        out << "alarm armed\n";
    } else if (action == "trigger") {
        alarm_.Trigger();
        out << "alarm triggered\n";
    } else if (action == "reset" || action == "disarm") {
        if (tokens.size() < 3) {
            throw MuseumException("usage: alarm " + action + " <code>");
        }
        const bool changed = action == "reset" ? alarm_.Reset(tokens[2]) : alarm_.Disarm(tokens[2]);
        out << (changed ? "alarm " + action + " completed\n" : "alarm was not active\n");
    } else {
        throw MuseumException("unknown alarm action: " + action);
    }
}

void ConsoleApp::HandleFeedback(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 3) {
        throw MuseumException("usage: feedback <rating> <comment...>");
    }
    const int rating = ParseInt(tokens[1]);
    feedbackBoard_.Add(VisitorFeedback("v1", rating, JoinFrom(tokens, 2), kDemoDay));
    out << "feedback recorded, average rating " << feedbackBoard_.AverageRating() << '\n';
}

void ConsoleApp::HandleClimate(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw MuseumException("usage: climate <temperature>");
    }
    climateLog_.Record(ParseDouble(tokens[1]));
    const bool stable = climateLog_.IsStable();
    out << "temperature recorded, stable: " << (stable ? "yes" : "no") << ", average "
        << climateLog_.Average() << '\n';
}

void ConsoleApp::HandleCondition(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 3) {
        throw MuseumException("usage: condition <exhibitId> <score>");
    }
    const std::shared_ptr<Exhibit> exhibit = museum_.FindExhibit(tokens[1]);
    if (exhibit == nullptr) {
        throw ExhibitNotFoundException("unknown exhibit: " + tokens[1]);
    }
    const int score = ParseInt(tokens[2]);
    exhibit->AssessCondition(score);
    const ConditionReport report(tokens[1], score, "console assessment", kDemoDay);
    out << exhibit->Title() << " condition " << score << ": " << report.Recommendation() << '\n';
}

void ConsoleApp::HandleDepreciate(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 3) {
        throw MuseumException("usage: depreciate <exhibitId> <percent>");
    }
    const std::shared_ptr<Exhibit> exhibit = museum_.FindExhibit(tokens[1]);
    if (exhibit == nullptr) {
        throw ExhibitNotFoundException("unknown exhibit: " + tokens[1]);
    }
    exhibit->Depreciate(ParseDouble(tokens[2]));
    out << exhibit->Title() << " value is now " << exhibit->AppraisedValue() << '\n';
}

}  // namespace museum
