/**
 * @file cli_tests.cpp
 * @brief Юнит-тесты CLI словаря (src/main.cpp) на GoogleTest.
 *
 * Запуск:
 *   ctest --test-dir <build-dir> --output-on-failure
 * или напрямую: ./test_dictionary
 */

#include "dictionary.hpp"
#include "dictionary_node.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Тело CLI подключается прямо в тестовый бинарник, а его main()
// переименовывается: так тесты вызывают CLI напрямую и целиком
// покрывают код src/main.cpp. Стандартные заголовки уже включены выше,
// чтобы макрос main не задел системные файлы.
#define main englorus_cli_main
#include "../src/main.cpp"
#undef main

namespace {

/// Результат одного запуска CLI: код возврата и перехваченные потоки.
struct CliResult {
    int code = -1;
    std::string out;
    std::string err;
};

/// Подменяет std::cin, std::cout и std::cerr и возвращает потоки назад.
class StreamGuard {
    public:
    StreamGuard(std::streambuf* in, std::streambuf* out, std::streambuf* err)
        : oldIn_(std::cin.rdbuf(in)),
          oldOut_(std::cout.rdbuf(out)),
          oldErr_(std::cerr.rdbuf(err)) {}

    ~StreamGuard() {
        std::cin.rdbuf(oldIn_);
        std::cout.rdbuf(oldOut_);
        std::cerr.rdbuf(oldErr_);
    }

    StreamGuard(const StreamGuard&) = delete;
    StreamGuard& operator=(const StreamGuard&) = delete;

    private:
    std::streambuf* oldIn_;
    std::streambuf* oldOut_;
    std::streambuf* oldErr_;
};

/// Запускает CLI с аргументами (args[0] — имя программы) и собирает вывод.
/// @param input содержимое stdin для интерактивного режима.
CliResult RunCli(const std::vector<std::string>& args, const std::string& input = "") {
    std::vector<std::string> storage = args;
    std::vector<char*> argv;
    argv.reserve(storage.size() + 1);
    for (std::string& argument : storage) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    std::istringstream in(input);
    std::ostringstream out;
    std::ostringstream err;
    CliResult result;
    {
        StreamGuard guard(in.rdbuf(), out.rdbuf(), err.rdbuf());
        result.code = englorus_cli_main(static_cast<int>(storage.size()), argv.data());
    }
    result.out = out.str();
    result.err = err.str();
    return result;
}

/// Создаёт временный файл словаря, возвращает его имя.
/// Имя уникально (и без путей), поэтому тесты безопасны при ctest -j
/// и одинаково работают на Windows и POSIX.
class TempDictionaryFile {
    public:
    explicit TempDictionaryFile(const std::string& contents) {
        const ::testing::TestInfo* info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        path_ = std::string("englorus_cli_dictionary_") + info->name() + ".txt";
        std::ofstream out(path_);
        out << contents;
    }

    ~TempDictionaryFile() { std::remove(path_.c_str()); }

    TempDictionaryFile(const TempDictionaryFile&) = delete;
    TempDictionaryFile& operator=(const TempDictionaryFile&) = delete;

    const std::string& path() const { return path_; }

    private:
    std::string path_;
};

}  // namespace

// ===========================================================================
// Справка и ошибки использования
// ===========================================================================
TEST(CliUsage, NoArgumentsStartsInteractiveMode) {
    const CliResult result = RunCli({"englorus"}, "exit\n");
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("Interactive dictionary"));
    EXPECT_TRUE(result.err.empty());
}

