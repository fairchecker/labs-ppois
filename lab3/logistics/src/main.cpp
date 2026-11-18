#include <iostream>

#include "logistics/cli/console_app.hpp"

/**
 * @file main.cpp
 * @brief Entry point of the logistics console application.
 */

int main() {
    logistics::ConsoleApp application;
    return application.Run(std::cin, std::cout);
}
