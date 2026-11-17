/**
 * @file test_cli.cpp
 * @brief End-to-end tests for the museum console interface.
 */

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "museum/cli/console_app.hpp"

namespace museum {
namespace {

/// Runs the console with the given scripted input and returns its output.
std::string RunConsole(const std::string& input) {
    ConsoleApp application;
    std::istringstream in(input);
    std::ostringstream out;
    const int code = application.Run(in, out);
    EXPECT_EQ(code, 0);
    return out.str();
}

/// Checks that a string contains a fragment.
void ExpectContains(const std::string& text, const std::string& fragment) {
    EXPECT_NE(text.find(fragment), std::string::npos) << "missing: " << fragment << "\n" << text;
}

}  // namespace

TEST(ConsoleTest, HelpSummaryExhibitsAndExit) {
    const std::string output = RunConsole("help\nsummary\nexhibits\nexit\n");
    ExpectContains(output, "Commands:");
    ExpectContains(output, "City Museum");
    ExpectContains(output, "collection value: 55000");
    ExpectContains(output, "[painting] Morning Light");
    ExpectContains(output, "[fossil] Dinosaur Bone");
    ExpectContains(output, "Goodbye.");
}

TEST(ConsoleTest, HandlesBlankUnknownAndQuit) {
    const std::string output = RunConsole("\nunknown\nquit\n");
    ExpectContains(output, "Unknown command");
    ExpectContains(output, "Goodbye.");
}

TEST(ConsoleTest, SellsDiscountedTickets) {
    const std::string output = RunConsole("ticket v1 200\nticket v2 100\n");
    ExpectContains(output, "costs 140");
    ExpectContains(output, "costs 100");
    ExpectContains(RunConsole("ticket unknown 100\n"), "unknown visitor");
    ExpectContains(RunConsole("ticket v1 abc\n"), "invalid number");
    ExpectContains(RunConsole("ticket v1\n"), "usage: ticket");
}

TEST(ConsoleTest, BooksTourUntilFull) {
    const std::string output = RunConsole("tour v1\ntour v2\ntour v1\n");
    ExpectContains(output, "places left 1");
    ExpectContains(output, "places left 0");
    ExpectContains(output, "tour is full");
    ExpectContains(RunConsole("tour unknown\n"), "unknown visitor");
    ExpectContains(RunConsole("tour\n"), "usage: tour");
}

TEST(ConsoleTest, RunsRestorationTask) {
    const std::string output = RunConsole("restore p1 3\n");
    ExpectContains(output, "restoration of Morning Light costs 75");
    ExpectContains(RunConsole("restore p1 3\nrestore p1 1\n"), "duplicate restoration task");
    ExpectContains(RunConsole("restore unknown 1\n"), "unknown exhibit");
    ExpectContains(RunConsole("restore p1\n"), "usage: restore");
    ExpectContains(RunConsole("restore p1 abc\n"), "invalid number");
}

TEST(ConsoleTest, AcceptsDonations) {
    ExpectContains(RunConsole("donate 5000\n"), "budget total 105000");
    ExpectContains(RunConsole("donate abc\n"), "invalid number");
    ExpectContains(RunConsole("donate\n"), "usage: donate");
}

TEST(ConsoleTest, ControlsAlarm) {
    const std::string output =
        RunConsole("alarm arm\nalarm trigger\nalarm reset wrong\nalarm reset 1234\n"
                   "alarm arm\nalarm disarm 1234\nalarm trigger\n");
    ExpectContains(output, "alarm armed");
    ExpectContains(output, "alarm triggered");
    ExpectContains(output, "wrong alarm code");
    ExpectContains(output, "alarm reset completed");
    ExpectContains(output, "alarm disarm completed");
    ExpectContains(output, "cannot trigger a disarmed alarm");
    ExpectContains(RunConsole("alarm\n"), "usage: alarm");
    ExpectContains(RunConsole("alarm fly\n"), "unknown alarm action");
    ExpectContains(RunConsole("alarm reset\n"), "usage: alarm reset");
}

TEST(ConsoleTest, RecordsFeedbackAndAverage) {
    const std::string output = RunConsole("feedback 2 Poor\nexhibits\n");
    ExpectContains(output, "average rating 3.5");
    ExpectContains(RunConsole("feedback 6 Bad\n"), "rating must be in [1, 5]");
    ExpectContains(RunConsole("feedback abc Bad\n"), "invalid number");
    ExpectContains(RunConsole("feedback 5\n"), "usage: feedback");
    ExpectContains(RunConsole("feedback 4 Very good\n"), "average rating 4.5");
}

TEST(ConsoleTest, RecordsClimateReadings) {
    const std::string output = RunConsole("climate 20\nclimate 21\nclimate 30\n");
    ExpectContains(output, "stable: yes");
    ExpectContains(output, "stable: no");
    ExpectContains(RunConsole("climate 60\n"), "temperature reading is implausible");
    ExpectContains(RunConsole("climate abc\n"), "invalid number");
    ExpectContains(RunConsole("climate\n"), "usage: climate");
}

TEST(ConsoleTest, AssessesExhibitCondition) {
    const std::string output = RunConsole("condition p1 60\n");
    ExpectContains(output, "Morning Light condition 60: monitor condition");
    ExpectContains(RunConsole("condition unknown 50\n"), "unknown exhibit");
    ExpectContains(RunConsole("condition p1 101\n"), "condition score must be in [0, 100]");
    ExpectContains(RunConsole("condition p1\n"), "usage: condition");
}

TEST(ConsoleTest, DepreciatesExhibitValue) {
    const std::string output = RunConsole("depreciate p1 10\n");
    ExpectContains(output, "Morning Light value is now 4500");
    ExpectContains(RunConsole("depreciate unknown 10\n"), "unknown exhibit");
    ExpectContains(RunConsole("depreciate p1 101\n"), "depreciation percent must be in [0, 100]");
    ExpectContains(RunConsole("depreciate p1\n"), "usage: depreciate");
}

TEST(ConsoleTest, StopsAtEndOfInput) {
    const std::string output = RunConsole("summary\n");
    ExpectContains(output, "City Museum");
    EXPECT_EQ(output.find("Goodbye."), std::string::npos);
}

}  // namespace museum
