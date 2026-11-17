#pragma once

#include <string>

/**
 * @file restoration_material.hpp
 * @brief Consumable material used by the restoration lab.
 */

namespace museum {

/**
 * @class RestorationMaterial
 * @brief Stock item measured in units with a unit cost.
 */
class RestorationMaterial {
    public:
    /**
     * @brief Creates a material stock record.
     * @throws RestorationException when the name is empty or quantities are negative.
     */
    RestorationMaterial(std::string name, double quantity, double unitCost);

    /**
     * @brief Consumes material from the stock.
     * @param amount strictly positive quantity.
     * @throws RestorationException when the amount is not positive or exceeds the stock.
     */
    void Consume(double amount);

    /**
     * @brief Adds material to the stock.
     * @param amount strictly positive quantity.
     * @throws RestorationException when the amount is not positive.
     */
    void Restock(double amount);

    /**
     * @brief Computes the stock value.
     * @return quantity multiplied by the unit cost.
     */
    double TotalValue() const;

    /// @brief Returns the material name.
    const std::string& Name() const { return name_; }

    /// @brief Returns the quantity in stock.
    double Quantity() const { return quantity_; }

    /// @brief Returns the unit cost.
    double UnitCost() const { return unitCost_; }

    private:
    /// Material name.
    std::string name_;
    /// Quantity in stock.
    double quantity_;
    /// Cost of one unit.
    double unitCost_;
};

}  // namespace museum
