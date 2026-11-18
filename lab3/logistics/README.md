# Transport and Logistics Management System — Lab 3

Large-scale object-oriented model of a freight transport company in C++17.
The domain covers geography and routing, cargo, vehicles and drivers, orders
and shipments, warehouses, tracking, finance, insurance and service quality.
The implementation is split into a domain library, a console interface and a
GoogleTest suite. The domain is intentionally different from the museum
domain of Lab 2.

## Requirements

- CMake 3.21 or newer
- A C++17 compiler (GCC, Clang or MSVC)
- Ninja is optional
- Clang and LLVM tools only for the coverage target

## Build and run

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/logistics
```

Without Ninja, omit `-G Ninja`. On Windows use the Visual Studio generator and
run `build/Debug/logistics.exe`.

The console starts with a seeded demo company. Type `help` for the command
list:

```
summary | clients | vehicles | warehouses | drivers | orders
client <id> <name> <contact> <credit> <balance> <rating>
warehouse <id> <name> <city> <capacityKg> <capacityM3>
truck <id> <plate> <capacityKg> <volumeM3> <axles> <trailerKg>
van <id> <plate> <capacityKg> <volumeM3> <parcels>
refrigerated <id> <plate> <capacityKg> <volumeM3> <axles> <trailerKg> <minC> <maxC>
driver <id> <name> <license> <category> <experience> <rating>
order <id> <client> <cargo> <fromCity> <toCity> <price> <priority 0-2>
confirm <orderId> | cancel <orderId> | quote <type> <km> <kg> <m3>
retire <vehicleId>
dispatch <order> <vehicle> <driver> <category> <route> <day>
deliver <shipmentId> <day> | invoice <client> <order> <amount> <issued> <due>
pay <invoiceId> <amount> <reference>
track <shipment> <lat> <lon> <day> <hour> <speed> | tracking <shipment>
report | exit
```

Example batch run:

```bash
printf 'summary\ndispatch O1 T1 D1 C R1 1\ndeliver SHP-1 3\ninvoice C1 O1 900 0 30\nexit\n' \
  | ./build/logistics
```

## Tests and coverage

```bash
cmake -S . -B build-test -G Ninja -DBUILD_TESTING=ON
cmake --build build-test
ctest --test-dir build-test --output-on-failure
```

Coverage uses LLVM source-based instrumentation:

```bash
cmake -S . -B build-coverage -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ -DCODE_COVERAGE=ON \
  -DPPOIS_COVERAGE_MIN_LINES=90
