#pragma once
#include <string>
namespace Logger {
    void initializeLogger(std::string file);
    void error(const std::string& str);
    void warning(const std::string& str);
    void log(const std::string& str);
    void disable();
}