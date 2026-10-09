#include "dictionary.hpp"
#include <fstream>

/**
 * @file dictionary.cpp
 * @brief Реализация класса DictionaryTree.
 */

/**
 * @brief Создаёт пустой словарь.
 */
DictionaryTree::DictionaryTree() = default; 

/**
 * @brief Создаёт словарь, загружая слова из файла.
 *
 * Файл должен быть в формате «одна запись на строку»:
 * `ключ:значение` (разделитель — двоеточие). Строки без
 * двоеточия пропускаются.
 * @param filepath путь к файлу со словарём.
 */
DictionaryTree::DictionaryTree(const std::string& filepath){
    std::ifstream in;
    in.open(filepath);
    std::string line, key, content;
    size_t pos;
    if(in.is_open()){
        while (std::getline(in, line))
        {
            pos = line.find(':');
            if (pos == std::string::npos) continue;
            key = line.substr(0, pos);
            content = line.substr(pos + 1);
            this->AddWord(key, content);
        }
    }
    in.close();
}

namespace {
/**
 * @brief Рекурсивно копирует поддерево, начиная с указанного узла.
 * @param node корень копируемого поддерева (может быть nullptr).
 * @return Копия поддерева (или nullptr, если node == nullptr).
 */
std::unique_ptr<DictionaryNode> copySubtree(const DictionaryNode* node) {
    if (node == nullptr) return nullptr;
    auto copy = std::make_unique<DictionaryNode>(node->getKey(), node->getContent());
    copy->setLeft(copySubtree(node->getLeft()));
    copy->setRight(copySubtree(node->getRight()));
    return copy;
}
}  // namespace

/**
 * @brief Копирующий конструктор.
 * @param other словарь, из которого копируется дерево.
 */
DictionaryTree::DictionaryTree(const DictionaryTree& other)
    : root_(copySubtree(other.root_.get())),
      num_words_(other.num_words_) {}

/**
 * @brief Оператор присваивания с копированием.
 * @param other словарь, из которого копируется дерево.
 * @return Ссылка на текущий словарь (*this).
 */
DictionaryTree& DictionaryTree::operator=(const DictionaryTree& other) {
    if (this != &other) {
        root_ = copySubtree(other.root_.get());
        num_words_ = other.num_words_;
    }
    return *this;
}

/**
 * @brief Добавляет слово в словарь.
 *
 * Если ключ уже присутствует, значение обновляется. Иначе создаётся
 * новый узел в соответствующем месте бинарного дерева поиска.
 * @param key слово-ключ.
 * @param content перевод/значение слова.
 */
void DictionaryTree::AddWord(std::string key, std::string content){
    if (root_ == nullptr) {
        root_ = std::make_unique<DictionaryNode>(std::move(key), std::move(content));
        ++num_words_;
        return;
    }
    DictionaryNode* cur = root_.get();
    while(true){
        if(cur->getKey() > key){
            if(cur->getLeft()){
                cur = cur->getLeft();
            }
            else {
                cur->setLeft(std::make_unique<DictionaryNode>(std::move(key), std::move(content)));
                num_words_++;
                return;
            }
        }
        else if(cur->getKey() < key){
            if(cur->getRight()){
                cur = cur->getRight();
            }
            else {
                cur->setRight(std::make_unique<DictionaryNode>(std::move(key), std::move(content)));
                num_words_++;
                return;
            }
        }
        else {
            cur->setContent(std::move(content));
            return;
        }
    }
}

/**
 * @brief Удаляет слово из словаря по ключу.
 *
 * Поддерживает три случая: у удаляемого узла нет потомков,
 * один потомок, либо оба потомка (замена на максимальный узел
 * левого поддерева). Если ключ не найден, ничего не меняется.
 * @param key ключ удаляемого слова.
 */
void DictionaryTree::DeleteWord(const std::string& key){
    if(root_ == nullptr){
        return;
    }
    std::unique_ptr<DictionaryNode>* cur = &root_;
    
    while(true){
        if((*cur) == nullptr) return;
        if((*cur)->getKey() > key){
            cur = &((*cur)->getLeftPtr());
        }
        else if((*cur)->getKey() < key){
            cur = &((*cur)->getRightPtr());
        }
        else break;
    }
    if((*cur)->getLeft() == nullptr && (*cur)->getRight() == nullptr){
        (*cur).reset();
        num_words_--;
        return;
    }
    if((*cur)->getLeft() && (*cur)->getRight()){
        std::unique_ptr<DictionaryNode>* add = cur;
        cur = &((*cur)->getLeftPtr());
        while((*cur)->getRight()){
            cur = &((*cur)->getRightPtr());
        }
        (*add)->setKey((*cur)->getKey());
        (*add)->setContent((*cur)->getContent());
        *cur = std::move((*cur)->getLeftPtr());
        num_words_--;
        return;
    }
    if((*cur)->getLeft()){
        *cur = std::move((*cur)->getLeftPtr());
        num_words_--;
        return;
    }
    else {
        *cur = std::move((*cur)->getRightPtr());
        num_words_--;
        return;
    }
}

/**
 * @brief Выполняет поиск слова по ключу.
 * @param key ключ искомого слова.
 * @return Указатель на узел с найденным словом или nullptr, если слово отсутствует.
 */
DictionaryNode* DictionaryTree::GetWord(const std::string& key){
    DictionaryNode* cur = root_.get();
    while(true){
        if(cur == nullptr) return nullptr;
        if(cur->getKey() < key){
            cur = cur->getRight();
        }
        else if(cur->getKey() > key){
            cur = cur->getLeft();
        }
        else if(cur->getKey() == key){
            return cur;
        }
        else return nullptr;
    }
}

/**
 * @brief Заменяет значение существующего слова.
 *
 * Если ключ не найден в дереве, метод ничего не делает.
 * @param key ключ слова, значение которого заменяется.
 * @param new_content новое значение (перевод).
 */
void DictionaryTree::SetWord(const std::string& key, const std::string& new_content){ // NOLINT(bugprone-easily-swappable-parameters)
    DictionaryNode* cur = root_.get();
    while(true){
        if(cur == nullptr) return;
        if(cur->getKey() < key){
            cur = cur->getRight();
        }
        else if(cur->getKey() > key){
            cur = cur->getLeft();
        }
        else if(cur->getKey() == key){
            cur->setContent(new_content);
            return;
        }
        else return;
    }
}

/**
 * @brief Оператор добавления: эквивалентен AddWord.
 * @param pair пара «ключ — значение».
 */
void DictionaryTree::operator+=(const std::pair<std::string, std::string>& pair){
    AddWord(pair.first, pair.second);
}

/**
 * @brief Оператор удаления: эквивалентен DeleteWord.
 * @param key ключ удаляемого слова.
 */
void DictionaryTree::operator-=(const std::string& key){
    DeleteWord(key);
}

/**
 * @brief Оператор доступа по ключу.
 *
 * Если ключ отсутствует, в словарь автоматически добавляется
 * новая запись с пустым значением.
 * @param key ключ слова.
 * @return Ссылка на значение, соответствующее ключу.
 */
std::string& DictionaryTree::operator[](const std::string& key){
    DictionaryNode* node = GetWord(key);
    if (node == nullptr) {
        AddWord(key, "");
        node = GetWord(key);
    }
    return node->getContentChangeable();
}