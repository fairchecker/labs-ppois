#pragma once

#include <string>

/**
 * @file shop_item.hpp
 * @brief Item sold in the museum shop.
 */

namespace museum {

/**
 * @class ShopItem
 * @brief Product with price, stock and category.
 */
class ShopItem {
    public:
    /**
     * @brief Creates a shop item.
     * @throws InvalidShopItemException when the name or category is empty,
     *         the price is negative or the stock is negative.
     */
    ShopItem(std::string name, double price, int stock, std::string category);

    /**
     * @brief Sells a number of items.
     * @param count strictly positive number of items.
     * @return revenue from the sale.
     * @throws InvalidShopItemException when count is not positive or stock is insufficient.
     */
    double Sell(int count);

    /**
     * @brief Adds items to the stock.
     * @param count strictly positive number of items.
     * @throws InvalidShopItemException when count is not positive.
     */
    void Restock(int count);

    /**
     * @brief Checks whether the stock is low.
     * @return true at or below kLowStockLevel.
     */
    bool IsLowStock() const;

    /// @brief Returns the item name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the item price.
    double Price() const { return price_; }

    /// @brief Returns the current stock.
    int Stock() const { return stock_; }

    /// @brief Returns the item category.
    const std::string& Category() const { return category_; }

    private:
    /// Item name.
    std::string name_;
    /// Item price.
    double price_;
    /// Units in stock.
    int stock_;
    /// Item category.
    std::string category_;
};

}  // namespace museum
