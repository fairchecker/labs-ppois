#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "logistics/company.hpp"
#include "logistics/geo.hpp"
#include "logistics/people.hpp"

/**
 * @file console_app.hpp
 * @brief Console interface separated from the domain model.
 */

namespace logistics {

/**
 * @class ConsoleApp
 * @brief Interactive command loop over a seeded company.
 */
class ConsoleApp {
    public:
    /// @brief Creates the application and seeds demonstration data.
    ConsoleApp();

    /**
     * @brief Runs the command loop until end of input or the exit command.
     * @param input command stream.
     * @param output report stream.
     * @return process exit code.
     */
    int Run(std::istream& input, std::ostream& output);

    private:
    void Seed();
    void DispatchCommand(const std::string& command, std::istringstream& arguments,
                         std::ostream& output);
    void PrintHelp(std::ostream& output) const;
    void PrintSummary(std::ostream& output) const;
    void PrintClients(std::ostream& output) const;
    void PrintVehicles(std::ostream& output) const;
    void PrintWarehouses(std::ostream& output) const;
    void PrintDrivers(std::ostream& output) const;
    void PrintOrders(std::ostream& output) const;
    void AddClient(std::istringstream& arguments, std::ostream& output);
    void AddWarehouse(std::istringstream& arguments, std::ostream& output);
    void AddTruck(std::istringstream& arguments, std::ostream& output);
    void AddVan(std::istringstream& arguments, std::ostream& output);
    void AddRefrigeratedTruck(std::istringstream& arguments, std::ostream& output);
    void AddDriver(std::istringstream& arguments, std::ostream& output);
    void AddOrder(std::istringstream& arguments, std::ostream& output);
    void ConfirmOrder(std::istringstream& arguments, std::ostream& output);
    void CancelOrder(std::istringstream& arguments, std::ostream& output);
    void RetireVehicle(std::istringstream& arguments, std::ostream& output);
    void QuoteShipment(std::istringstream& arguments, std::ostream& output);
    void DispatchShipment(std::istringstream& arguments, std::ostream& output);
    void DeliverShipment(std::istringstream& arguments, std::ostream& output);
    void IssueInvoice(std::istringstream& arguments, std::ostream& output);
    void RegisterPayment(std::istringstream& arguments, std::ostream& output);
    void RecordTracking(std::istringstream& arguments, std::ostream& output);
    void ShowTracking(std::istringstream& arguments, std::ostream& output);
    void PrintReport(std::ostream& output) const;
    Driver* FindDriver(const std::string& driverId);
    Route* FindRoute(const std::string& routeId);

    TransportCompany company_;
    std::vector<Driver> drivers_;
    std::vector<Route> routes_;
};

}  // namespace logistics
