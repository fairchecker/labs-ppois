#include "logistics/cli/console_app.hpp"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "logistics/exceptions.hpp"

namespace logistics {
namespace {

constexpr double kSeedLatitude = 53.9023;
constexpr double kSeedLongitude = 27.5619;
constexpr double kBrestLatitude = 52.0976;
constexpr double kBrestLongitude = 23.7341;
constexpr double kWarsawLatitude = 52.2297;
constexpr double kWarsawLongitude = 21.0122;
constexpr double kSeedOdometerKm = 10000.0;
constexpr double kSeedFuelLiters = 150.0;
constexpr int kSeedHiredYear = 2020;
constexpr double kSeedSalary = 2500.0;
constexpr int kLicenseIssuedDay = 0;
constexpr int kLicenseValidityDays = 3650;
constexpr int kReportPrecision = 2;
constexpr int kExpressPriorityCode = 1;
constexpr int kCriticalPriorityCode = 2;
const char* const kDefaultPhone = "+375 29 000-00-00";
const char* const kEmailDomain = "@translogistics.example";
const char* const kDefaultStreet = "Depot 1";
const char* const kDefaultPostalCode = "000000";

Address MakeAddress(const std::string& city, double latitude, double longitude) {
    return Address(kDefaultStreet, city, kDefaultPostalCode, GeoPoint(latitude, longitude));
}

std::string VehicleStatusName(VehicleStatus status) {
    switch (status) {
        case VehicleStatus::Available:
            return "available";
        case VehicleStatus::OnRoute:
            return "on route";
        case VehicleStatus::Maintenance:
            return "maintenance";
        case VehicleStatus::Retired:
            return "retired";
    }
    return "unknown";
}

std::string OrderStatusName(OrderStatus status) {
    switch (status) {
        case OrderStatus::Draft:
            return "draft";
        case OrderStatus::Confirmed:
            return "confirmed";
        case OrderStatus::InTransit:
            return "in transit";
        case OrderStatus::Delivered:
            return "delivered";
        case OrderStatus::Cancelled:
            return "cancelled";
    }
    return "unknown";
}

std::string DriverStatusName(DriverStatus status) {
    switch (status) {
        case DriverStatus::Available:
            return "available";
        case DriverStatus::OnTrip:
            return "on trip";
        case DriverStatus::OffDuty:
            return "off duty";
    }
    return "unknown";
}

OrderPriority ParsePriority(int code) {
    if (code == kExpressPriorityCode) {
        return OrderPriority::Express;
    }
    if (code == kCriticalPriorityCode) {
        return OrderPriority::Critical;
    }
    return OrderPriority::Standard;
}

Driver MakeSeedDriver(const std::string& id, const std::string& name, const std::string& license,
                      const std::string& category, int experience, double rating) {
    return Driver(id, name, kDefaultPhone, id + kEmailDomain, "EMP-" + id, kSeedHiredYear,
                  kSeedSalary, DrivingLicense(license, category, kLicenseIssuedDay,
                                              kLicenseValidityDays),
                  experience, rating);
}

}  // namespace

ConsoleApp::ConsoleApp() : company_("TransLogistics", 10000.0) { Seed(); }

void ConsoleApp::Seed() {
    SeedNetwork();
    SeedCommercial();
}

void ConsoleApp::SeedNetwork() {
    company_.OnboardClient(Client("C1", "Acme Retail", "Olga Petrova", 2000.0, 5000.0, 4.5));
    company_.OnboardClient(Client("C2", "Fresh Market", "Ivan Sidorov", 1000.0, 1200.0, 3.8));
    const Address depot = MakeAddress("Minsk", kSeedLatitude, kSeedLongitude);
    company_.AddWarehouse(Warehouse("W1", "Central Hub", depot, 50000.0, 1200.0));
    company_.RegisterVehicle(std::make_unique<Truck>("T1", "AB1234-7", 20000.0, 80.0,
                                                     kSeedOdometerKm, kSeedFuelLiters, 3, 25000.0));
    company_.RegisterVehicle(std::make_unique<Van>("V1", "AC5678-2", 1500.0, 12.0, kSeedOdometerKm,
                                                   kSeedFuelLiters, true, 120));
    drivers_.push_back(MakeSeedDriver("D1", "Pavel Ivanov", "LIC-D1", "C", 8, 4.7));
    drivers_.push_back(MakeSeedDriver("D2", "Anna Smirnova", "LIC-D2", "B", 4, 4.9));
    company_.AddTariff(Tariff("TR1", "general", 1.2, 50.0, 0.05, 8.0));
    company_.AddTariff(Tariff("TR2", "perishable", 1.8, 80.0, 0.04, 12.0));
    company_.AddTariff(Tariff("TR3", "fragile", 1.5, 60.0, 0.06, 10.0));
}

void ConsoleApp::SeedCommercial() {
    const Address depot = MakeAddress("Minsk", kSeedLatitude, kSeedLongitude);
    const Address brest = MakeAddress("Brest", kBrestLatitude, kBrestLongitude);
    const Address warsaw = MakeAddress("Warsaw", kWarsawLatitude, kWarsawLongitude);
    Route route("R1", 1.2);
    route.AddSegment(RouteSegment(depot, brest, 350.0, "highway", 20.0));
    route.AddSegment(RouteSegment(brest, warsaw, 210.0, "highway", 15.0));
    routes_.push_back(route);
    company_.PlaceOrder(std::make_unique<Order>("O1", "C1", "CGO1", depot, brest, 0, 900.0,
                                                OrderPriority::Standard));
    company_.Orders().Require("O1").Confirm();
}

int ConsoleApp::Run(std::istream& input, std::ostream& output) {
    std::string line;
    while (std::getline(input, line)) {
        std::istringstream arguments(line);
        std::string command;
        if (!(arguments >> command)) {
            continue;
        }
        if (command == "exit" || command == "quit") {
            output << "bye\n";
            break;
        }
        try {
            DispatchCommand(command, arguments, output);
        } catch (const TransportException& error) {
            output << "error: " << error.what() << '\n';
        }
    }
    return 0;
}

void ConsoleApp::DispatchCommand(const std::string& command, std::istringstream& arguments,
                                 std::ostream& output) {
    if (DispatchOverviewCommand(command, output)) {
        return;
    }
    if (DispatchRegistrationCommand(command, arguments, output)) {
        return;
    }
    if (DispatchOperationsCommand(command, arguments, output)) {
        return;
    }
    output << "unknown command: " << command << '\n';
}

bool ConsoleApp::DispatchOverviewCommand(const std::string& command, std::ostream& output) const {
    if (command == "help") {
        PrintHelp(output);
    } else if (command == "summary") {
        PrintSummary(output);
    } else if (command == "clients") {
        PrintClients(output);
    } else if (command == "vehicles") {
        PrintVehicles(output);
    } else if (command == "warehouses") {
        PrintWarehouses(output);
    } else if (command == "drivers") {
        PrintDrivers(output);
    } else if (command == "orders") {
        PrintOrders(output);
    } else if (command == "report") {
        PrintReport(output);
    } else {
        return false;
    }
    return true;
}

bool ConsoleApp::DispatchRegistrationCommand(const std::string& command,
                                             std::istringstream& arguments,
                                             std::ostream& output) {
    if (command == "client") {
        AddClient(arguments, output);
    } else if (command == "warehouse") {
        AddWarehouse(arguments, output);
    } else if (command == "truck") {
        AddTruck(arguments, output);
    } else if (command == "van") {
        AddVan(arguments, output);
    } else if (command == "refrigerated") {
        AddRefrigeratedTruck(arguments, output);
    } else if (command == "driver") {
        AddDriver(arguments, output);
    } else if (command == "order") {
        AddOrder(arguments, output);
    } else {
        return false;
    }
    return true;
}

bool ConsoleApp::DispatchOperationsCommand(const std::string& command,
                                           std::istringstream& arguments,
                                           std::ostream& output) {
    if (DispatchOrderCommand(command, arguments, output)) {
        return true;
    }
    return DispatchDeliveryCommand(command, arguments, output);
}

bool ConsoleApp::DispatchOrderCommand(const std::string& command, std::istringstream& arguments,
                                      std::ostream& output) {
    if (command == "confirm") {
        ConfirmOrder(arguments, output);
    } else if (command == "cancel") {
        CancelOrder(arguments, output);
    } else if (command == "retire") {
        RetireVehicle(arguments, output);
    } else if (command == "quote") {
        QuoteShipment(arguments, output);
    } else {
        return false;
    }
    return true;
}

bool ConsoleApp::DispatchDeliveryCommand(const std::string& command,
                                         std::istringstream& arguments,
                                         std::ostream& output) {
    if (command == "dispatch") {
        DispatchShipment(arguments, output);
    } else if (command == "deliver") {
        DeliverShipment(arguments, output);
    } else if (command == "invoice") {
        IssueInvoice(arguments, output);
    } else if (command == "pay") {
        RegisterPayment(arguments, output);
    } else if (command == "track") {
        RecordTracking(arguments, output);
    } else if (command == "tracking") {
        ShowTracking(arguments, output);
    } else {
        return false;
    }
    return true;
}

void ConsoleApp::PrintHelp(std::ostream& output) const {
    output << "summary | clients | vehicles | warehouses | drivers | orders\n"
           << "client <id> <name> <contact> <credit> <balance> <rating>\n"
           << "warehouse <id> <name> <city> <capacityKg> <capacityM3>\n"
           << "truck <id> <plate> <capacityKg> <volumeM3> <axles> <trailerKg>\n"
           << "van <id> <plate> <capacityKg> <volumeM3> <parcels>\n"
           << "refrigerated <id> <plate> <capacityKg> <volumeM3> <axles> <trailerKg> <minC> <maxC>\n"
           << "driver <id> <name> <license> <category> <experience> <rating>\n"
           << "order <id> <client> <cargo> <fromCity> <toCity> <price> <priority 0-2>\n"
           << "confirm <orderId> | cancel <orderId> | quote <type> <km> <kg> <m3>\n"
           << "retire <vehicleId>\n"
           << "dispatch <order> <vehicle> <driver> <category> <route> <day>\n"
           << "deliver <shipmentId> <day> | invoice <client> <order> <amount> <issued> <due>\n"
           << "pay <invoiceId> <amount> <reference>\n"
           << "track <shipment> <lat> <lon> <day> <hour> <speed> | tracking <shipment>\n"
           << "report | exit\n";
}

void ConsoleApp::PrintSummary(std::ostream& output) const {
    output << company_.GenerateReport() << '\n';
    output << "tariffs: " << company_.Pricing().TariffCount()
           << ", tracked shipments: " << company_.Tracking().TrackedShipments() << '\n';
}

void ConsoleApp::PrintClients(std::ostream& output) const {
    for (const Client& client : company_.Clients()) {
        output << client.Describe() << ", rating " << client.Rating() << '\n';
    }
}

void ConsoleApp::PrintVehicles(std::ostream& output) const {
    for (const std::unique_ptr<Vehicle>& vehicle : company_.Fleet().Vehicles()) {
        output << vehicle->Id() << " " << vehicle->TypeName() << " " << vehicle->CapacityKg()
               << " kg, " << VehicleStatusName(vehicle->Status()) << '\n';
    }
}

void ConsoleApp::PrintWarehouses(std::ostream& output) const {
    for (const Warehouse& warehouse : company_.Warehouses()) {
        output << warehouse.Describe() << ", utilization " << std::fixed << std::setprecision(1)
               << warehouse.UtilizationPercent() << "%\n";
    }
}

void ConsoleApp::PrintDrivers(std::ostream& output) const {
    for (const Driver& driver : drivers_) {
        output << driver.Id() << " " << driver.FullName() << ", category "
               << driver.License().Category() << ", " << DriverStatusName(driver.Status()) << '\n';
    }
}

void ConsoleApp::PrintOrders(std::ostream& output) const {
    for (const std::unique_ptr<Order>& order : company_.Orders().Orders()) {
        output << order->Describe() << ", " << OrderStatusName(order->Status()) << '\n';
    }
}

void ConsoleApp::AddClient(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string name;
    std::string contact;
    double creditLimit = 0.0;
    double balance = 0.0;
    double rating = 0.0;
    if (!(arguments >> id >> name >> contact >> creditLimit >> balance >> rating)) {
        output << "usage: client <id> <name> <contact> <credit> <balance> <rating>\n";
        return;
    }
    company_.OnboardClient(Client(id, name, contact, creditLimit, balance, rating));
    output << "client added: " << id << '\n';
}

void ConsoleApp::AddWarehouse(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string name;
    std::string city;
    double capacityKg = 0.0;
    double capacityM3 = 0.0;
    if (!(arguments >> id >> name >> city >> capacityKg >> capacityM3)) {
        output << "usage: warehouse <id> <name> <city> <capacityKg> <capacityM3>\n";
        return;
    }
    const Address address = MakeAddress(city, kSeedLatitude, kSeedLongitude);
    company_.AddWarehouse(Warehouse(id, name, address, capacityKg, capacityM3));
    output << "warehouse added: " << id << '\n';
}

void ConsoleApp::AddTruck(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string plate;
    double capacityKg = 0.0;
    double volumeM3 = 0.0;
    int axles = 0;
    double trailerKg = 0.0;
    if (!(arguments >> id >> plate >> capacityKg >> volumeM3 >> axles >> trailerKg)) {
        output << "usage: truck <id> <plate> <capacityKg> <volumeM3> <axles> <trailerKg>\n";
        return;
    }
    company_.RegisterVehicle(std::make_unique<Truck>(id, plate, capacityKg, volumeM3,
                                                      kSeedOdometerKm, kSeedFuelLiters, axles,
                                                      trailerKg));
    output << "truck added: " << id << '\n';
}

void ConsoleApp::AddVan(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string plate;
    double capacityKg = 0.0;
    double volumeM3 = 0.0;
    int parcels = 0;
    if (!(arguments >> id >> plate >> capacityKg >> volumeM3 >> parcels)) {
        output << "usage: van <id> <plate> <capacityKg> <volumeM3> <parcels>\n";
        return;
    }
    company_.RegisterVehicle(std::make_unique<Van>(id, plate, capacityKg, volumeM3, kSeedOdometerKm,
                                                    kSeedFuelLiters, false, parcels));
    output << "van added: " << id << '\n';
}

void ConsoleApp::AddRefrigeratedTruck(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string plate;
    double capacityKg = 0.0;
    double volumeM3 = 0.0;
    int axles = 0;
    double trailerKg = 0.0;
    double minTemperatureC = 0.0;
    double maxTemperatureC = 0.0;
    if (!(arguments >> id >> plate >> capacityKg >> volumeM3 >> axles >> trailerKg >>
          minTemperatureC >> maxTemperatureC)) {
        output << "usage: refrigerated <id> <plate> <kg> <m3> <axles> <trailerKg> <minC> <maxC>\n";
        return;
    }
    company_.RegisterVehicle(std::make_unique<RefrigeratedTruck>(
        id, plate, capacityKg, volumeM3, kSeedOdometerKm, kSeedFuelLiters, axles, trailerKg,
        minTemperatureC, maxTemperatureC));
    output << "refrigerated truck added: " << id << '\n';
}

void ConsoleApp::AddDriver(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string name;
    std::string licenseNumber;
    std::string category;
    int experience = 0;
    double rating = 0.0;
    if (!(arguments >> id >> name >> licenseNumber >> category >> experience >> rating)) {
        output << "usage: driver <id> <name> <license> <category> <experience> <rating>\n";
        return;
    }
    const DrivingLicense license(licenseNumber, category, kLicenseIssuedDay, kLicenseValidityDays);
    drivers_.emplace_back(id, name, kDefaultPhone, id + kEmailDomain, "EMP-" + id, kSeedHiredYear,
                          kSeedSalary, license, experience, rating);
    output << "driver added: " << id << '\n';
}

void ConsoleApp::AddOrder(std::istringstream& arguments, std::ostream& output) {
    std::string id;
    std::string clientId;
    std::string cargoId;
    std::string fromCity;
    std::string toCity;
    double price = 0.0;
    int priorityCode = 0;
    if (!(arguments >> id >> clientId >> cargoId >> fromCity >> toCity >> price >> priorityCode)) {
        output << "usage: order <id> <client> <cargo> <from> <to> <price> <priority>\n";
        return;
    }
    const Address origin = MakeAddress(fromCity, kSeedLatitude, kSeedLongitude);
    const Address destination = MakeAddress(toCity, kBrestLatitude, kBrestLongitude);
    company_.PlaceOrder(std::make_unique<Order>(id, clientId, cargoId, origin, destination, 0, price,
                                                 ParsePriority(priorityCode)));
    output << "order placed: " << id << '\n';
}

void ConsoleApp::ConfirmOrder(std::istringstream& arguments, std::ostream& output) {
    std::string orderId;
    if (!(arguments >> orderId)) {
        output << "usage: confirm <orderId>\n";
        return;
    }
    company_.Orders().Require(orderId).Confirm();
    output << "order confirmed: " << orderId << '\n';
}

void ConsoleApp::CancelOrder(std::istringstream& arguments, std::ostream& output) {
    std::string orderId;
    if (!(arguments >> orderId)) {
        output << "usage: cancel <orderId>\n";
        return;
    }
    company_.Orders().Require(orderId).Cancel();
    output << "order cancelled: " << orderId << '\n';
}

void ConsoleApp::RetireVehicle(std::istringstream& arguments, std::ostream& output) {
    std::string vehicleId;
    if (!(arguments >> vehicleId)) {
        output << "usage: retire <vehicleId>\n";
        return;
    }
    Vehicle* vehicle = company_.Fleet().FindVehicle(vehicleId);
    if (vehicle == nullptr) {
        throw InvalidVehicleException("Vehicle not found: " + vehicleId);
    }
    vehicle->Retire();
    output << "vehicle retired: " << vehicleId << '\n';
}

void ConsoleApp::QuoteShipment(std::istringstream& arguments, std::ostream& output) {
    std::string cargoType;
    double distanceKm = 0.0;
    double weightKg = 0.0;
    double volumeM3 = 0.0;
    if (!(arguments >> cargoType >> distanceKm >> weightKg >> volumeM3)) {
        output << "usage: quote <type> <km> <kg> <m3>\n";
        return;
    }
    const double price = company_.Quote(cargoType, distanceKm, weightKg, volumeM3);
    output << "quote: " << std::fixed << std::setprecision(kReportPrecision) << price << '\n';
}

void ConsoleApp::DispatchShipment(std::istringstream& arguments, std::ostream& output) {
    std::string orderId;
    std::string vehicleId;
    std::string driverId;
    std::string category;
    std::string routeId;
    int day = 0;
    if (!(arguments >> orderId >> vehicleId >> driverId >> category >> routeId >> day)) {
        output << "usage: dispatch <order> <vehicle> <driver> <category> <route> <day>\n";
        return;
    }
    Order& order = RequireOrder(orderId);
    Vehicle& vehicle = RequireVehicle(vehicleId);
    Driver& driver = RequireDriver(driverId);
    RequireRoute(routeId);
    const Shipment& shipment = company_.Dispatch(order, vehicle, driver, category, routeId, day);
    output << "shipment created: " << shipment.Id() << '\n';
}

void ConsoleApp::DeliverShipment(std::istringstream& arguments, std::ostream& output) {
    std::string shipmentId;
    int day = 0;
    if (!(arguments >> shipmentId >> day)) {
        output << "usage: deliver <shipmentId> <day>\n";
        return;
    }
    company_.CompleteDelivery(shipmentId, day);
    output << "shipment delivered: " << shipmentId << '\n';
}

void ConsoleApp::IssueInvoice(std::istringstream& arguments, std::ostream& output) {
    std::string clientId;
    std::string orderId;
    double amount = 0.0;
    int issuedDay = 0;
    int dueDay = 0;
    if (!(arguments >> clientId >> orderId >> amount >> issuedDay >> dueDay)) {
        output << "usage: invoice <client> <order> <amount> <issued> <due>\n";
        return;
    }
    const Invoice& invoice = company_.BillClient(clientId, orderId, amount, issuedDay, dueDay);
    output << "invoice issued: " << invoice.Id() << '\n';
}

void ConsoleApp::RegisterPayment(std::istringstream& arguments, std::ostream& output) {
    std::string invoiceId;
    double amount = 0.0;
    std::string reference;
    if (!(arguments >> invoiceId >> amount >> reference)) {
        output << "usage: pay <invoiceId> <amount> <reference>\n";
        return;
    }
    const Payment payment =
        company_.ReceivePayment(invoiceId, amount, PaymentMethod::BankTransfer, 0, reference);
    output << "payment registered: " << payment.Id() << '\n';
}

void ConsoleApp::RecordTracking(std::istringstream& arguments, std::ostream& output) {
    std::string shipmentId;
    double latitude = 0.0;
    double longitude = 0.0;
    int day = 0;
    int hour = 0;
    double speedKmh = 0.0;
    if (!(arguments >> shipmentId >> latitude >> longitude >> day >> hour >> speedKmh)) {
        output << "usage: track <shipment> <lat> <lon> <day> <hour> <speed>\n";
        return;
    }
    company_.RecordTrackingPoint(
        TrackPoint(shipmentId, GeoPoint(latitude, longitude), day, hour, speedKmh));
    output << "tracking point recorded: " << shipmentId << '\n';
}

void ConsoleApp::ShowTracking(std::istringstream& arguments, std::ostream& output) {
    std::string shipmentId;
    if (!(arguments >> shipmentId)) {
        output << "usage: tracking <shipment>\n";
        return;
    }
    const std::vector<TrackPoint>& history = company_.Tracking().History(shipmentId);
    output << "points: " << history.size() << ", latest: " << history.back().Describe() << '\n';
    output << "average speed: " << company_.Tracking().AverageSpeedKmh(shipmentId)
           << " km/h, distance: " << company_.Tracking().DistanceTravelled(shipmentId) << " km\n";
}

void ConsoleApp::PrintReport(std::ostream& output) const { output << company_.GenerateReport() << '\n'; }

Driver* ConsoleApp::FindDriver(const std::string& driverId) {
    for (Driver& driver : drivers_) {
        if (driver.Id() == driverId) {
            return &driver;
        }
    }
    return nullptr;
}

Route* ConsoleApp::FindRoute(const std::string& routeId) {
    for (Route& route : routes_) {
        if (route.Id() == routeId) {
            return &route;
        }
    }
    return nullptr;
}

Order& ConsoleApp::RequireOrder(const std::string& orderId) {
    Order* order = company_.Orders().Find(orderId);
    if (order == nullptr) {
        throw OrderNotFoundException("Order not found: " + orderId);
    }
    return *order;
}

Vehicle& ConsoleApp::RequireVehicle(const std::string& vehicleId) {
    Vehicle* vehicle = company_.Fleet().FindVehicle(vehicleId);
    if (vehicle == nullptr) {
        throw InvalidVehicleException("Vehicle not found: " + vehicleId);
    }
    return *vehicle;
}

Driver& ConsoleApp::RequireDriver(const std::string& driverId) {
    Driver* driver = FindDriver(driverId);
    if (driver == nullptr) {
        throw InvalidClientException("Driver not found: " + driverId);
    }
    return *driver;
}

Route& ConsoleApp::RequireRoute(const std::string& routeId) {
    Route* route = FindRoute(routeId);
    if (route == nullptr) {
        throw InvalidRouteException("Route not found: " + routeId);
    }
    return *route;
}

}  // namespace logistics