cmake --build build-coverage
cmake --build build-coverage --target coverage
```

The suite contains 129 GoogleTest cases and reaches 99.2% line coverage and
100% function coverage across the domain and console sources.

## Architecture

- `include/logistics/` — declarations grouped by subsystem.
- `src/` — implementation grouped by subsystem.
- `src/cli/console_app.cpp` — console layer, independent of domain logic.
- `tests/` — GoogleTest suites per subsystem.
- `../cmake/` — shared build, test and coverage modules.

`TransportCompany` is the aggregate root. It owns the fleet, warehouses,
clients, orders, tracking history, financial ledger, pricing engine,
shipments and invoices. Drivers, routes and console commands are composed by
the console layer so the domain library does not depend on user input.

## Class reference

The tables list every class, its declared fields, substantive behaviours and
typed associations. Getters, setters, constructors and destructors are
omitted from the behaviour column. Fields inherited from a base class are
documented on the base class.

### Geography and routing

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| GeoPoint | 2: latitude_, longitude_ | 2: DistanceTo, Describe | Address, TrackPoint |
| Address | 4: street_, city_, postalCode_, location_ | 3: Format, IsSameCity, DistanceTo | GeoPoint, RouteSegment, Warehouse, Order, RoutePlanner |
| RouteSegment | 5: origin_, destination_, distanceKm_, roadType_, tollCost_ | 3: TravelMinutes, IsTollRoad, Describe | Address, Route |
| Route | 3: id_, segments_, baseRatePerKm_ | 6: AddSegment, TotalDistanceKm, TotalTollCost, EstimatedMinutes, IsLongHaul, Describe | RouteSegment |

### Cargo

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Cargo | 6: id_, description_, weightKg_, volumeM3_, declaredValue_, ownerId_ | 5: DensityKgPerM3, IsOverweight, ValuePerKg, Describe, Category | CargoManifest, Warehouse |
| FragileCargo | 2: maxDropHeightCm_, packingGrade_ | 3: IsSecurelyPacked, HandlingInstructions, Category | Cargo (base) |
| PerishableCargo | 3: minTemperatureC_, maxTemperatureC_, shelfLifeHours_ | 4: IsTemperatureCompatible, IsExpired, RemainingShelfLifeHours, Category | Cargo (base), StorageZone |
| HazardousCargo | 3: hazardClass_, unNumber_, requiresEscort_ | 2: IsCompatibleWith, Category | Cargo (base) |
| OversizedCargo | 3: lengthCm_, widthCm_, heightCm_ | 3: LongestSideCm, NeedsPermit, Category | Cargo (base) |
| Packaging | 3: type_, costPerUnit_, units_ | 4: TotalCost, AddUnits, IsReusable, TypeName | — |
| CargoManifest | 4: cargoIds_, totalWeightKg_, totalValue_, maxWeightKg_ | 4: AddCargo, RemoveCargo, IsEmpty, AverageValuePerKg | Cargo |

### Orders and shipments

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Order | 9: id_, clientId_, cargoId_, origin_, destination_, createdDay_, status_, price_, priority_ | 7: Confirm, StartTransit, Complete, Cancel, IsActive, Describe, TypeName | Address, OrderRegistry, TransportCompany |
| ExpressOrder | 2: deadlineDay_, surchargePercent_ | 3: IsLate, ApplySurcharge, TypeName | Order (base) |
| BulkOrder | 2: requiresCrane_, loadingHours_ | 3: LoadingCost, TotalHandlingCost, TypeName | Order (base) |
| Shipment | 8: id_, orderId_, vehicleId_, driverId_, routeId_, departureDay_, arrivalDay_, status_ | 7: Depart, Arrive, MarkDelayed, ReassignVehicle, IsDelivered, TransitDays, Describe | TransportCompany |
| DeliverySlot | 5: id_, orderId_, day_, startHour_, endHour_ | 3: DurationHours, Overlaps, ContainsHour | — |
| OrderRegistry | 1: orders_ | 7: Place, Find, Require, ByStatus, Remove, ActiveCount, TotalValue | Order, TransportCompany |

### Clients and service quality

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Client | 6: id_, companyName_, contactPerson_, creditLimit_, balance_, rating_ | 6: CanAfford, Charge, TopUp, UpdateRating, IsReliable, Describe | TransportCompany |
| LoyaltyProgram | 3: clientId_, points_, tier_ | 4: AddPoints, Redeem, DiscountPercent, UpgradeTier | — |
| CustomerFeedback | 5: id_, clientId_, orderId_, rating_, comment_ | 3: IsPositive, IsComplaint, Summary | ServiceQualityMonitor |

### Personnel

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Person | 4: id_, fullName_, phone_, email_ | 3: Describe, HasContactInfo, UpdateContact | Employee (base) |
| Employee | 4: employeeId_, hiredYear_, monthlySalary_, department_ | 4: AnnualSalary, Promote, IsSenior, Role | Person, Driver (base) |
| Driver | 4: license_, experienceYears_, rating_, status_ | 5: CanDrive, BeginTrip, EndTrip, UpdateRating, Role | Employee, DrivingLicense, HRManager, TransportCompany |
| Dispatcher | 2: shift_, assignedShipmentIds_ | 5: AssignShipment, ReleaseShipment, IsOverloaded, Workload, Role | Employee |
| LogisticsManager | 3: region_, budgetLimit_, approvedRouteIds_ | 4: ApproveRoute, IsWithinBudget, ApprovedCount, Role | Employee |
| WarehouseManager | 3: warehouseId_, shift_, processedOperations_ | 3: RegisterOperation, CanHandle, Role | Employee |
| Mechanic | 3: specialization_, completedRepairs_, certificationLevel_ | 3: CanRepair, CompleteRepair, Role | Employee |
| HRManager | 2: trainingBudget_, minRating_ | 3: CanHire, AllocateTraining, Role | Employee, Driver |
| Accountant | 2: taxRatePercent_, processedInvoices_ | 3: NetAmount, RegisterInvoice, Role | Employee |

### Vehicles and fleet

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| DrivingLicense | 4: number_, category_, issuedDay_, expiresDay_ | 4: IsValidOn, AllowsCategory, Renew, DaysUntilExpiry | Driver |
| MaintenanceRecord | 7: id_, vehicleId_, day_, kind_, cost_, odometerKm_, notes_ | 3: IsOverdue, CostPerKm, Describe | VehicleFleet |
| FuelRecord | 5: vehicleId_, liters_, pricePerLiter_, odometerKm_, day_ | 2: Cost, ConsumptionPer100Km | VehicleFleet |
| Vehicle | 7: id_, plateNumber_, capacityKg_, capacityVolumeM3_, odometerKm_, fuelLiters_, status_ | 12: CanCarry, ValidateLoad, UtilizationPercent, Refuel, Drive, SendToMaintenance, ReturnToService, MarkOnRoute, CompleteRoute, Retire, IsAvailable, RangeKm | VehicleFleet, TransportCompany |
| Truck | 2: axleCount_, maxTrailerWeightKg_ | 3: CanPull, TypeName, FuelPer100Km | Vehicle (base) |
| Van | 2: refrigerated_, maxParcelCount_ | 3: FitsParcels, TypeName, FuelPer100Km | Vehicle (base) |
| RefrigeratedTruck | 2: minTemperatureC_, maxTemperatureC_ | 3: KeepsTemperature, TypeName, FuelPer100Km | Truck (base) |
| Trailer | 6: id_, plateNumber_, capacityKg_, volumeM3_, refrigerated_, attachedVehicleId_ | 4: AttachTo, Detach, IsAttached, CanCarry | — |
| VehicleFleet | 3: vehicles_, maintenanceRecords_, fuelRecords_ | 8: AddVehicle, FindVehicle, AvailableVehicles, RemoveVehicle, TotalCapacityKg, AvailabilityPercent, ScheduleMaintenance, RecordFuel | Vehicle, MaintenanceRecord, FuelRecord, TransportCompany |

### Warehouses

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Warehouse | 7: id_, name_, address_, capacityKg_, usedKg_, capacityM3_, usedM3_ | 6: CanStore, Store, Release, UtilizationPercent, IsFull, Describe | Cargo, Address, TransportCompany |
| StorageZone | 5: id_, name_, targetTemperatureC_, capacitySlots_, occupiedSlots_ | 5: Fits, Occupy, Release, IsTemperatureSuitable, UtilizationPercent | PerishableCargo |
| Rack | 4: id_, zoneId_, capacitySlots_, occupiedSlots_ | 3: PlaceSlot, RemoveSlot, IsFull | — |
| InventoryRecord | 5: cargoId_, warehouseId_, zoneId_, storedDay_, quantity_ | 3: AgeDays, IsOverdue, Describe | — |
| WarehouseOperation | 6: id_, warehouseId_, kind_, cargoId_, units_, day_ | 3: IsInbound, IsOutbound, Describe | — |

### Tracking

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| TrackPoint | 5: shipmentId_, position_, day_, hour_, speedKmh_ | 3: IsSpeeding, IsSameDay, Describe | GeoPoint, TrackingService, TransportCompany |
| TrackingService | 1: tracks_ | 6: Record, Latest, History, DistanceTravelled, AverageSpeedKmh, IsStale | TrackPoint, TransportCompany |

### Finance and insurance

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Invoice | 7: id_, clientId_, orderId_, amount_, issuedDay_, dueDay_, paid_ | 4: IsOverdue, MarkPaid, DaysUntilDue, Describe | TransportCompany |
| Payment | 6: id_, invoiceId_, amount_, method_, paidDay_, reference_ | 4: IsComplete, IsRefundable, MethodName, Describe | TransportCompany |
| Tariff | 6: id_, cargoType_, pricePerKm_, minimumCharge_, weightRatePerKg_, volumeRatePerM3_ | 3: Calculate, WithFuelSurcharge, IsApplicable | PricingEngine |
| PricingEngine | 2: tariffs_, fuelSurchargePercent_ | 4: AddTariff, FindFor, Quote, AverageBaseRate | Tariff, TransportCompany |
| InsurancePolicy | 6: id_, vehicleId_, provider_, monthlyPremium_, coverageAmount_, validUntilDay_ | 4: PremiumFor, Covers, IsValidOn, ClaimRatio | Claim |
| Claim | 7: id_, shipmentId_, reason_, policy_, amount_, status_, filedDay_ | 5: Approve, Reject, IsOpen, IsCoveredByPolicy, Describe | InsurancePolicy |
| CompanyFinance | 5: balance_, revenue_, expenses_, taxRatePercent_, taxPaid_ | 7: RecordRevenue, RecordExpense, GrossProfit, TaxAmount, NetProfit, PayTax, IsSolvent | TransportCompany |
| TaxCalculator | 2: ratePercent_, deduction_ | 2: TaxFor, EffectiveRate | — |
| Salary | 5: employeeId_, baseSalary_, bonusPercent_, deduction_, periodMonth_ | 4: Gross, Net, ApplyBonus, Describe | — |

### Operational services

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| RoutePlanner | 2: waypoints_, averageSpeedKmh_ | 4: AddWaypoint, TotalDistanceKm, EstimatedHours, OptimizeOrder | Address |
| Notification | 4: recipientId_, message_, channel_, sent_ | 3: Send, IsSent, Shorten | — |
| ServiceQualityMonitor | 3: feedbacks_, lateDeliveries_, totalDeliveries_ | 5: RecordFeedback, RecordDelivery, AverageRating, OnTimeRate, QualityGrade | CustomerFeedback |
| IncidentReport | 6: id_, shipmentId_, kind_, severity_, day_, resolved_ | 4: Escalate, Resolve, IsCritical, Describe | — |

### Aggregate root

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| TransportCompany | 11: name_, fleet_, warehouses_, clients_, orders_, tracking_, finance_, pricing_, shipments_, invoices_, payments_ | 17: OnboardClient, FindClient, RequireClient, AddWarehouse, FindWarehouse, RegisterVehicle, AddTariff, PlaceOrder, Quote, Dispatch, CompleteDelivery, RequireShipment, BillClient, ReceivePayment, RecordExpense, RecordTrackingPoint, GenerateReport | VehicleFleet, Warehouse, Client, OrderRegistry, TrackingService, CompanyFinance, PricingEngine, Shipment, Invoice, Payment, Order, Driver, Vehicle, TrackPoint |

## Associations

The model has 34 typed associations. An association exists when one class
appears in another as a field type or as a method parameter. Inheritance
relations (`Employee`–`Person`, `Driver`–`Employee`, cargo subclasses,
vehicle subclasses and order subclasses) are documented in the tables but
are not counted as associations.

1. GeoPoint → Address (field `location_`)
2. GeoPoint → TrackPoint (field `position_`)
3. Address → RouteSegment (fields `origin_`, `destination_`)
4. Address → Warehouse (field `address_`)
5. Address → Order (fields `origin_`, `destination_`)
6. Address → RoutePlanner (field `waypoints_`, parameter of `AddWaypoint`)
7. RouteSegment → Route (field `segments_`, parameter of `AddSegment`)
8. Cargo → CargoManifest (parameters of `AddCargo`, `RemoveCargo`)
9. Cargo → Warehouse (parameter of `Store`)
10. PerishableCargo → StorageZone (parameter of `IsTemperatureSuitable`)
11. DrivingLicense → Driver (field `license_`)
12. Vehicle → VehicleFleet (field `vehicles_`, parameter of `AddVehicle`)
13. Vehicle → TransportCompany (parameters of `RegisterVehicle`, `Dispatch`)
14. MaintenanceRecord → VehicleFleet (field `maintenanceRecords_`, parameter)
15. FuelRecord → VehicleFleet (field `fuelRecords_`, parameter)
16. Client → TransportCompany (field `clients_`, parameter of `OnboardClient`)
17. Warehouse → TransportCompany (field `warehouses_`, parameter)
18. VehicleFleet → TransportCompany (field `fleet_`)
19. OrderRegistry → TransportCompany (field `orders_`)
20. TrackingService → TransportCompany (field `tracking_`)
21. CompanyFinance → TransportCompany (field `finance_`)
22. PricingEngine → TransportCompany (field `pricing_`)
23. Shipment → TransportCompany (field `shipments_`)
24. Invoice → TransportCompany (field `invoices_`)
25. Payment → TransportCompany (field `payments_`)
26. CustomerFeedback → ServiceQualityMonitor (field `feedbacks_`, parameter)
27. TrackPoint → TrackingService (field `tracks_`, parameter of `Record`)
28. TrackPoint → TransportCompany (parameter of `RecordTrackingPoint`)
29. Tariff → PricingEngine (field `tariffs_`, parameter of `AddTariff`)
30. Driver → HRManager (parameter of `CanHire`)
31. Driver → TransportCompany (parameter of `Dispatch`)
32. Order → OrderRegistry (field `orders_`, parameter of `Place`)
33. Order → TransportCompany (parameters of `PlaceOrder`, `Dispatch`)
34. InsurancePolicy → Claim (field `policy_`)

## Exceptions

The hierarchy lives in `include/logistics/exceptions.hpp`. All classes derive
from `TransportException`, which derives from `std::runtime_error`. There are
**18 concrete custom exception classes** plus the base class:

| Exception | Raised when |
|---|---|
| TransportException | Base class; holds the diagnostic message |
| InvalidRouteException | Coordinates, addresses, segments, routes or route budgets are invalid |
| InvalidVehicleException | A vehicle, trailer, service or fuel record is malformed |
| VehicleNotAvailableException | A vehicle cannot enter, leave or change an operational state |
| VehicleOverloadException | Cargo weight or volume exceeds the vehicle capacity |
| DriverNotAvailableException | A driver is unavailable or lacks the required license category |
| InvalidLicenseException | A driving license is malformed or its renewal window is invalid |
| InvalidCargoException | Cargo, packaging, manifest, zone or inventory data is invalid |
| CargoOverweightException | A manifest weight limit would be exceeded |
| InvalidOrderException | An order, delivery slot or state transition is invalid |
| OrderNotFoundException | A registry lookup cannot find the order |
| InvalidClientException | Client, contact, personnel, loyalty or feedback data is invalid |
| InsufficientFundsException | A client, trainer budget, company balance or tax payment lacks funds |
| WarehouseCapacityException | A warehouse, zone or rack lacks free capacity |
| InvalidShipmentException | A shipment, warehouse operation or incident is malformed |
| TrackingException | Tracking data is missing, stale or invalid |
| InvalidTariffException | A tariff, pricing rule or tax calculator is invalid |
| InvalidPaymentException | An invoice or payment is malformed or already settled |
| InsuranceClaimException | A policy, claim ratio or claim transition is invalid |

## Requirement coverage

| Requirement | Required | Implemented |
|---|---|---|
| Classes | ≥ 50 | 59 domain classes + 19 exception classes = 78 |
| Fields | ≥ 150 | 249 |
| Unique behaviours | ≥ 100 | 256 |
| Associations | ≥ 30 | 34 |
| Custom exceptions | ≥ 12 | 18 concrete + 1 base |
| README | All classes, fields, methods, associations, exceptions | This document |
| Tests | > 90% line coverage, console-runnable | 129 tests, 99.2% lines, 100% functions |
| CLI | Meaningful operations, separated from domain | `ConsoleApp` in `src/cli/` |
| Build | Console build without IDE | CMake + presets |
| CI/CD | Tests, coverage, static analysis, release artifact | `.github/workflows/cpp-ci.yml`, `release.yml` |

## Code-quality notes

- Numeric thresholds are named constants in headers; no magic numbers in
  validation logic.
- Methods are kept short, with nesting below three levels.
- Getters are inline; substantive logic is declared in headers and defined
  in `src/`.
- No dead code, commented-out blocks or TODOs are present.
- Formatting and naming are consistent: PascalCase methods, `snake_case_`
  fields, `camelCase` parameters and locals.
