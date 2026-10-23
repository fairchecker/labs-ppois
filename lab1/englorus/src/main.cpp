#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "dictionary.hpp"

/**
 * @file main.cpp
 * @brief Интерфейс командной строки англо-русского словаря.
 *
 * Поддерживаются два режима:
 * - пакетный: команды передаются аргументами при запуске;
 * - интерактивный: команды читаются построчно со стандартного ввода
 *   (запускается, если команды не переданы).
 *
 * В интерактивном режиме доступны команды add, set, get, del, help,
 * exit и quit. Значение слова может содержать пробелы, в том числе
 * в кавычках: `add hello "привет мир"`.
 */

namespace {

const std::string kUsage =
    "Usage: englorus [-f <file>] [<command> [<args> ...]]\n"
    "\n"
    "Options:\n"
    "  -f <file>    load the dictionary from a file (\"key:value\" per line)\n"
    "  -h, --help   show this help\n"
    "\n"
    "Without commands the program starts interactive mode: commands are read\n"
    "line by line until 'exit' or 'quit'.\n"
    "\n"
    "Commands:\n"
    "  add <key> <value>   add a word (replaces an existing translation)\n"
    "  set <key> <value>   update an existing word\n"
    "  get <key>           print the translation\n"
    "  del <key>           delete a word\n"
    "  help                show this help\n"
    "  exit, quit          leave interactive mode\n"
    "\n"
    "In interactive mode a value may contain spaces, for example:\n"
    "  add hello \"привет мир\"\n"
    "\n"
    "In batch mode commands run from left to right and execution stops at the\n"
    "first error.\n"
    "Exit codes: 0 success, 1 word not found, 2 usage error.\n";

void PrintUsage(std::ostream& out) { out << kUsage; }

void Fail(const std::string& message, bool interactive) {
    std::cerr << "error: " << message << '\n';
    if (!interactive) std::cerr << kUsage;
}

void NotFound(const std::string& key) {
    std::cerr << "error: word not found: " << key << '\n';
}

/// Разбирает строку на аргументы с поддержкой кавычек и пробелов в значении.
std::vector<std::string> Tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool hasToken = false;
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;

    for (const char ch : line) {
        if (inSingleQuotes) {
            if (ch == '\'') {
                inSingleQuotes = false;
            } else {
                current += ch;
            }
        } else if (inDoubleQuotes) {
            if (ch == '"') {
                inDoubleQuotes = false;
            } else {
                current += ch;
            }
        } else if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            if (hasToken) {
                tokens.push_back(current);
                current.clear();
                hasToken = false;
            }
        } else if (ch == '\'') {
            inSingleQuotes = true;
            hasToken = true;
        } else if (ch == '"') {
            inDoubleQuotes = true;
            hasToken = true;
        } else {
            current += ch;
            hasToken = true;
        }
    }

    if (hasToken) tokens.push_back(current);
    return tokens;
}

/// Выполняет одну команду. Возвращает код возврата (0/1/2).
int ExecuteCommand(DictionaryTree& dictionary, const std::string& command,
                   const std::vector<std::string>& args, bool interactive) {
    if (command == "add" || command == "set") {
        if (args.size() < 2) {
            Fail(command + " expects <key> <value>", interactive);
            return 2;
        }

        const std::string& key = args[0];
        std::string value = args[1];
        for (std::size_t i = 2; i < args.size(); ++i) {
            value += ' ';
            value += args[i];
        }

        if (command == "add") {
            dictionary.AddWord(key, value);
            std::cout << "added " << key << '\n';
        } else {
            if (dictionary.GetWord(key) == nullptr) {
                NotFound(key);
                return 1;
            }
            dictionary.SetWord(key, value);
            std::cout << "updated " << key << '\n';
        }
        return 0;
    }

    if (command == "get" || command == "del") {
        if (args.size() != 1) {
            Fail(command + " expects <key>", interactive);
            return 2;
        }

        DictionaryNode* node = dictionary.GetWord(args[0]);
        if (node == nullptr) {
            NotFound(args[0]);
            return 1;
        }

        if (command == "get") {
            std::cout << node->getContent() << '\n';
        } else {
            dictionary.DeleteWord(args[0]);
            std::cout << "deleted " << args[0] << '\n';
        }
        return 0;
    }

    Fail("unknown command: '" + command + "'", interactive);
    return 2;
}

int RunInteractive(DictionaryTree& dictionary) {
    std::cout << "Interactive dictionary. Type 'help' for commands, "
                 "'exit' to quit.\n";

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        const std::vector<std::string> tokens = Tokenize(line);
        if (tokens.empty()) continue;

        const std::string& command = tokens[0];
        const std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (command == "exit" || command == "quit") return 0;
        if (command == "help" || command == "-h" || command == "--help") {
            PrintUsage(std::cout);
            continue;
        }

        ExecuteCommand(dictionary, command, args, true);
    }

    return 0;
}

int RunBatch(DictionaryTree& dictionary, int argc, char* argv[], int index) {
    while (index < argc) {
        const std::string command = argv[index];
        ++index;

        if (command == "help") {
            PrintUsage(std::cout);
            return 0;
        }
        if (command == "exit" || command == "quit") return 0;

        std::vector<std::string> args;
        if (command == "add" || command == "set") {
            if (index + 2 > argc) {
                Fail(command + " expects <key> <value>", false);
                return 2;
            }
            args.emplace_back(argv[index]);
            args.emplace_back(argv[index + 1]);
            index += 2;
        } else if (command == "get" || command == "del") {
            if (index + 1 > argc) {
                Fail(command + " expects <key>", false);
                return 2;
            }
            args.emplace_back(argv[index]);
            ++index;
        }

        const int code = ExecuteCommand(dictionary, command, args, false);
        if (code != 0) return code;
    }

    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    int index = 1;
    std::string file;
    while (index < argc && argv[index][0] == '-') {
        const std::string option = argv[index];
        if (option == "-f") {
            if (index + 1 >= argc) {
                Fail("option -f requires a file", false);
                return 2;
            }
            file = argv[index + 1];
            index += 2;
        } else if (option == "-h" || option == "--help") {
            PrintUsage(std::cout);
            return 0;
        } else {
            Fail("unknown option: '" + option + "'", false);
            return 2;
        }
    }

    DictionaryTree dictionary(file);

    if (index >= argc) return RunInteractive(dictionary);
    return RunBatch(dictionary, argc, argv, index);
}
