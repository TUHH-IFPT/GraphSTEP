#include "Logger.h"
#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void Logger::initializeLogger(std::string file) {
    try {
        file = fs::current_path().string() + "/Log/" + file;
        spdlog::drop_all();

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(file));

        auto combinedLogger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
        combinedLogger->set_pattern("[%H:%M:%S.%e] [%P-%t] [%L] %v");

        spdlog::register_logger(combinedLogger);
        spdlog::flush_every(std::chrono::seconds(10));

    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
    }
}

void Logger::error(const std::string& str) {
    #ifndef DEBUG
        Logger::disable();
    #endif
    if (spdlog::get("logger")) spdlog::get("logger")->error(str);
    else spdlog::error(str);
}

void Logger::warning(const std::string& str) {
    #ifndef DEBUG
        Logger::disable();
    #endif
    if (spdlog::get("logger")) spdlog::get("logger")->warn(str);
    else spdlog::warn(str);
}

void Logger::log(const std::string& str) {
    #ifndef DEBUG
        Logger::disable();
    #endif
    if (spdlog::get("logger")) spdlog::get("logger")->info(str);
    else spdlog::info(str);
}

void Logger::disable() {
    if (spdlog::get("logger"))
        spdlog::get("logger")->set_level(spdlog::level::off);
    else
        spdlog::set_level(spdlog::level::off);
}