TEST(CliUsage, InteractiveModeEndsOnEof) {
    const CliResult result = RunCli({"englorus"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
}

TEST(CliUsage, HelpVariantsPrintUsageToStdout) {
    for (const char* flag : {"help", "-h", "--help"}) {
        const CliResult result = RunCli({"englorus", flag});
        EXPECT_EQ(0, result.code) << flag;
        EXPECT_NE(std::string::npos, result.out.find("Usage: englorus")) << flag;
        EXPECT_TRUE(result.err.empty()) << flag;
    }
}

TEST(CliUsage, HelpCommandStopsProcessing) {
    const CliResult result = RunCli({"englorus", "help", "get", "missing"});
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("Usage: englorus"));
    EXPECT_TRUE(result.err.empty());
}

TEST(CliUsage, UnknownCommandFails) {
    const CliResult result = RunCli({"englorus", "replace", "a", "b"});
    EXPECT_EQ(2, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: unknown command: 'replace'"));
    EXPECT_NE(std::string::npos, result.err.find("Usage: englorus"));
}

TEST(CliUsage, UnknownOptionFails) {
    const CliResult result = RunCli({"englorus", "-x", "get", "a"});
    EXPECT_EQ(2, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: unknown option: '-x'"));
}

TEST(CliUsage, MissingFileArgumentFails) {
    const CliResult result = RunCli({"englorus", "-f"});
    EXPECT_EQ(2, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: option -f requires a file"));
}

TEST(CliUsage, MissingArgumentsFail) {
    const std::vector<std::vector<std::string>> cases = {
        {"englorus", "add", "key"},
        {"englorus", "set", "key"},
        {"englorus", "get"},
        {"englorus", "del"},
    };
    for (const std::vector<std::string>& args : cases) {
        const CliResult result = RunCli(args);
        EXPECT_EQ(2, result.code) << args[1];
        EXPECT_TRUE(result.out.empty()) << args[1];
        EXPECT_NE(std::string::npos, result.err.find("error: ")) << args[1];
    }
}

// ===========================================================================
// Интерактивный режим
// ===========================================================================
TEST(CliInteractive, CommandsRunOnePerLine) {
    const CliResult result = RunCli(
        {"englorus"},
        "add cat кот\nget cat\nset cat кошка\nget cat\ndel cat\nget cat\nexit\n");
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("added cat"));
    EXPECT_NE(std::string::npos, result.out.find("updated cat"));
    EXPECT_NE(std::string::npos, result.out.find("кошка"));
    EXPECT_NE(std::string::npos, result.out.find("deleted cat"));
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: cat"));
}

TEST(CliInteractive, ValueMayContainSpaces) {
    const CliResult result = RunCli(
        {"englorus"},
        "add hello \"привет мир\"\nget hello\nadd bye пока до встречи\nget bye\nexit\n");
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("привет мир\n"));
    EXPECT_NE(std::string::npos, result.out.find("пока до встречи\n"));
}

TEST(CliInteractive, ErrorsDoNotStopTheSession) {
    const CliResult result =
        RunCli({"englorus"}, "add key\nget ghost\nadd cat кот\nget cat\nexit\n");
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.err.find("error: add expects <key> <value>"));
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: ghost"));
    EXPECT_NE(std::string::npos, result.out.find("added cat"));
    EXPECT_NE(std::string::npos, result.out.find("кот"));
}

TEST(CliInteractive, HelpPrintsUsageAndContinues) {
    const CliResult result = RunCli({"englorus"}, "help\nadd cat кот\nexit\n");
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("Usage: englorus"));
    EXPECT_NE(std::string::npos, result.out.find("added cat"));
    EXPECT_TRUE(result.err.empty());
}

TEST(CliInteractive, LoadsDictionaryFileBeforeSession) {
    const TempDictionaryFile file("cat:кошка\n");
    const CliResult result = RunCli({"englorus", "-f", file.path()}, "get cat\nexit\n");
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("кошка"));
    EXPECT_TRUE(result.err.empty());
}

