#pragma once

#include <string>
#include <vector>

#include "museum/shop_item.hpp"

/**
 * @file museum_shop.hpp
 * @brief Museum shop that manages a catalog and revenue.
 */

namespace museum {

/**
 * @class MuseumShop
 * @brief Point of sale for souvenirs and publications.
 */
class MuseumShop {
    public:
    /**
     * @brief Adds an item to the catalog.
     * @param item item to add.
     * @return true when added, false when an item with the same name exists.
     */
    bool AddItem(const ShopItem& item);

    /**
     * @brief Sells an item by name.
     * @param name item name.
     * @param count strictly positive number of items.
     * @return revenue from the sale.
     * @throws InvalidShopItemException when the item is unknown or the sale is invalid.
     */
    double SellItem(const std::string& name, int count);

    /**
     * @brief Sums the stock of every catalog item.
     * @return total units in stock.
     */
    int TotalStock() const;

    /// @brief Returns accumulated revenue.
    double Revenue() const { return revenue_; }

    /// @brief Returns the number of catalog items.
    std::size_t ItemCount() const { return items_.size(); }

    private:
    /// Catalog items.
    std::vector<ShopItem> items_;
    /// Revenue accumulated from sales.
    double revenue_ = 0.0;
};

}  // namespace museum
