/**
 * @file test_cli.cpp
 * @brief Unit tests for the console interface.
 */

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "logistics/cli/console_app.hpp"

namespace logistics {
namespace {

std::string RunConsole(const std::string& input) {
    ConsoleApp application;
    std::istringstream inputStream(input);
    std::ostringstream outputStream;
    application.Run(inputStream, outputStream);
    return outputStream.str();
}

std::size_t CountOccurrences(const std::string& text, const std::string& needle) {
    std::size_t count = 0;
    std::size_t position = text.find(needle);
    while (position != std::string::npos) {
        ++count;
        position = text.find(needle, position + needle.size());
    }
    return count;
}

}  // namespace

TEST(ConsoleAppTest, PrintsSeededState) {
    const std::string output = RunConsole(
        "help\nsummary\nclients\nvehicles\nwarehouses\ndrivers\norders\nreport\n\nquit\n");
    EXPECT_NE(output.find("summary | clients"), std::string::npos);
    EXPECT_NE(output.find("company: TransLogistics"), std::string::npos);
    EXPECT_NE(output.find("Acme Retail"), std::string::npos);
    EXPECT_NE(output.find("T1 truck"), std::string::npos);
    EXPECT_NE(output.find("Central Hub"), std::string::npos);
    EXPECT_NE(output.find("Pavel Ivanov"), std::string::npos);
    EXPECT_NE(output.find("confirmed"), std::string::npos);
    EXPECT_NE(output.find("bye"), std::string::npos);
}

TEST(ConsoleAppTest, RunsEndToEndWorkflow) {
    const std::string input =
        "client C3 NewCo Contact 500 100 4.0\n"
        "warehouse W2 EastHub Grodno 1000 50\n"
        "truck T2 XX111 3000 20 2 5000\n"
        "van V2 YY222 1000 8 50\n"
        "refrigerated R2 ZZ333 5000 30 3 10000 -20 5\n"
        "driver D3 Petr LIC-D3 C 5 4.2\n"
        "order O2 C1 CGO2 Minsk Brest 700 1\n"
        "order O3 C1 CGO3 Minsk Brest 50 0\n"
        "order O4 C1 CGO4 Minsk Brest 40 2\n"
        "orders\n"
        "confirm O2\n"
        "cancel O3\n"
        "dispatch O2 T1 D1 C R1 1\n"
        "drivers\n"
        "orders\n"
        "deliver SHP-1 2\n"
        "invoice C1 O2 700 0 30\n"
        "pay INV-1 700 REF2\n"
        "track SHP-1 53.9 27.5 1 10 70\n"
        "track SHP-1 52.1 23.7 2 12 80\n"
        "tracking SHP-1\n"
        "quote general 350 1200 12\n"
        "retire V1\n"
        "vehicles\n"
        "orders\n"
        "report\n"
        "exit\n";
    const std::string output = RunConsole(input);
    EXPECT_NE(output.find("client added: C3"), std::string::npos);
    EXPECT_NE(output.find("warehouse added: W2"), std::string::npos);
    EXPECT_NE(output.find("truck added: T2"), std::string::npos);
    EXPECT_NE(output.find("van added: V2"), std::string::npos);
    EXPECT_NE(output.find("refrigerated truck added: R2"), std::string::npos);
    EXPECT_NE(output.find("driver added: D3"), std::string::npos);
    EXPECT_NE(output.find("order placed: O2"), std::string::npos);
    EXPECT_NE(output.find("draft"), std::string::npos);
    EXPECT_NE(output.find("order confirmed: O2"), std::string::npos);
    EXPECT_NE(output.find("order cancelled: O3"), std::string::npos);
    EXPECT_NE(output.find("shipment created: SHP-1"), std::string::npos);
    EXPECT_NE(output.find("on trip"), std::string::npos);
    EXPECT_NE(output.find("in transit"), std::string::npos);
    EXPECT_NE(output.find("shipment delivered: SHP-1"), std::string::npos);
    EXPECT_NE(output.find("invoice issued: INV-1"), std::string::npos);
    EXPECT_NE(output.find("payment registered: PAY-1"), std::string::npos);
    EXPECT_NE(output.find("tracking point recorded: SHP-1"), std::string::npos);
    EXPECT_NE(output.find("points: 2"), std::string::npos);
    EXPECT_NE(output.find("quote: 604.80"), std::string::npos);
    EXPECT_NE(output.find("vehicle retired: V1"), std::string::npos);
    EXPECT_NE(output.find("retired"), std::string::npos);
    EXPECT_NE(output.find("delivered"), std::string::npos);
    EXPECT_NE(output.find("cancelled"), std::string::npos);
    EXPECT_NE(output.find("paid: 1"), std::string::npos);
}

TEST(ConsoleAppTest, PrintsUsageForMissingArguments) {
    const std::string input =
        "client\nwarehouse\ntruck\nvan\nrefrigerated\ndriver\norder\nconfirm\ncancel\nretire\n"
        "quote\ndispatch\ndeliver\ninvoice\npay\ntrack\ntracking\nbogus\nexit\n";
    const std::string output = RunConsole(input);
    EXPECT_EQ(CountOccurrences(output, "usage:"), 17U);
    EXPECT_NE(output.find("unknown command: bogus"), std::string::npos);
}

TEST(ConsoleAppTest, ReportsMissingEntities) {
    const std::string input =
        "confirm missing\n"
        "cancel missing\n"
        "retire missing\n"
        "dispatch missing T1 D1 C R1 1\n"
        "dispatch O1 missing D1 C R1 1\n"
        "dispatch O1 T1 missing C R1 1\n"
        "dispatch O1 T1 D1 C missing 1\n"
        "deliver missing 2\n"
        "invoice missing O1 10 0 10\n"
        "pay missing 10 REF\n"
        "track SHP-1 100 0 1 1 10\n"
        "tracking SHP-1\n"
        "exit\n";
    const std::string output = RunConsole(input);
    EXPECT_NE(output.find("error: Order not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: Vehicle not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: Driver not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: Route not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: Shipment not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: Client not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: Invoice not found: missing"), std::string::npos);
    EXPECT_NE(output.find("error: GeoPoint coordinates are out of range"), std::string::npos);
    EXPECT_NE(output.find("error: No tracking data for shipment SHP-1"), std::string::npos);
}

TEST(ConsoleAppTest, StopsAtEndOfInput) {
    const std::string output = RunConsole("");
    EXPECT_TRUE(output.empty());
}

}  // namespace logistics
