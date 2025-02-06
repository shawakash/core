#include <cstdlib>
#include <string>
#include <iostream>

const int SLEEP_TIME = []() -> int {
    const char* envValue = std::getenv("SLEEP_TIME");
    if (envValue != nullptr) {
        try {
            return std::stoi(envValue);
        } catch (const std::exception& e) {
            std::cerr << "Error converting SLEEP_TIME to int: " << e.what() << std::endl;
        }
    }
    return 5;
}();
