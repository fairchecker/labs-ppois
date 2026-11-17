#pragma once

#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "museum/alarm_system.hpp"
#include "museum/climate_log.hpp"
#include "museum/feedback_board.hpp"
#include "museum/museum.hpp"
#include "museum/restoration_lab.hpp"
#include "museum/security_system.hpp"
#include "museum/ticket_office.hpp"
#include "museum/tour.hpp"
#include "museum/visitor.hpp"

/**
 * @file console_app.hpp
 * @brief Console interface separated from the museum domain model.
 */

namespace museum {

/**
 * @class ConsoleApp
 * @brief Interactive command dispatcher over a seeded museum.
 */
class ConsoleApp {
    public:
    /**
     * @brief Creates the application with demo data.
     */
    ConsoleApp();

    /**
     * @brief Runs the command loop until exit, quit or end of input.
     * @param in command stream.
     * @param out response stream.
     * @return process exit code.
     */
    int Run(std::istream& in, std::ostream& out);

    private:
    /// Builds the demo museum used by the console.
    void SeedDemoData();

    /// Creates the demo departments and exhibition.
    void SeedOrganization();

    /// Hires the demo employees.
    void SeedEmployees();

    /// Adds the demo exhibits and collection.
    void SeedCollection();

    /// Creates demo visitors and facility state.
    void SeedVisitorServices();

    /// Prints the command reference.
    void PrintHelp(std::ostream& out) const;

    /// Prints a museum overview.
    void PrintSummary(std::ostream& out) const;

    /// Prints every exhibit in the collection.
    void PrintExhibits(std::ostream& out) const;

    /**
     * @brief Processes one input line.
     * @return true when the line asks to leave the console.
     */
    bool ProcessLine(const std::string& line, std::ostream& out);

    /**
     * @brief Dispatches one command.
     * @return true when the command is known.
     */
    bool HandleCommand(const std::vector<std::string>& tokens, std::ostream& out);

    /// Handles reporting and collection commands.
    bool HandleReportCommand(const std::vector<std::string>& tokens, std::ostream& out);

    /// Handles ticketing, tour and feedback commands.
    bool HandleVisitorCommand(const std::vector<std::string>& tokens, std::ostream& out);

    /// Handles restoration, donation, alarm and climate commands.
    bool HandleFacilityCommand(const std::vector<std::string>& tokens, std::ostream& out);

    /// Handles `ticket <visitorId> <price>`.
    void HandleTicket(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `tour <visitorId>`.
    void HandleTour(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `restore <exhibitId> <hours>`.
    void HandleRestore(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `donate <amount>`.
    void HandleDonate(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `alarm <arm|trigger|reset|disarm> [code]`.
    void HandleAlarm(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `feedback <rating> <comment...>`.
    void HandleFeedback(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `climate <temperature>`.
    void HandleClimate(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `condition <exhibitId> <score>`.
    void HandleCondition(const std::vector<std::string>& tokens, std::ostream& out);
    /// Handles `depreciate <exhibitId> <percent>`.
    void HandleDepreciate(const std::vector<std::string>& tokens, std::ostream& out);

    /// Museum aggregate root.
    Museum museum_;
    /// Ticket sales desk.
    TicketOffice ticketOffice_;
    /// Featured guided tour.
    Tour tour_;
    /// Restoration lab.
    RestorationLab restorationLab_;
    /// Security control system.
    SecuritySystem security_;
    /// Emergency alarm.
    AlarmSystem alarm_;
    /// Visitor feedback board.
    FeedbackBoard feedbackBoard_;
    /// Storage climate history.
    ClimateLog climateLog_;
    /// Known visitors by identifier.
    std::map<std::string, Visitor> visitors_;
};

}  // namespace museum
