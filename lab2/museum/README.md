# Museum Management System — Lab 2

Large-scale object-oriented model of a museum in C++17. The domain covers the
collection, exhibitions, ticketing, guided tours, restoration, research,
finance, security and visitor services. The implementation is split into a
domain library, a console interface and a GoogleTest suite.

## Requirements

- CMake 3.21 or newer
- A C++17 compiler (GCC, Clang or MSVC)
- Ninja is optional

## Build and run

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/museum
```

Without Ninja, omit `-G Ninja`. On Windows use the Visual Studio generator and
run `build/Debug/museum.exe`.

The console starts in interactive mode. Type `help` for the command list:

```
summary                           museum overview
exhibits                          list collection
ticket <visitorId> <price>        sell a discounted ticket
tour <visitorId>                  book the featured tour
restore <exhibitId> <hours>       run a restoration task
donate <amount>                   add funds to the budget
alarm <arm|trigger|reset|disarm> [code]
feedback <rating> <comment...>    leave visitor feedback
climate <temperature>             record storage climate
condition <exhibitId> <score>     assess exhibit condition
depreciate <exhibitId> <percent>  reduce appraised value
exit, quit                        leave the console
```

Example batch run:

```bash
printf 'summary\nrestore p1 3\ndonate 5000\nexit\n' | ./build/museum
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
  -DCMAKE_CXX_COMPILER=clang++ -DCODE_COVERAGE=ON -DPPOIS_COVERAGE_MIN_LINES=90
