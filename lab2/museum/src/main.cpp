#include <iostream>

#include "museum/cli/console_app.hpp"

/**
 * @file main.cpp
 * @brief Entry point of the museum console application.
 */

int main() {
    museum::ConsoleApp application;
    return application.Run(std::cin, std::cout);
}
