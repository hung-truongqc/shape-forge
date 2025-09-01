#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <atomic>

/**
 * @class Logger
 * @brief Smart pointer managed RAII wrapper for the spdlog library.
 *
 * This class provides controlled creation of logging system instances through
 * smart pointers, ensuring proper initialization and cleanup. Multiple instances
 * are prevented through internal reference counting.
 */
class Logger {
public:
    /**
     * @brief Creates a new Logger instance.
     * @return std::unique_ptr<Logger> on success, nullptr if already exists.
     * @throws spdlog::spdlog_ex on initialization failure.
     * 
     * Only one Logger instance can exist at a time. Subsequent calls will
     * return nullptr until the existing instance is destroyed.
     */
    static std::unique_ptr<Logger> create();

    /**
     * @brief Destroys the Logger and shuts down the spdlog system.
     *
     * This is critical for ensuring all asynchronous logs are written to disk.
     * After destruction, create() can be called again to create a new instance.
     */
    ~Logger();

    // Prevent copying and moving
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

private:
    /**
     * @brief Private constructor to enforce smart pointer creation.
     * @throws spdlog::spdlog_ex on initialization failure.
     */
    Logger();

    /**
     * @brief Ensures that the directory for log files exists.
     *
     * The path is hardcoded to 'logs'.
     */
    void createLogDirectory();

    /**
     * @brief Tracks the number of Logger instances.
     * Used to prevent multiple simultaneous instances.
     */
    static std::atomic<bool> instance_exists;
};