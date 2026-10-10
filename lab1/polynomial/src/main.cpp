#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "polynomial.hpp"

int main(int argc, char* argv[]) {
    const std::string usage =
        "Usage: polynomial <command> [args]\n"
        "\n"
        "Commands:\n"
        "  eval <x> <coeffs>   evaluate the polynomial at x\n"
        "  add <a> <b>         print a + b\n"
        "  sub <a> <b>         print a - b\n"
        "  mul <a> <b>         print a * b\n"
        "  div <a> <b>         print a / b (quotient only)\n"
        "  degree <coeffs>     print the degree\n"
        "  coeffs <coeffs>     print normalized coefficients\n"
        "  help                show this help\n"
        "\n"
        "<coeffs> is a single argument holding the coefficients in ascending\n"
        "order separated by spaces: \"1 2 3\" means 1 + 2x + 3x^2.\n"
        "\n"
        "Exit codes: 0 success, 1 domain error, 2 usage error.\n";

    const auto showUsage = [&]() { std::cerr << usage; };

    const auto fail = [&](const std::string& message) {
        std::cerr << "error: " << message << '\n' << usage;
    };

    const auto parseNumber = [](const std::string& text, double& value) -> bool {
        if (text.empty()) return false;
        try {
            std::size_t pos = 0;
            value = std::stod(text, &pos);
            return pos == text.size();
        } catch (const std::exception&) {
            return false;
        }
    };

    const auto parseCoeffs = [&](const std::string& text, Polynomial& out) -> bool {
        std::istringstream stream(text);
        std::string token;
        std::vector<double> values;
        while (stream >> token) {
            double value = 0.0;
            if (!parseNumber(token, value)) return false;
            values.push_back(value);
        }
        if (values.empty()) return false;
        Polynomial parsed{};
        for (std::size_t i = 0; i < values.size(); ++i) {
            parsed[i] = values[i];
        }
        // += вызывает Trim() и отбрасывает старшие нулевые коэффициенты.
        parsed += Polynomial{};
        out = parsed;
        return true;
    };

    const auto printCoeffs = [](std::ostream& out, const Polynomial& polynomial) {
        out << std::setprecision(15);
        for (std::size_t i = 0; i <= polynomial.getDegree(); ++i) {
            if (i != 0) out << ' ';
            out << polynomial[i];
        }
        out << '\n';
    };

    if (argc < 2) {
        showUsage();
        return 2;
    }

    const std::string command = argv[1];

    if (command == "help" || command == "-h" || command == "--help") {
        std::cout << usage;
        return 0;
    }

    if (command == "degree" || command == "coeffs") {
        if (argc != 3) {
            fail(command + " expects <coeffs>");
            return 2;
        }
        Polynomial polynomial{};
        if (!parseCoeffs(argv[2], polynomial)) {
            fail(std::string("invalid coefficients: '") + argv[2] + "'");
            return 2;
        }
        if (command == "degree") {
            std::cout << polynomial.getDegree() << '\n';
        } else {
            printCoeffs(std::cout, polynomial);
        }
        return 0;
    }

    if (command == "eval") {
        if (argc != 4) {
            fail("eval expects <x> <coeffs>");
            return 2;
        }
        double x = 0.0;
        if (!parseNumber(argv[2], x)) {
            fail(std::string("invalid number: '") + argv[2] + "'");
            return 2;
        }
        Polynomial polynomial{};
        if (!parseCoeffs(argv[3], polynomial)) {
            fail(std::string("invalid coefficients: '") + argv[3] + "'");
            return 2;
        }
        std::cout << std::setprecision(15) << polynomial(x) << '\n';
        return 0;
    }

    if (command == "add" || command == "sub" || command == "mul" || command == "div") {
        if (argc != 4) {
            fail(command + " expects <a> <b>");
            return 2;
        }
        Polynomial left{};
        Polynomial right{};
        if (!parseCoeffs(argv[2], left)) {
            fail(std::string("invalid coefficients: '") + argv[2] + "'");
            return 2;
        }
        if (!parseCoeffs(argv[3], right)) {
            fail(std::string("invalid coefficients: '") + argv[3] + "'");
            return 2;
        }
        if (command == "add") {
            printCoeffs(std::cout, left + right);
        } else if (command == "sub") {
            printCoeffs(std::cout, left - right);
        } else if (command == "mul") {
            printCoeffs(std::cout, left * right);
        } else {
            try {
                printCoeffs(std::cout, left / right);
            } catch (const std::invalid_argument&) {
                std::cerr << "error: division by zero\n";
                return 1;
            }
        }
        return 0;
    }

    fail("unknown command: '" + command + "'");
    return 2;
}
