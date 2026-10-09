#pragma once

#include <string>
#include <memory>
#include <utility>

#include "dictionary_node.hpp"

/**
 * @file dictionary.hpp
 * @brief Англо-русский словарь, реализованный на бинарном дереве поиска.
 */

/**
 * @class DictionaryTree
 * @brief Англо-русский словарь на основе бинарного дерева поиска.
 *
 * Хранит пары «ключ — значение», где ключом является английское слово,
 * а значением — перевод (контент). Поддерживает добавление, удаление
 * и поиск слов, а также операторы-обёртки для удобного доступа
 * (см. операторы, определённые в dictionary.cpp).
 */
class DictionaryTree {
    public:
    /**
     * @brief Создаёт пустой словарь.
     */
    DictionaryTree();

    /**
     * @brief Создаёт словарь, загружая слова из файла.
     * @param filepath путь к файлу со словарём.
     */
    DictionaryTree(const std::string& filepath);

    /**
     * @brief Копирующий конструктор.
     * @param other словарь, из которого копируется дерево.
     */
    DictionaryTree(const DictionaryTree& other);

    /**
     * @brief Оператор присваивания с копированием.
     * @param other словарь, из которого копируется дерево.
     * @return Ссылка на текущий словарь (*this).
     */
    DictionaryTree& operator=(const DictionaryTree& other);

    /**
     * @brief Добавляет слово в словарь.
     *
     * Если ключ уже существует, его значение обновляется.
     * @param key слово-ключ.
     * @param content перевод/значение слова.
     */
    void AddWord(std::string key, std::string content);

    /**
     * @brief Удаляет слово из словаря по ключу.
     * @param key ключ удаляемого слова.
     */
    void DeleteWord(const std::string& key);

    /**
     * @brief Выполняет поиск слова по ключу.
     * @param key ключ искомого слова.
     * @return Указатель на узел с найденным словом или nullptr, если слово отсутствует.
     */
    DictionaryNode* GetWord(const std::string& key);

    /**
     * @brief Заменяет значение существующего слова.
     *
     * Если ключ не найден, метод ничего не делает.
     * @param key ключ слова, значение которого заменяется.
     * @param new_content новое значение (перевод).
     */
    void SetWord(const std::string& key, const std::string& new_content); // NOLINT(bugprone-easily-swappable-parameters)

    /**
     * @brief Оператор добавления: эквивалентен AddWord.
     * @param pair пара «ключ — значение».
     */
    void operator+=(const std::pair<std::string, std::string>& pair);

    /**
     * @brief Оператор удаления: эквивалентен DeleteWord.
     * @param key ключ удаляемого слова.
     */
    void operator-=(const std::string& key);

    /**
     * @brief Оператор доступа по ключу.
     *
     * Если ключ отсутствует, в словарь автоматически добавляется
     * новая запись с пустым значением.
     * @param key ключ слова.
     * @return Ссылка на значение, соответствующее ключу.
     */
    std::string& operator[](const std::string& key);

    private:
    /// Корень бинарного дерева поиска.
    std::unique_ptr<DictionaryNode> root_;
    /// Количество слов в словаре.
    std::size_t num_words_ = 0;

};