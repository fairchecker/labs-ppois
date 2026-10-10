#include <iostream>
#include <string>

#include "dictionary.hpp"

int main(int argc, char* argv[]) {
    const std::string usage =
        "Usage: englorus [-f <file>] <command> [<command> ...]\n"
        "\n"
        "Options:\n"
        "  -f <file>    load the dictionary from a file (\"key:value\" per line)\n"
        "  -h, --help   show this help\n"
        "\n"
        "Commands:\n"
        "  add <key> <value>   add a word (replaces an existing translation)\n"
        "  set <key> <value>   update an existing word\n"
        "  get <key>           print the translation\n"
        "  del <key>           delete a word\n"
        "  help                show this help\n"
        "\n"
        "Commands run from left to right and execution stops at the first error.\n"
        "Exit codes: 0 success, 1 word not found, 2 usage error.\n";

    const auto fail = [&](const std::string& message) {
        std::cerr << "error: " << message << '\n' << usage;
    };

    const auto notFound = [](const std::string& key) {
        std::cerr << "error: word not found: " << key << '\n';
    };

    int index = 1;
    std::string file;
    while (index < argc && argv[index][0] == '-') {
        const std::string option = argv[index];
        if (option == "-f") {
            if (index + 1 >= argc) {
                fail("option -f requires a file");
                return 2;
            }
            file = argv[index + 1];
            index += 2;
        } else if (option == "-h" || option == "--help") {
            std::cout << usage;
            return 0;
        } else {
            fail("unknown option: '" + option + "'");
            return 2;
        }
    }

    if (index >= argc) {
        std::cerr << usage;
        return 2;
    }

    DictionaryTree dictionary(file);

    while (index < argc) {
        const std::string command = argv[index];
        ++index;

        if (command == "help") {
            std::cout << usage;
            return 0;
        }

        if (command == "add" || command == "set") {
            if (index + 2 > argc) {
                fail(command + " expects <key> <value>");
                return 2;
            }
            const std::string key = argv[index];
            const std::string value = argv[index + 1];
            index += 2;
            if (command == "add") {
                dictionary.AddWord(key, value);
                std::cout << "added " << key << '\n';
            } else {
                if (dictionary.GetWord(key) == nullptr) {
                    notFound(key);
                    return 1;
                }
                dictionary.SetWord(key, value);
                std::cout << "updated " << key << '\n';
            }
        } else if (command == "get" || command == "del") {
            if (index + 1 > argc) {
                fail(command + " expects <key>");
                return 2;
            }
            const std::string key = argv[index];
            ++index;
            DictionaryNode* node = dictionary.GetWord(key);
            if (node == nullptr) {
                notFound(key);
                return 1;
            }
            if (command == "get") {
                std::cout << node->getContent() << '\n';
            } else {
                dictionary.DeleteWord(key);
                std::cout << "deleted " << key << '\n';
            }
        } else {
            fail("unknown command: '" + command + "'");
            return 2;
        }
    }

    return 0;
}
