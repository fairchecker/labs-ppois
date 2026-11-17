#pragma once

#include <string>

/**
 * @file book.hpp
 * @brief Book held by the museum research library.
 */

namespace museum {

/**
 * @class Book
 * @brief Library volume with borrow state.
 */
class Book {
    public:
    /**
     * @brief Creates an available book.
     * @throws LibraryException when the title, author or ISBN is empty.
     */
    Book(std::string title, std::string author, std::string isbn);

    /**
     * @brief Marks the book as borrowed.
     * @return true when the book was available.
     */
    bool Borrow();

    /**
     * @brief Returns the book to the library.
     * @return true when the book was borrowed.
     */
    bool Return();

    /// @brief Checks whether the book is available.
    bool IsAvailable() const { return available_; }

    /// @brief Returns the title.
    const std::string& Title() const { return title_; }

    /// @brief Returns the author.
    const std::string& Author() const { return author_; }

    /// @brief Returns the ISBN.
    const std::string& Isbn() const { return isbn_; }

    private:
    /// Book title.
    std::string title_;
    /// Book author.
    std::string author_;
    /// International Standard Book Number.
    std::string isbn_;
    /// Whether the book is on the shelf.
    bool available_ = true;
};

}  // namespace museum
