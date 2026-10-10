/**
 * @file cli_tests.cpp
 * @brief Юнит-тесты CLI многочлена (src/main.cpp) на GoogleTest.
 *
 * Запуск:
 *   ctest --test-dir <build-dir> --output-on-failure
 * или напрямую: ./test_polynomial
 */

#include "polynomial.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Тело CLI подключается прямо в тестовый бинарник, а его main()
// переименовывается: так тесты вызывают CLI напрямую и целиком
// покрывают код src/main.cpp. Стандартные заголовки уже включены выше,
// чтобы макрос main не задел системные файлы.
#define main polynomial_cli_main
#include "../src/main.cpp"
#undef main

namespace {

/// Результат одного запуска CLI: код возврата и перехваченные потоки.
struct CliResult {
    int code = -1;
    std::string out;
    std::string err;
};

/// Подменяет std::cout и std::cerr на строки и возвращает потоки назад.
class StreamGuard {
    public:
    StreamGuard(std::streambuf* out, std::streambuf* err)
        : oldOut_(std::cout.rdbuf(out)), oldErr_(std::cerr.rdbuf(err)) {}

    ~StreamGuard() {
        std::cout.rdbuf(oldOut_);
        std::cerr.rdbuf(oldErr_);
    }

    StreamGuard(const StreamGuard&) = delete;
    StreamGuard& operator=(const StreamGuard&) = delete;

    private:
    std::streambuf* oldOut_;
    std::streambuf* oldErr_;
};

/// Запускает CLI с аргументами (args[0] — имя программы) и собирает вывод.
CliResult RunCli(const std::vector<std::string>& args) {
    std::vector<std::string> storage = args;
    std::vector<char*> argv;
    argv.reserve(storage.size() + 1);
    for (std::string& argument : storage) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    std::ostringstream out;
    std::ostringstream err;
    CliResult result;
    {
        StreamGuard guard(out.rdbuf(), err.rdbuf());
        result.code = polynomial_cli_main(static_cast<int>(storage.size()), argv.data());
    }
    result.out = out.str();
    result.err = err.str();
    return result;
}

/// Разбирает вывод CLI на вещественные числа (пустые токены игнорируются).
std::vector<double> ParseNumbers(const std::string& text) {
    std::istringstream stream(text);
    std::string token;
    std::vector<double> values;
    while (stream >> token) {
        values.push_back(std::stod(token));
    }
    return values;
}

/// Сравнивает вывод CLI с ожидаемым списком коэффициентов.
void ExpectNumbers(const std::string& actual, const std::vector<double>& expected) {
    const std::vector<double> values = ParseNumbers(actual);
    ASSERT_EQ(expected.size(), values.size()) << "вывод: " << actual;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(expected[i], values[i], 1e-9) << "элемент " << i;
    }
}

}  // namespace

// ===========================================================================
// Справка и ошибки использования
// ===========================================================================
TEST(CliUsage, NoArgumentsPrintsUsageToStderr) {
    const CliResult result = RunCli({"polynomial"});
    EXPECT_EQ(2, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("Usage: polynomial"));
}

TEST(CliUsage, HelpVariantsPrintUsageToStdout) {
    for (const std::string& flag : {"help", "-h", "--help"}) {
        const CliResult result = RunCli({"polynomial", flag});
        EXPECT_EQ(0, result.code) << flag;
        EXPECT_NE(std::string::npos, result.out.find("Usage: polynomial")) << flag;
        EXPECT_TRUE(result.err.empty()) << flag;
    }
}

