#pragma once

#include <string>
#include <vector>

#include "museum/shop_item.hpp"

/**
 * @file cafe.hpp
 * @brief Museum cafe with a menu and seating capacity.
 */

namespace museum {

/**
 * @class Cafe
 * @brief Sells menu items and tracks revenue and seating.
 */
class Cafe {
    public:
    /**
     * @brief Creates a cafe.
     * @throws FacilityException when the name is empty or seats are not positive.
     */
    Cafe(std::string name, int seats);

    /**
     * @brief Adds an item to the menu.
     * @param item menu item.
     * @return true when added, false when the item already exists.
     */
    bool AddMenuItem(const ShopItem& item);

    /**
     * @brief Sells a menu item by name.
     * @param name item name.
     * @param count strictly positive number of items.
     * @return revenue from the sale.
     * @throws FacilityException when the item is unknown.
     * @throws InvalidShopItemException when the sale is invalid.
     */
    double Sell(const std::string& name, int count);

    /**
     * @brief Seats a group of guests.
     * @param guests number of guests.
     * @return true when the group fits in the cafe.
     * @throws FacilityException when the guest count is negative.
     */
    bool SeatGuests(int guests) const;

    /// @brief Returns accumulated revenue.
    double Revenue() const { return revenue_; }

    /// @brief Returns the cafe name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the number of seats.
    int Seats() const { return seats_; }

    /// @brief Returns the number of menu items.
    std::size_t MenuSize() const { return menu_.size(); }

    private:
    /// Cafe name.
    std::string name_;
    /// Number of seats.
    int seats_;
    /// Revenue accumulated from sales.
    double revenue_ = 0.0;
    /// Menu items.
    std::vector<ShopItem> menu_;
};

}  // namespace museum
