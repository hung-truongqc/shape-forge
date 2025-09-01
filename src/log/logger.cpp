#include "logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

// Initialize static member
std::atomic<bool> Logger::instance_exists{false};

std::unique_ptr<Logger> Logger::create() {
    // Check if an instance already exists
    bool expected = false;
    if (!instance_exists.compare_exchange_strong(expected, true)) {
        // Another instance already exists
        return nullptr;
    }
    
    try {
        // Create the Logger instance
        // Note: We can't use std::make_unique because the constructor is private
        // We'll handle the flag reset in the destructor instead
        std::unique_ptr<Logger> logger(new Logger());
        
        return logger;
    } catch (...) {
        // If construction fails, reset the flag and re-throw
        instance_exists.store(false);
        throw;
    }
}

Logger::Logger() {
    try {
        // Step 1: Ensure the log directory exists.
        createLogDirectory();
        
        // Step 2: Initialize the global thread pool for asynchronous logging.
        spdlog::init_thread_pool(8192, 1);
        
        // Step 3: Create the sinks. A logger can have multiple sinks.
        // In this case, we create a single rotating file sink.
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/app.log",      // filename
            1048576 * 5,         // max size: 5 MB
            3,                   // max files
            false                // rotate on open
        );
        
        // A vector to hold all our sinks
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(rotating_sink);
        
        // Step 4: Create the asynchronous logger.
        auto logger = std::make_shared<spdlog::async_logger>(
            "shape-forge-logger",
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );
        
        // Step 5: Set the logging level for this logger.
        logger->set_level(spdlog::level::trace);
        
        // Step 6: Register the logger so it can be accessed globally.
        spdlog::register_logger(logger);
        
        // Log an initialization message
        logger->info("**************************************************");
        logger->info("     Logging System Initialized Successfully");
        logger->info("**************************************************");
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        throw; // Re-throw to allow the application to handle it
    }
}

Logger::~Logger() {
    auto logger = spdlog::get("shape-forge-logger");
    if (logger) {
        logger->info("**************************************************");
        logger->info("        Logging System Shutting Down");
        logger->info("**************************************************");
    }
    
    // Crucial: This flushes all async logs to their targets (e.g., files)
    // and releases all spdlog resources.
    spdlog::shutdown();
    
    // Reset the instance flag to allow creation of new instances
    instance_exists.store(false);
}

void Logger::createLogDirectory() {
    try {
        // This path must match the directory used in the sink's filename.
        std::filesystem::create_directory("logs");
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating log directory 'logs': " << e.what() << std::endl;
        throw std::runtime_error("Failed to create log directory: " + std::string(e.what()));
    }
}