TEST(CliUsage, UnknownCommandFails) {
    const CliResult result = RunCli({"polynomial", "frobnicate", "1 2"});
    EXPECT_EQ(2, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: unknown command: 'frobnicate'"));
    EXPECT_NE(std::string::npos, result.err.find("Usage: polynomial"));
}

TEST(CliUsage, MissingArgumentsFail) {
    const std::vector<std::vector<std::string>> cases = {
        {"polynomial", "add"},
        {"polynomial", "add", "1 2"},
        {"polynomial", "sub", "1 2"},
        {"polynomial", "mul", "1 2"},
        {"polynomial", "div", "1 2"},
        {"polynomial", "eval", "1"},
        {"polynomial", "degree"},
        {"polynomial", "coeffs"},
    };
    for (const std::vector<std::string>& args : cases) {
        const CliResult result = RunCli(args);
        EXPECT_EQ(2, result.code) << args[1];
        EXPECT_TRUE(result.out.empty()) << args[1];
        EXPECT_NE(std::string::npos, result.err.find("error: ")) << args[1];
    }
}

// ===========================================================================
// Разбор коэффициентов и чисел
// ===========================================================================
TEST(CliParsing, NonNumericCoefficientsFail) {
    const std::vector<std::vector<std::string>> cases = {
        {"polynomial", "coeffs", "1 x"},
        {"polynomial", "coeffs", "1 2x"},
        {"polynomial", "coeffs", ""},
        {"polynomial", "coeffs", "   "},
        {"polynomial", "degree", "1 2.3.4"},
        {"polynomial", "eval", "1", "1 2x"},
        {"polynomial", "add", "oops", "1 2"},
        {"polynomial", "mul", "1 2", ""},
    };
    for (const std::vector<std::string>& args : cases) {
        const CliResult result = RunCli(args);
        EXPECT_EQ(2, result.code) << args.back();
        EXPECT_TRUE(result.out.empty()) << args.back();
        EXPECT_NE(std::string::npos, result.err.find("error: invalid")) << args.back();
    }
}

TEST(CliParsing, NonNumericPointFails) {
    const CliResult result = RunCli({"polynomial", "eval", "abc", "1 2"});
    EXPECT_EQ(2, result.code);
    EXPECT_TRUE(result.out.empty());
    EXPECT_NE(std::string::npos, result.err.find("error: invalid number: 'abc'"));
}

// ===========================================================================
// Вычисление значения
// ===========================================================================
TEST(CliEval, IntegerPoint) {
    const CliResult result = RunCli({"polynomial", "eval", "2", "1 2 3"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
    EXPECT_NEAR(17.0, std::stod(result.out), 1e-9);
}

TEST(CliEval, FractionalAndNegativePoints) {
    const CliResult fractional = RunCli({"polynomial", "eval", "0.5", "1 2 3"});
    EXPECT_EQ(0, fractional.code);
    EXPECT_NEAR(2.75, std::stod(fractional.out), 1e-9);

    const CliResult negative = RunCli({"polynomial", "eval", "-1", "1 -2 3"});
    EXPECT_EQ(0, negative.code);
    EXPECT_NEAR(6.0, std::stod(negative.out), 1e-9);
}

TEST(CliEval, ZeroPolynomial) {
    const CliResult result = RunCli({"polynomial", "eval", "10", "0 0"});
    EXPECT_EQ(0, result.code);
    EXPECT_NEAR(0.0, std::stod(result.out), 1e-9);
}

// ===========================================================================
// Арифметика
// ===========================================================================
TEST(CliArithmetic, Add) {
    const CliResult result = RunCli({"polynomial", "add", "1 2", "3 4"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
    EXPECT_EQ("4 6\n", result.out);
    ExpectNumbers(result.out, {4.0, 6.0});
}

TEST(CliArithmetic, Sub) {
    const CliResult result = RunCli({"polynomial", "sub", "5 6 7", "1 2 3"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("4 4 4\n", result.out);
}

TEST(CliArithmetic, SubProducesNegativeCoefficients) {
    const CliResult result = RunCli({"polynomial", "sub", "1 2", "3 4 5"});
    EXPECT_EQ(0, result.code);
    ExpectNumbers(result.out, {-2.0, -2.0, -5.0});
}

TEST(CliArithmetic, Mul) {
    const CliResult result = RunCli({"polynomial", "mul", "1 2", "3 4"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("3 10 8\n", result.out);
}

TEST(CliArithmetic, MulByZero) {
    const CliResult result = RunCli({"polynomial", "mul", "1 2 3", "0"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("0\n", result.out);
}

TEST(CliArithmetic, Div) {
    const CliResult result = RunCli({"polynomial", "div", "3 10 8", "1 2"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
    EXPECT_EQ("3 4\n", result.out);
}

TEST(CliArithmetic, DivByLargerDegreeGivesZero) {
    const CliResult result = RunCli({"polynomial", "div", "1 2", "1 0 1"});
    EXPECT_EQ(0, result.code);
    EXPECT_EQ("0\n", result.out);
}

TEST(CliArithmetic, DivByZeroPolynomialFails) {
    for (const std::string& divisor : {"0", "0 0"}) {
        const CliResult result = RunCli({"polynomial", "div", "1 2", divisor});
        EXPECT_EQ(1, result.code) << divisor;
        EXPECT_TRUE(result.out.empty()) << divisor;
        EXPECT_NE(std::string::npos, result.err.find("error: division by zero")) << divisor;
    }
}

// ===========================================================================
// Степень и нормализация коэффициентов
// ===========================================================================
TEST(CliCoeffs, DegreeTrimsTrailingZeros) {
    EXPECT_EQ("1\n", RunCli({"polynomial", "degree", "1 2 0 0"}).out);
    EXPECT_EQ("3\n", RunCli({"polynomial", "degree", "1 2 3 4"}).out);
    EXPECT_EQ("0\n", RunCli({"polynomial", "degree", "0 0"}).out);
}

TEST(CliCoeffs, PrintsNormalizedCoefficients) {
    EXPECT_EQ("1 2\n", RunCli({"polynomial", "coeffs", "1 2 0 0"}).out);
    EXPECT_EQ("0\n", RunCli({"polynomial", "coeffs", "0 0 0"}).out);
    EXPECT_EQ("-1 0 -3\n", RunCli({"polynomial", "coeffs", "-1 0 -3"}).out);
}

TEST(CliCoeffs, SuccessIsSilentOnStderr) {
    const CliResult result = RunCli({"polynomial", "coeffs", "1 2"});
    EXPECT_EQ(0, result.code);
    EXPECT_TRUE(result.err.empty());
}