// ===========================================================================
// Добавление и поиск
// ===========================================================================
TEST(CliWords, AddThenGet) {
    const CliResult result = RunCli({"englorus", "add", "hello", "привет", "get", "hello"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
    EXPECT_EQ("added hello\nпривет\n", result.out);
}

TEST(CliWords, AddReplacesExistingTranslation) {
    const CliResult result = RunCli(
        {"englorus", "add", "cat", "кот", "add", "cat", "кошка", "get", "cat"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("added cat\nadded cat\nкошка\n", result.out);
}

TEST(CliWords, AddKeepsOtherWords) {
    const CliResult result = RunCli(
        {"englorus", "add", "cat", "кот", "add", "dog", "собака", "get", "cat", "get", "dog"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("added cat\nadded dog\nкот\nсобака\n", result.out);
}

TEST(CliWords, GetMissingWordFails) {
    const CliResult result = RunCli({"englorus", "get", "ghost"});
    EXPECT_EQ(1, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: ghost"));
}

TEST(CliWords, EmptyTranslationIsPrintedAsEmptyLine) {
    const CliResult result = RunCli({"englorus", "add", "hmm", "", "get", "hmm"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("added hmm\n\n", result.out);
}

// ===========================================================================
// Обновление и удаление
// ===========================================================================
TEST(CliWords, SetUpdatesExistingWord) {
    const CliResult result = RunCli(
        {"englorus", "add", "hello", "привет", "set", "hello", "мир", "get", "hello"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("added hello\nupdated hello\nмир\n", result.out);
}

TEST(CliWords, SetMissingWordFails) {
    const CliResult result = RunCli({"englorus", "set", "ghost", "призрак"});
    EXPECT_EQ(1, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: ghost"));
}

TEST(CliWords, DelRemovesWord) {
    const CliResult result = RunCli(
        {"englorus", "add", "hello", "привет", "del", "hello", "get", "hello"});
    EXPECT_EQ(1, result.code);
    EXPECT_EQ("added hello\ndeleted hello\n", result.out);
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: hello"));
}

TEST(CliWords, DelMissingWordFails) {
    const CliResult result = RunCli({"englorus", "del", "ghost"});
    EXPECT_EQ(1, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: ghost"));
}

TEST(CliWords, ExecutionStopsAtFirstError) {
    const CliResult result =
        RunCli({"englorus", "add", "cat", "кот", "get", "ghost", "add", "dog", "собака"});
    EXPECT_EQ(1, result.code);
    EXPECT_EQ("added cat\n", result.out);
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: ghost"));
}

// ===========================================================================
// Загрузка из файла
// ===========================================================================
TEST(CliFileOption, LoadsDictionaryBeforeCommands) {
    const TempDictionaryFile file("cat:кошка\ndog:собака\n");
    const CliResult result = RunCli({"englorus", "-f", file.path(), "get", "cat"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
    EXPECT_EQ("кошка\n", result.out);
}

TEST(CliFileOption, CommandsAddToLoadedDictionary) {
    const TempDictionaryFile file("cat:кошка\n");
    const CliResult result =
        RunCli({"englorus", "-f", file.path(), "get", "cat", "add", "dog", "собака", "get", "dog"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("кошка\nadded dog\nсобака\n", result.out);
}

TEST(CliFileOption, MissingFileGivesEmptyDictionary) {
    const CliResult result = RunCli({"englorus", "-f", "englorus_cli_no_such_file.txt", "get", "cat"});
    EXPECT_EQ(1, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: cat"));
}

TEST(CliFileOption, OptionsMustComeBeforeCommands) {
    const CliResult result = RunCli({"englorus", "add", "cat", "кот", "-h"});
    EXPECT_EQ(2, result.code);
    EXPECT_EQ("added cat\n", result.out);
    EXPECT_NE(std::string::npos, result.err.find("error: unknown command: '-h'"));
}

// ===========================================================================
// Несколько команд подряд
// ===========================================================================
TEST(CliSequence, AllCommandsInOneRun) {
    const CliResult result = RunCli(
        {"englorus", "add", "cat", "кот", "add", "dog", "собака", "set", "dog", "пёс",
         "get", "dog", "del", "cat", "get", "cat"});
    EXPECT_EQ(1, result.code);
    EXPECT_EQ("added cat\nadded dog\nupdated dog\nпёс\ndeleted cat\n", result.out);
    EXPECT_NE(std::string::npos, result.err.find("error: word not found: cat"));
}

TEST(CliSequence, HelpInSequenceEndsRun) {
    const CliResult result = RunCli({"englorus", "add", "cat", "кот", "help", "get", "cat"});
    EXPECT_EQ(0, result.code);
    EXPECT_NE(std::string::npos, result.out.find("Usage: englorus"));
    EXPECT_TRUE(result.err.empty());
}
