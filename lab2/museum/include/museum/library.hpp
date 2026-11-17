#pragma once

#include <string>
#include <vector>

#include "museum/book.hpp"

/**
 * @file library.hpp
 * @brief Research library with a capacity and lending desk.
 */

namespace museum {

/**
 * @class Library
 * @brief Holds books and tracks availability.
 */
class Library {
    public:
    /**
     * @brief Creates a library.
     * @throws LibraryException when the capacity is not positive.
     */
    explicit Library(int capacity);

    /**
     * @brief Adds a book to the catalog.
     * @param book book to add.
     * @return true when added, false when the ISBN already exists or the library is full.
     */
    bool AddBook(const Book& book);

    /**
     * @brief Lends a book by ISBN.
     * @param isbn book identifier.
     * @return true when the book was available and is now borrowed.
     * @throws LibraryException when the ISBN is unknown.
     */
    bool LendBook(const std::string& isbn);

    /**
     * @brief Returns a book by ISBN.
     * @param isbn book identifier.
     * @return true when the book was borrowed and is now available.
     * @throws LibraryException when the ISBN is unknown.
     */
    bool ReturnBook(const std::string& isbn);

    /// @brief Counts available books.
    std::size_t AvailableCount() const;

    /// @brief Returns the catalog size.
    std::size_t BookCount() const { return books_.size(); }

    private:
    /// Catalog of books.
    std::vector<Book> books_;
    /// Maximum number of books.
    int capacity_;
};

}  // namespace museum