cmake --build build-coverage
cmake --build build-coverage --target coverage
```

The current suite has 146 tests and reaches 99% line / 100% function coverage.

## Architecture

- `include/museum/` — declarations, one class per header.
- `src/` — implementation grouped by subsystem.
- `src/cli/console_app.cpp` — console layer, independent of domain logic.
- `tests/` — GoogleTest suites per subsystem.
- `../cmake/` — shared build, test and coverage modules.

`Museum` is the aggregate root. It owns departments, employees, exhibits and
exhibitions, and exposes the budget. Other subsystems (ticketing, tours,
restoration, security, facilities) are composed by the console layer.

## Class reference

The table lists declared fields and substantive behaviours. Getters and
setters are omitted from the behaviour column. Associations list classes used
as fields or method parameters.

### People and membership

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Person | id_, fullName_, age_ | IsAdult, Role, Describe | Employee, Visitor (base class) |
| Employee | monthlySalary_, hiredYear_, departmentId_ | AnnualSalary, Promote, IsSenior, Role | Person, Department |
| Curator | specialty_, managedExhibitIds_ | Role, AssignExhibit, ManagedExhibitCount | Employee, Exhibit, Department |
| Restorer | specialization_, completedTasks_ | Role, CanRestore, CompleteTask | Employee, RestorationTask |
| Guide | languages_, rating_ | Role, Speaks, AddLanguage, UpdateRating | Employee, Tour, VisitorGroup |
| Researcher | field_, publications_ | Role, Publish, HasTenure | Employee, Publication, ResearchProject |
| Visitor | ticketType_, balance_ | Role, CanAfford, TopUp, ApplyDiscount | Person, Ticket, Tour, Membership, AudioGuide, VisitorFeedback |
| Membership | visitorId_, level_, validUntilDay_ | IsActive, DiscountPercent, Extend, BelongsTo | Visitor |

### Collection and storage

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Exhibit | id_, title_, epoch_, conditionScore_, appraisedValue_ | AssessCondition, Depreciate, IsLoanable, Category, Summary | Department, Exhibition, Museum, ConditionReport |
| Artifact | material_, weightKg_, origin_, locationRoomId_ | Category, MoveTo, IsFragile | Exhibit |
| Painting | artist_, technique_, widthCm_, heightCm_ | Category, Area, IsLarge | Artifact |
| Sculpture | heightCm_, hasPedestal_, outdoorSuitable_ | Category, VolumeEstimate | Artifact |
| Manuscript | pageCount_, language_, publicationYear_ | Category, IsRare, ReadableBy | Artifact |
| Fossil | species_, era_, ageThousandYears_ | Category, IsPrehistoric, EraDescription | Artifact |
| Jewelry | gemstone_, carat_, authenticated_ | Category, Appraise, IsHighValue | Artifact |
| Weapon | weaponType_, yearMade_, licenseRequired_ | Category, IsAntique | Artifact |
| Textile | fabric_, lengthCm_, widthCm_ | Category, Area, NeedsClimateControl | Artifact |
| Department | name_, floor_, curatorId_, capacity_, exhibitIds_ | AddExhibit, RemoveExhibit, RequireExhibit, AssignCurator, AcceptExhibit, IsFull | Curator, Exhibit, Museum |
| StorageRoom | id_, capacity_, temperature_, humidity_, exhibitIds_ | Store, Remove, IsFull, ClimateOk | Exhibit |
| Showcase | id_, roomId_, capacity_, lit_, exhibitIds_ | Place, Remove, TurnOn, TurnOff, IsFull | Exhibit |

### Exhibitions, tickets and tours

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Exhibition | id_, title_, startDay_, endDay_, ticketPrice_, exhibitIds_ | AddExhibit(id), AddExhibit(Exhibit), IsActiveOn, DurationDays, ExpectedRevenue | Exhibit, Museum |
| Ticket | id_, visitorId_, type_, price_, day_ | ApplyDiscount, IsValidOn, IsRefundable, BelongsTo | Visitor, TicketOffice |
| TicketOffice | tickets_, dailyRevenue_, dailyCapacity_, nextTicketNumber_ | SellTicket, Refund, RevenueForDay | Ticket, Visitor |
| Tour | id_, guideId_, routeName_, maxVisitors_, bookedVisitors_, price_ | Book, AssignGuide, Cancel, IsFull, Revenue | Guide, Visitor |
| TourBooking | tourId_, visitorId_, day_, amount_, paid_, cancelled_ | Pay, Cancel | Tour, Visitor |
| VisitorGroup | id_, schoolName_, size_, guideId_, memberIds_ | AddMember, AssignGuide, IsFull, NeedsGuide | Visitor, Guide |
| AudioGuide | id_, language_, batteryPercent_, rentedToVisitorId_ | Rent, Return, Charge, IsAvailable | Visitor |
| Event | id_, name_, day_, capacity_, ticketsSold_, ticketPrice_ | SellTickets, IsSoldOut, Revenue | TicketOffice |

### Finance and commerce

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Budget | totalAmount_, spentAmount_, category_ | Allocate, Spend, Remaining, IsOverBudget | Museum, Donation, Expense, Grant |
| Expense | id_, category_, amount_, day_, status_ | Approve, Reject, ChargeTo | Budget |
| Donation | donorName_, amount_, purpose_, anonymous_ | IsLarge, DisplayName, ApplyToBudget | Budget |
| ShopItem | name_, price_, stock_, category_ | Sell, Restock, IsLowStock | MuseumShop, Cafe |
| MuseumShop | items_, revenue_ | AddItem, SellItem, TotalStock | ShopItem |
| Invoice | id_, vendor_, amount_, paidAmount_, dueDay_ | Pay, IsOverdue, IsFullyPaid | Budget |
| Grant | title_, amount_, deadlineDay_, awarded_ | Apply, IsExpired, Award, ApplyToBudget | Budget |
| LoanAgreement | exhibitId_, borrower_, startDay_, endDay_, dailyFee_ | IsActiveOn, CalculateFee, Extend, Covers | Exhibit |
| InsurancePolicy | exhibitId_, provider_, premium_, coverage_ | IsUnderinsured, Claim, Renew, Covers | Exhibit |

### Research, restoration and library

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| ResearchProject | title_, leadResearcherId_, budget_, findings_, publications_, completed_ | AddFinding, AddPublication, Complete, LedBy | Researcher, Publication |
| Publication | title_, authorId_, year_, citations_ | Cite, IsRecent, ImpactScore, AuthoredBy | Researcher, ResearchProject |
| ConditionReport | exhibitId_, score_, notes_, day_ | IsCritical, Recommendation, Concerns | Exhibit |
| Library | books_, capacity_ | AddBook, LendBook, ReturnBook, AvailableCount | Book |
| Book | title_, author_, isbn_, available_ | Borrow, Return | Library |
| RestorationLab | tasks_, materials_, capacity_ | AssignTask, CompleteTask, AddMaterial, ConsumeMaterial, PendingCount, TotalCost | RestorationTask, RestorationMaterial |
| RestorationTask | id_, exhibitId_, restorerId_, status_, hoursSpent_, hourlyRate_ | Start, AddHours, Complete, Cost | Exhibit, Restorer, RestorationLab |
| RestorationMaterial | name_, quantity_, unitCost_ | Consume, Restock, TotalValue | RestorationLab |

### Security and visitor services

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| SecurityCamera | id_, zone_, recording_, resolutionP_ | StartRecording, StopRecording, CaptureFrame, IsHighResolution | SecuritySystem |
| Incident | id_, type_, severity_, description_, resolved_ | Resolve, IsCritical, Escalate | SecuritySystem |
| AccessLog | personId_, zone_, day_, granted_ | Grant, Deny | SecuritySystem |
| AccessCard | id_, ownerId_, level_, active_ | Activate, Deactivate, HasLevel | SecuritySystem |
| SecuritySystem | armed_, disarmCode_, cameras_, incidents_, logs_ | Arm, Disarm, ReportIncident, CamerasOnline, LogAccess, RequireLevel, AddCamera | SecurityCamera, Incident, AccessLog, AccessCard |
| AlarmSystem | armed_, triggered_, code_ | Arm, Trigger, Reset, Disarm | — |
| VisitorFeedback | visitorId_, rating_, comment_, day_ | IsPositive, IsComplaint, WrittenBy | Visitor, FeedbackBoard |
| FeedbackBoard | feedbacks_, maxSize_ | Add, AverageRating, ComplaintCount, Clear | VisitorFeedback |
| Cafe | name_, seats_, revenue_, menu_ | AddMenuItem, Sell, SeatGuests | ShopItem |
| ParkingLot | capacity_, occupied_, hourlyFee_ | Park, Leave, Fee, IsFull | — |
| ClimateLog | readings_, minTemperature_, maxTemperature_ | Record, IsStable, Average | — |

### Aggregate root

| Class | Fields | Behaviours | Associations |
|---|---|---|---|
| Address | street_, city_, postalCode_, buildingNumber_ | Format, IsSameCity, Validate | Museum |
| Museum | name_, address_, budget_, departments_, employees_, exhibits_, exhibitions_ | AddDepartment, FindDepartment, Hire, FindEmployee, AddExhibit, FindExhibit, OpenExhibition, AnnualPayroll, TotalCollectionValue, ExhibitsByCategory | Address, Budget, Department, Employee, Exhibit, Exhibition |

## Associations

The model declares 38 associations where one class is stored in another or
passed as a method parameter:

- Museum → Address, Budget, Department, Employee, Exhibit, Exhibition
- Department → Curator, Exhibit
- Exhibition → Exhibit
- TicketOffice → Ticket, Visitor
- Ticket → Visitor
- Tour → Guide, Visitor
- VisitorGroup → Guide, Visitor
- AudioGuide → Visitor
- Membership → Visitor
- VisitorFeedback → Visitor
- FeedbackBoard → VisitorFeedback
- Donation → Budget
- Expense → Budget
- Grant → Budget
- LoanAgreement → Exhibit
- InsurancePolicy → Exhibit
- ConditionReport → Exhibit
- MuseumShop → ShopItem
- Cafe → ShopItem
- Library → Book
- ResearchProject → Researcher, Publication
- Publication → Researcher
- RestorationLab → RestorationTask, RestorationMaterial
- SecuritySystem → SecurityCamera, Incident, AccessLog, AccessCard

## Exceptions

All exceptions derive from `MuseumException`, which derives from
`std::runtime_error`.

| Exception | Raised when |
|---|---|
| MuseumException | base class and invalid museum-level state |
| InvalidPersonException | empty person id/name, negative age, invalid salary or promotion |
| InvalidAddressException | empty address field or postal code that is not six digits |
| InvalidMembershipException | unsupported level, empty owner, negative validity, invalid extension |
| InvalidExhibitException | empty exhibit fields, negative value/weight, invalid capacity |
| ExhibitNotFoundException | an exhibit is missing from a department |
| InvalidConditionException | condition score outside [0, 100] |
| DepartmentNotFoundException | a department name is unknown |
| InvalidDepartmentException | empty name/curator, negative floor or non-positive capacity |
| RestorationException | invalid restoration workflow, unknown task or material |
| InvalidTicketException | malformed ticket, discount or ticket-office capacity |
| TicketSoldOutException | ticket office or event capacity is exhausted |
| BookingException | tour, audio guide or booking operation is invalid |
| InvalidBudgetException | malformed budget, expense, invoice, grant or payment |
| BudgetExceededException | spending exceeds the remaining budget |
| LoanException | invalid loan dates or extension |
| SecurityException | invalid camera, incident, access or alarm operation |
| AccessDeniedException | an access card is inactive or has an insufficient level |
| InvalidRatingException | rating outside [1, 5] |
| InvalidExhibitionException | invalid exhibition dates, price or visitor count |
| InvalidShopItemException | invalid item, price, stock or sale |
| ClimateException | implausible temperature or missing climate readings |
| InsuranceException | invalid policy, claim or renewal |
| LibraryException | invalid book or library operation |
| ResearchException | invalid publication or research project workflow |
| FeedbackException | empty or full feedback board |
| FacilityException | invalid cafe or parking operation |

## Requirement summary

| Metric | Required | Implemented |
|---|---|---|
| Classes | ≥ 50 | 58 |
| Fields | ≥ 150 | 227 |
| Unique behaviours | ≥ 100 | 214 |
| Associations | ≥ 30 | 38 |
| Custom exception classes | ≥ 12 | 27 |

## Project layout

```
lab2/
  cmake/              shared CMake modules
  museum/
    CMakeLists.txt
    Doxyfile
    README.md
    include/museum/   domain and CLI headers
    src/              implementations
    tests/            GoogleTest suites
```
