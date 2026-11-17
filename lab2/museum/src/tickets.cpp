#include "museum/exhibition.hpp"

#include <algorithm>
#include <utility>

#include "museum/constants.hpp"
#include "museum/event.hpp"
#include "museum/exceptions.hpp"
#include "museum/exhibit.hpp"
#include "museum/ticket_office.hpp"

namespace museum {

Exhibition::Exhibition(std::string id, std::string title, int startDay, int endDay,
                       double ticketPrice)
    : id_(std::move(id)),
      title_(std::move(title)),
      startDay_(startDay),
      endDay_(endDay),
      ticketPrice_(ticketPrice) {
    if (id_.empty() || title_.empty()) {
        throw InvalidExhibitionException("exhibition id and title must not be empty");
    }
    if (startDay_ < 0 || endDay_ < startDay_) {
        throw InvalidExhibitionException("exhibition dates are invalid");
    }
    if (ticketPrice_ < 0.0) {
        throw InvalidExhibitionException("ticket price must not be negative");
    }
}

bool Exhibition::AddExhibit(const std::string& exhibitId) {
    if (exhibitId.empty()) {
        throw InvalidExhibitException("exhibit id must not be empty");
    }
    if (std::find(exhibitIds_.begin(), exhibitIds_.end(), exhibitId) != exhibitIds_.end()) {
        return false;
    }
    exhibitIds_.push_back(exhibitId);
    return true;
}

bool Exhibition::AddExhibit(const Exhibit& exhibit) { return AddExhibit(exhibit.Id()); }

bool Exhibition::IsActiveOn(int day) const { return day >= startDay_ && day <= endDay_; }

int Exhibition::DurationDays() const { return endDay_ - startDay_ + 1; }

double Exhibition::ExpectedRevenue(int visitors) const {
    if (visitors < 0) {
        throw InvalidExhibitionException("visitor count must not be negative");
    }
    return static_cast<double>(visitors) * ticketPrice_;
}

Ticket::Ticket(std::string id, std::string visitorId, std::string type, double price, int day)
    : id_(std::move(id)),
      visitorId_(std::move(visitorId)),
      type_(std::move(type)),
      price_(price),
      day_(day) {
    if (id_.empty() || visitorId_.empty() || type_.empty()) {
        throw InvalidTicketException("ticket id, visitor id and type must not be empty");
    }
    if (price_ < 0.0 || day_ < 0) {
        throw InvalidTicketException("ticket price and day must not be negative");
    }
}

double Ticket::ApplyDiscount(double percent) {
    if (percent < kPercentMin || percent > kPercentMax) {
        throw InvalidTicketException("discount percent must be in [0, 100]");
    }
    price_ -= price_ * percent / kPercentMax;
    return price_;
}

bool Ticket::IsValidOn(int day) const { return day_ == day; }

bool Ticket::IsRefundable() const { return type_ != "vip"; }

bool Ticket::BelongsTo(const Visitor& visitor) const { return visitorId_ == visitor.Id(); }

TicketOffice::TicketOffice(int dailyCapacity) : dailyCapacity_(dailyCapacity) {
    if (dailyCapacity_ <= 0) {
        throw InvalidTicketException("ticket office capacity must be positive");
    }
}

Ticket TicketOffice::SellTicket(Visitor& visitor, const std::string& type, double basePrice,
                                int day) {
    if (tickets_.size() >= static_cast<std::size_t>(dailyCapacity_)) {
        throw TicketSoldOutException("ticket office capacity reached");
    }
    const double price = visitor.ApplyDiscount(basePrice);
    Ticket ticket("T" + std::to_string(nextTicketNumber_), visitor.Id(), type, price, day);
    ++nextTicketNumber_;
    dailyRevenue_ += price;
    tickets_.push_back(ticket);
    return ticket;
}

bool TicketOffice::Refund(const std::string& ticketId) {
    const auto match = std::find_if(tickets_.begin(), tickets_.end(),
                                    [&ticketId](const Ticket& ticket) {
                                        return ticket.Id() == ticketId;
                                    });
    if (match == tickets_.end() || !match->IsRefundable()) {
        return false;
    }
    dailyRevenue_ -= match->Price();
    tickets_.erase(match);
    return true;
}

double TicketOffice::RevenueForDay(int day) const {
    double revenue = 0.0;
    for (const Ticket& ticket : tickets_) {
        if (ticket.IsValidOn(day)) {
            revenue += ticket.Price();
        }
    }
    return revenue;
}

Event::Event(std::string id, std::string name, int day, int capacity, double ticketPrice)
    : id_(std::move(id)),
      name_(std::move(name)),
      day_(day),
      capacity_(capacity),
      ticketPrice_(ticketPrice) {
    if (id_.empty() || name_.empty()) {
        throw InvalidExhibitionException("event id and name must not be empty");
    }
    if (day_ < 0 || capacity_ <= 0 || ticketPrice_ < 0.0) {
        throw InvalidExhibitionException("event day, capacity or price is invalid");
    }
}

int Event::SellTickets(int count) {
    if (count <= 0) {
        throw InvalidTicketException("ticket count must be positive");
    }
    if (ticketsSold_ + count > capacity_) {
        throw TicketSoldOutException("event capacity exceeded");
    }
    ticketsSold_ += count;
    return ticketsSold_;
}

bool Event::IsSoldOut() const { return ticketsSold_ >= capacity_; }

double Event::Revenue() const { return static_cast<double>(ticketsSold_) * ticketPrice_; }

}  // namespace museum
